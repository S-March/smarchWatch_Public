/**
 ****************************************************************************************
 *
 * @file ad_nvms.c
 *
 * @brief NVMS adapter implementation
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configNVMS_ADAPTER

#include <osal.h>
#include <ad_flash.h>
#include <ad_nvms.h>
#include <ad_nvms_direct.h>
#include <ad_nvms_ves.h>
#ifndef OS_BAREMETAL
#include <sys_power_mgr.h>
#endif

/* Structure to hold partition table default definition */
typedef struct {
        uint16_t start;
        uint16_t size;
        uint8_t  id;
        uint8_t  flags;
} partiation_table_initializer_t;

/* Define partition table macros to create entries in table. Default macros are empty */
#define PARTITION_TABLE_BEGIN static const partiation_table_initializer_t part_init[] = {
#define PARTITION_TABLE_ENTRY(start, size, id) \
        {(start) / (FLASH_SECTOR_SIZE), (size) / (FLASH_SECTOR_SIZE), (id), 0},
#define PARTITION(start, id, flags) \
        {(start) / (FLASH_SECTOR_SIZE), 0, (id), (flags)},
#define PARTITION2(start, size, id, flags) \
        {(start) / (FLASH_SECTOR_SIZE), (size) / (FLASH_SECTOR_SIZE), (id), (flags)},
#define PARTITION_TABLE_END };

/* Include default partition table definition */
#include <flash_partitions.h>

#define PARTITION_ENTRY_MAGIC  0xEA

/* Code to create partition table on flash that does not have it is on by default */
#ifndef CONFIG_PARTITION_TABLE_CREATE
#define CONFIG_PARTITION_TABLE_CREATE 1
#endif

static PRIVILEGED_DATA partition_t *partitions;

static void add_partition_entry(const partition_entry_t *entry)
{
        partition_t *p = (partition_t *) OS_MALLOC(sizeof(partition_t));
        OS_ASSERT(p);

        if (p) {
                p->data = *entry;
                p->next = partitions;
                p->driver = NULL;
                p->driver_data = 0;
                partitions = p;
        }
}

#if CONFIG_PARTITION_TABLE_CREATE

bool erase_needed(const uint8_t *old, const uint8_t *new, uint32_t size)
{
        int i;

        for (i = 0; i < size; ++i) {
                if ((old[i] & new[i]) != new[i])
                        return true;
        }
        return false;
}

static void add_partition(uint8_t type, uint16_t start_sector, uint16_t sector_count,
                                                                                uint8_t flags)
{
        partition_entry_t new_entry = { PARTITION_ENTRY_MAGIC, type, 0xFF, flags, start_sector,
                                                                                sector_count };
        partition_entry_t entry;
        uint32_t flash_addr = PARTITION_TABLE_ADDR;

        for (;;) {
                ad_flash_read(flash_addr, (uint8_t *) &entry, sizeof(partition_entry_t));
                if (entry.type == type && entry.magic == PARTITION_ENTRY_MAGIC &&
                                                                        entry.valid == 0xFF) {
                        /* Same partition found, check if position is same, if so do nothing */
                        if (entry.sector_count == sector_count &&
                                                        entry.start_sector == start_sector) {
                                return;
                        }
                }
                if (entry.type == 0xFF && !erase_needed((uint8_t *) &entry, (uint8_t *) &new_entry,
                                                                sizeof(partition_entry_t))) {
                        break;
                }
                flash_addr += sizeof(partition_entry_t);
        }
        ad_flash_write(flash_addr, (uint8_t *) &new_entry, sizeof(new_entry));
        add_partition_entry(&new_entry);
}

static void init_default_partitions(void)
{
        size_t i;

        for (i = 0; i < sizeof(part_init) / sizeof(part_init[0]); ++i) {
                int size = (int) part_init[i].size;
                /*
                 * If partition size is not specified in table, compute it from next entry if
                 * it exists.
                 */
                if (size == 0 && (i + 1 < sizeof(part_init) / sizeof(part_init[0]))) {
                        size = (int) part_init[i + 1].start - (int) part_init[i].start;
                }

                /*
                 * Do not add partitions with wrong size.
                 */
                if (size > 0) {
                        add_partition(part_init[i].id, part_init[i].start, size, part_init[i].flags);
                }
        }
}
#endif

void ad_nvms_bind_partition_driver(partition_t *part)
{
        if (ad_nvms_direct_driver.bind(part)) {
                return;
        }
#if dg_configNVMS_VES
        if (ad_nvms_ves_driver.bind(part)) {
                return;
        }
#endif
}

void ad_nvms_bind_drivers(void)
{
        partition_t *part = partitions;

        for (part = partitions; part != NULL; part = part->next) {
                ad_nvms_bind_partition_driver(part);
        }
}

size_t ad_nvms_get_partition_count(void)
{
        size_t count = 0;
        partition_t *partition = partitions;

        while (partition) {
                partition = partition->next;
                count++;
        }

        return count;
}

bool ad_nvms_get_partition_info(size_t index, partition_entry_t *info)
{
        partition_t *partition = partitions;

        while (partition) {
                if (index == 0) {
                        *info = partition->data;
                        return true;
                }
                partition = partition->next;
                index--;
        }

        return false;
}

void ad_nvms_init(void)
{
        partition_entry_t entry;
        uint32_t flash_addr = PARTITION_TABLE_ADDR;

        ad_flash_init();
        ad_nvms_direct_init();
#if dg_configNVMS_VES
        ad_nvms_ves_init();
#endif

        /*
         * Read partition table
         */
        do {
                ad_flash_read(flash_addr, (uint8_t *) &entry, sizeof(partition_entry_t));
                if (entry.type != 0xFF && entry.type != 0 && entry.magic == PARTITION_ENTRY_MAGIC &&
                                                                        entry.valid == 0xFF) {
                        add_partition_entry(&entry);
                }
                flash_addr += sizeof(partition_entry_t);
        } while (entry.type != 0xFF);

#if CONFIG_PARTITION_TABLE_CREATE
        if (partitions == NULL) {
                init_default_partitions();
        }
#endif

        ad_nvms_bind_drivers();
}

nvms_t ad_nvms_open(nvms_partition_id_t id)
{
        partition_t *part = partitions;

        while (part) {
                if (part->data.type == id && part->driver) {
                        return (nvms_t) part;
                }
                part = part->next;
        }
        return NULL;
}

size_t ad_nvms_get_size(nvms_t handle)
{
        partition_t *part = (partition_t *) handle;

        if (part == NULL) {
                OS_ASSERT(0);
                return 0;
        }

        if (part->driver->get_size) {
                return part->driver->get_size(part);
        } else {
                return part->data.sector_count * FLASH_SECTOR_SIZE;
        }
}

int ad_nvms_read(nvms_t handle, uint32_t addr, uint8_t *buf, uint32_t len)
{
        partition_t *part = (partition_t *) handle;

        if (part == NULL) {
                OS_ASSERT(0);
                return -1;
        }

        return part->driver->read(part, addr, buf, len);
}

int ad_nvms_write(nvms_t handle, uint32_t addr, const uint8_t *buf, uint32_t size)
{
        partition_t *part = (partition_t *) handle;

        if (part == NULL) {
                OS_ASSERT(0);
                return -1;
        }

        return part->driver->write(part, addr, buf, size);
}

bool ad_nvms_erase_region(nvms_t handle, uint32_t addr, size_t size)
{
        partition_t *part = (partition_t *) handle;

        if (part == NULL) {
                OS_ASSERT(0);
                return -1;
        }

        if (!part->driver->erase) {
                return false;
        }

        return part->driver->erase(part, addr, size);
}

size_t ad_nvms_get_pointer(nvms_t handle, uint32_t addr, size_t size, const void **ptr)
{
        partition_t *part = (partition_t *) handle;

        if (part == NULL || part->driver == NULL || part->driver->get_ptr == NULL) {
                OS_ASSERT(0);
                return 0;
        }

        return part->driver->get_ptr(part, addr, size, ptr);
}

size_t ad_nvms_erase_size(void)
{
        return FLASH_SECTOR_SIZE;
}

bool ad_nvms_no_cache_flushing(nvms_t handle, uint32_t base, uint32_t size)
{
        size_t sz;
        uint32_t addr;
        partition_t *part = (partition_t *) handle;

        if (part == NULL) {
                OS_ASSERT(0);
                return false;
        }

        sz = ad_nvms_get_size(handle);
        if (sz == 0  ||  base >= size)
                return false;

        if (base + size > sz)
                size = sz - base;

        /* convert partition offset to flash offset */
        addr = part->data.start_sector * FLASH_SECTOR_SIZE + base;
        ad_flash_skip_cache_flushing(addr, size);

        return true;
}

void ad_nvms_flush(nvms_t handle, bool free_mem)
{
        partition_t *part = (partition_t *) handle;

        if (part == NULL || part->driver == NULL) {
                OS_ASSERT(0);
                return;
        }

        if (part->driver->flush) {
                part->driver->flush(part, free_mem);
        }
}

#ifndef OS_BAREMETAL
ADAPTER_INIT_DEP1(ad_nvms_adapter, ad_nvms_init, ad_flash_adapter)
#endif

#endif /* dg_configNVMS_ADAPTER */
