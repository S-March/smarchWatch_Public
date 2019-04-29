/**
 ****************************************************************************************
 *
 * @file ad_nvms_ves.c
 *
 * @brief NVMS VES driver implementation
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configNVMS_VES

#include <string.h>
#include <stddef.h>
#include <osal.h>
#include <ad_flash.h>
#include <ad_nvms_ves.h>

#define FLASH_SECTOR_SIZE 0x1000

/* keep cont_ix_t as small as possible since it is used in big table */
#if ((FLASH_SECTOR_SIZE) / (AD_NVMS_VES_CONTAINER_SIZE)) <= 256
typedef uint8_t con_ix_t;
#else
typedef uint16_t con_ix_t;
#endif

/* container counter type, must hold values from cont_ix_t + 1 */
#if ((FLASH_SECTOR_SIZE) / (AD_NVMS_VES_CONTAINER_SIZE)) < 256
typedef uint8_t con_cnt_t;
#else
typedef uint16_t con_cnt_t;
#endif

/* keep sec_ix_t as small as possible since it is used in big table */
#if (AD_NVMS_MAX_SECTOR_COUNT) <= 256
typedef uint8_t sec_ix_t;
#else
typedef uint16_t sec_ix_t;
#endif

#if (AD_NVMS_MAX_SECTOR_COUNT) < 256
typedef uint8_t sec_cnt_t;
#else
typedef uint16_t sec_cnt_t;
#endif

typedef uint16_t cat_ix_t;

#ifndef OS_BAREMETAL
static PRIVILEGED_DATA OS_MUTEX lock;
#endif

static int ad_nvms_ves_read(struct partition_t *part, uint32_t addr, uint8_t *buf,
                                                                                uint32_t size);
static int ad_nvms_ves_write(struct partition_t *part, uint32_t addr, const uint8_t *buf,
                                                                                uint32_t size);
static bool ad_nvms_ves_erase(struct partition_t *part, uint32_t addr, uint32_t size);
static size_t ad_nvms_ves_get_ptr(struct partition_t *part, uint32_t addr, uint32_t size,
                                                                                const void **ptr);
static bool ad_nvms_ves_bind(struct partition_t *part);

static size_t ad_nvms_ves_get_size(struct partition_t *part);

const partition_driver_t ad_nvms_ves_driver = {
        .bind = ad_nvms_ves_bind,
        .read = ad_nvms_ves_read,
        .write = ad_nvms_ves_write,
        .erase = ad_nvms_ves_erase,
        .get_ptr = ad_nvms_ves_get_ptr,
        .get_size = ad_nvms_ves_get_size,
};

void ad_nvms_ves_init(void)
{
#ifndef OS_BAREMETAL
        if (OS_MUTEX_CREATE_SUCCESS != OS_MUTEX_CREATE(lock)) {
                OS_ASSERT(0);
        }
#endif
}

static inline void part_lock(struct partition_t *part)
{
#ifndef OS_BAREMETAL
        OS_MUTEX_GET(lock, OS_MUTEX_FOREVER);
#endif
}

static inline void part_unlock(struct partition_t *part)
{
#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(lock);
#endif
}


/*
 * CAT - Container Allocation Table provides virtual address translation.
 * It maps virtual address used when addressing reads and writes on partition to proper container.
 * Same virtual address will point to different (sector, container) pair after each write.
 * To access data from Flash virtual address is first divided by container data size and result
 * is used to select entry in CAT while the remainder is offset in container.
 */

/* CAT entry */
typedef struct {
        sec_ix_t sector;                /**< Flash sector for this CAT entry */
        con_ix_t container;             /**< Container number in sector 0xFF no container yet */
} cat_entry_t;

typedef struct {
        uint8_t *free_sector_map;       /**< Bitmap of free sectors */
        con_cnt_t *sector_dirty_count;  /**< Sector dirty container count */
        cat_entry_t *cat;               /**< Container Allocation Table */
        uint16_t cat_size;              /**< Number of entries in CAT */
        uint16_t start_sector;          /**< Partition start sector */
        sec_cnt_t sector_count;         /**< Number of sectors in ves partition */
        uint16_t free_sector_count;     /**< Number of free sectors */
        uint8_t container_size;         /**< Container size 16, 32, 64 */
        uint8_t container_data_size;    /**< Container data size container_data_off less then container_size */
        uint8_t container_data_off;     /**< User data inside container 2 or 4 */
        con_cnt_t containers_per_sector;/**< Number of containers in sector */
        con_cnt_t free_container;       /**< Free container in current sector */
        sec_ix_t current_sector;        /**< Current sector with free containers */
        sec_ix_t last_erased_sector;    /**< Sector that was erased last */
} ves_driver_data_t;

/* When this bit is set container is invalid whole sector should be erased */
#define CONTAINER_INVALID       0x8000U
/* When this bit is set container data is valid, if it's cleared and there index is not 0
 * it means power failure during container write. If there is no other valid copy for this index
 * sector should be rewritten.
 */
#define CONTAINER_CURRENT       0x4000U
/* Bit mask for CAT index */
#define CONTAINER_INDEX_MASK    0x3FFFU
/* Container is unused but ready */
#define CONTAINER_UNUSED        0x7FFFU
/* Container is dirty */
#define CONTAINER_CLEARED       0x0000U
/* Container is not initialized yet */
#define CONTAINER_UNINITIALIZED 0xFFFFU

#define CAT_ENTRY_NONE          ((con_ix_t) 0xFFFF)

typedef struct {
        uint16_t index;
#ifdef CONFIG_NVMS_USE_CRC
        uint16_t crc16;
        uint8_t data[AD_NVMS_VES_CONTAINER_SIZE - (2 * sizeof(uint16_t))];
#else
        uint8_t data[AD_NVMS_VES_CONTAINER_SIZE - sizeof(uint16_t)];
#endif
} container_t;

static inline uint32_t container_addr(ves_driver_data_t *ves, sec_ix_t sector, con_ix_t container)
{
        return (ves->start_sector + sector) * FLASH_SECTOR_SIZE + container * ves->container_size;
}

static inline uint32_t container_data_addr(ves_driver_data_t *ves, sec_ix_t sector,
                                                                con_ix_t container, uint8_t offset)
{
        return (ves->start_sector + sector) * FLASH_SECTOR_SIZE + offset +
                                container * ves->container_size + offsetof(container_t, data);
}

static inline bool current_sector_full(ves_driver_data_t *ves)
{
        return ves->free_container >= ves->containers_per_sector;
}

/* Get free sector, implementation keeps at least one sector */
static sec_ix_t ves_get_free_sector(ves_driver_data_t *ves)
{
        const size_t sc = (ves->sector_count + 7) / 8;
        uint8_t free_bits;
        int i;
        int j;
        for (i = 0; i < sc && ves->free_sector_map[i] == 0; ++i) {
        }

        /*
         * Each bit represents a sector, check that the max
         * number of sectors of the given partition is not exceeded.
         */
        OS_ASSERT(i < sc);
        free_bits = ves->free_sector_map[i];
        for (j = 0; (free_bits & 1) == 0; ++j) {
                free_bits >>= 1;
        }
        ves->free_sector_map[i] ^= (1 << j);
        ves->free_sector_count--;
        return (sec_ix_t) (i * 8 + j);
}

/* Update container index filed on flash */
static void ves_write_index(ves_driver_data_t *ves, sec_ix_t sector, con_ix_t container,
                                                                                uint16_t index)
{
        uint32_t addr = container_addr(ves, sector, container);
        ad_flash_write(addr, (const uint8_t *) &index, 2);
}

static void ves_mark_free_sector(ves_driver_data_t *ves, sec_ix_t sector)
{
        uint8_t mask = 1 << (sector & 7);
        if (0 == (ves->free_sector_map[sector / 8] & mask)) {
                ves->free_sector_map[sector / 8] |= mask;
                ves->free_sector_count++;
        }
}

static void ves_mark_used_sector(ves_driver_data_t *ves, sec_ix_t sector)
{
        uint8_t mask = 1 << (sector & 7);
        if (0 != (ves->free_sector_map[sector / 8] & mask)) {
                ves->free_sector_map[sector / 8] &= ~mask;
                ves->free_sector_count--;
        }
}

/*
 * Erase sector on flash, prepare all containers.
 *
 * If check_dirty is true, all containers are checked, it they are clean no erase is needed.
 */
static void ves_init_sector(ves_driver_data_t *ves, sec_ix_t sector, bool check_dirty)
{
        con_cnt_t i;
        con_cnt_t j;
        uint16_t index;
        uint32_t addr = container_addr(ves, sector, 0);
        bool erase_needed = true;

        /* Check if sector can be initialized without erase */
        if (check_dirty) {
                container_t *cont = (container_t *) ad_flash_get_ptr(addr);
                erase_needed = false;
                for (i = 0; i < ves->containers_per_sector && !erase_needed; ++i, ++cont) {
                        /* Check index field first */
                        if (cont->index != CONTAINER_UNUSED &&
                                                        cont->index != CONTAINER_UNINITIALIZED) {
                                erase_needed = true;
                                break;
                        }
                        /* Check all data */
                        for (j = 0; j < ves->container_data_size; ++j) {
                                if (cont->data[j] != 0xFF) {
                                        erase_needed = true;
                                        break;
                                }
                        }
                }
        }

        if (erase_needed) {
                /* Erase whole sector */
                ad_flash_erase_region(addr, FLASH_SECTOR_SIZE);
        }

        /* Set all containers valid flag to 0, index and current stay as 1s */
        for (i = 0; i < ves->containers_per_sector; ++i) {
                index = (uint16_t) ~CONTAINER_INVALID;
                ves_write_index(ves, sector, i, index);
        }
        /* Update sector bookkeeping */
        ves_mark_free_sector(ves, sector);

        ves->sector_dirty_count[sector] = 0;
}

/*
 * This function moves whole container from one sector to another.
 * It is used during garbage collection.
 * To make it power fail safe following procedure is used:
 * - copy container data to new place
 * - clear old container current flag
 * - write new container index from CAT
 * - clear old container index to 0
 */
static void ves_move_container(ves_driver_data_t *ves, sec_ix_t old_sector,
                        con_ix_t old_container, sec_ix_t new_sector, con_ix_t new_container)
{
        uint32_t new_container_addr;
        const container_t *cont = ad_flash_get_ptr(container_addr(ves, old_sector, old_container));
        uint16_t index = cont->index & CONTAINER_INDEX_MASK;

        new_container_addr = container_data_addr(ves, new_sector, new_container, 0);

        ad_flash_write(new_container_addr, &cont->data[0], ves->container_data_size);

        /* Invalidate old container, index with no current flag */
        ves_write_index(ves, old_sector, old_container, index);

        /* Store new container index field */
        ves_write_index(ves, new_sector, new_container, index | CONTAINER_CURRENT);

        /* Fully invalidate old container */
        ves_write_index(ves, old_sector, old_container, 0);

        ves->sector_dirty_count[old_sector]++;

        /*
         * Dirty containers per sector cannot exceed the max number of
         * containers per sector.
         */
        OS_ASSERT(ves->sector_dirty_count[old_sector] <= ves->containers_per_sector);

        ves->cat[index].sector = new_sector;
        ves->cat[index].container = new_container;
}

/*
 * This function marks unused container as dirty. It is called when more than one sector was
 * found with some unused containers. In normal case there may be many sectors with unused
 * containers but only one with unused and dirty ones.
 * This function is part of flash corruption recovery functionality.
 */
static void ves_mark_unused_as_dirty(ves_driver_data_t *ves, sec_ix_t sector)
{
        const container_t *cont;
        con_ix_t i;

        for (i = 0; i < ves->containers_per_sector; ++i) {
                cont = ad_flash_get_ptr(container_addr(ves, sector, i));
                if (cont->index == CONTAINER_UNUSED) {
                        ves_write_index(ves, sector, i, 0);
                }
        }
}

/*
 * Function clears most dirty sectors to leave desired_free_count of unused sectors.
 * This function sets current sector if current sector was full or not selected.
 */
static void ves_gc(ves_driver_data_t *ves, size_t desired_free_count)
{
        int i;
        const container_t *cont;
        cat_ix_t cat_ix;

        while (desired_free_count > ves->free_sector_count) {
                /*
                 * Starting searching from sector after one that was erased last.
                 */
                sec_ix_t ix = (sec_ix_t) ((ves->last_erased_sector + 1) % ves->sector_count);
                sec_ix_t most_dirty_sector = ix;
                uint8_t max_dirty_count = ves->sector_dirty_count[ix];

                for (i = 1; i < ves->sector_count; ++i, ++ix) {
                        if (ix >= ves->sector_count) {
                                ix = 0;
                        }
#if AD_NVMS_VES_GC_THRESHOLD < 0
                        if (max_dirty_count < ves->sector_dirty_count[ix]) {
                                max_dirty_count = ves->sector_dirty_count[ix];
                                most_dirty_sector = ix;
                        }
#else
                        if (ves->sector_dirty_count[ix] >= AD_NVMS_VES_GC_THRESHOLD &&
                                                                        ix != ves->current_sector) {
                                max_dirty_count = ves->sector_dirty_count[ix];
                                most_dirty_sector = ix;
                                break;
                        }
#endif
                }

                /* Dirty but some containers are still valid */
                if (max_dirty_count < ves->containers_per_sector) {
                        cont = ad_flash_get_ptr(container_addr(ves, most_dirty_sector, 0));

                        for (i = 0; i < ves->containers_per_sector; ++i, ++cont) {
                                cat_ix = cont->index;
                                /* Skip dirty, invalid and unused containers */
                                if ((cat_ix == CONTAINER_UNUSED) || (cat_ix == CONTAINER_CLEARED) ||
                                        (cat_ix & CONTAINER_INVALID) ||
                                        (cat_ix & CONTAINER_INDEX_MASK) >= ves->cat_size) {
                                        continue;
                                }
                                /*
                                 * No space in current sector, find next free sector.
                                 */
                                if (current_sector_full(ves) && ves->free_sector_count > 0) {
                                        ves->free_container = 0;
                                        ves->current_sector = ves_get_free_sector(ves);
                                }

                                /*
                                 * If there is space, move data. Otherwise, the flash is corrupted.
                                 */
                                if (!current_sector_full(ves)) {
                                        ves_move_container(ves, most_dirty_sector, i,
                                                        ves->current_sector, ves->free_container++);
                                }
                        }
                }
                ves->last_erased_sector = most_dirty_sector;

                ves_init_sector(ves, most_dirty_sector, false);

                if (current_sector_full(ves)) {
                        /* Now at least one sector is free, use it */
                        ves->free_container = 0;
                        ves->current_sector = ves_get_free_sector(ves);
                }
        }
}

/*
 * Read CAT structure from flash.
 *
 * This function reads index field from all sectors on partition to fill CAT table.
 * Index has following fields
 *   15 bit - Valid - must be zero
 *   14 bit - Current - 1 if data is current
 * 13-0 bits - Index in cat
 * Index can have following values
 * 1xxx xxxx xxxx xxxx - MSB set container is invalid this happen after flash is erased, usually
 *                       all containers in sector are invalid after erase and they are initialized
 *                       to 0x7FFF by ves_init_sector()
 * 0000 0000 0000 0000 - All zeros mean that this container is no longer used, it's dirty.
 * 0100 0000 0000 0000 - This value is invalid and is not result of normal operation. If this
 *                       value is found container is treated as uninitialized.
 * 01xx xxxx xxxx xxxx - This is normal value for container with valid data, 14 bits hold index
 *                       to CAT table.
 * 00xx xxxx xxxx xxxx - This contents means power failure during writing new container. If this
 *                       value is found along with same index having current flag set (0x4000),
 *                       container index is cleared during ves_read_cat since new version is
 *                       available.
 *                       If there is no container with current flag, container will stay in cat
 *                       as long as there is not write to same virtual address, at which time
 *                       index of this container will be cleared.
 *
 * If sector has only invalid containers and dirty ones it's erased and initialized during read_cat.
 * If sector has all containers unused, whole sector is marked as free.
 * If sector has some valid data it's not touched unless there are no free sectors and then data
 *   from this sector is moved to new sector during garbage collection.
 *
 * There may be many sectors with 0 or all unused containers but only one
 * with 0 < unused_count < containers_per_sector, this sector will be current sector and
 * will receive new data.
 */
static void ves_read_cat(ves_driver_data_t *ves)
{
        size_t i;
        size_t j;
        con_cnt_t uninitialized_count;
        con_cnt_t unused_count;
        con_cnt_t dirty_count;
        sec_ix_t old_sector;
        con_ix_t old_container;
        cat_ix_t cat_ix;
        const container_t *cont;
        int16_t first_free_sector = -1;
        int16_t current_sector = -1;
        ves->last_erased_sector = 0;
        ves->free_container = ves->containers_per_sector;

        for (i = 0; i < ves->sector_count; ++i) {
                uninitialized_count = 0;
                old_container = 0;
                unused_count = 0;
                dirty_count = 0;
                for (j = 0; j < ves->containers_per_sector; ++j) {

                        cont = ad_flash_get_ptr(container_addr(ves, i, j));
                        if (cont->index == CONTAINER_UNUSED) {
                                unused_count++;
                                continue;
                        }
                        cat_ix = cont->index & CONTAINER_INDEX_MASK;
                        /*
                         * Even if there were containers marked as unused but after them there are
                         * some other (used, unused, invalid), flash is corrupted and those
                         * unused containers should be treated as dirty ones. Unused containers
                         * can only be at the end of a sector.
                         */
                        dirty_count += unused_count;
                        unused_count = 0;
                        if ((cont->index & CONTAINER_INVALID) || cont->index == CONTAINER_CURRENT) {
                                /* Looks like uninitialized container */
                                uninitialized_count++;
                        } else if ((cat_ix == CONTAINER_CLEARED) || (cat_ix >= ves->cat_size)) {
                                dirty_count++;
                        } else {
                                old_sector = ves->cat[cat_ix].sector;
                                old_container = ves->cat[cat_ix].container;

                                if (old_container == CAT_ENTRY_NONE) {
                                        /*
                                         * First copy of container just store in cat, regardless
                                         * if it is current or not.
                                         */
                                        ves->cat[cat_ix].sector = i;
                                        ves->cat[cat_ix].container = j;
                                } else {
                                        /* Found second copy of container */
                                        if (cont->index & CONTAINER_CURRENT) {
                                                /* New one is current, mark old one as dirty */
                                                ves_write_index(ves, old_sector, old_container, 0);
                                                ves->cat[cat_ix].sector = i;
                                                ves->cat[cat_ix].container = j;
                                                /*
                                                 * If old container was in other sector, increase
                                                 * dirty counter in old sector. If it was in
                                                 * current sector, just increase dirty_count.
                                                 */
                                                if (i != old_sector) {
                                                        ves->sector_dirty_count[old_sector]++;
                                                        /*
                                                         * Dirty containers per sector cannot exceed
                                                         * the max number of containers per sector.
                                                         */
                                                        OS_ASSERT(ves->sector_dirty_count[old_sector]
                                                                    <= ves->containers_per_sector);
                                                } else {
                                                        dirty_count++;
                                                }
                                        } else {
                                                /* This one is old mark as dirty */
                                                ves_write_index(ves, i, j, 0);
                                                dirty_count++;
                                        }
                                }
                        }
                }
                if (ves->containers_per_sector == unused_count) {
                        /* All entries are unused whole sector is free */
                        ves_mark_free_sector(ves, i);
                        if (first_free_sector < 0) {
                                first_free_sector = i;
                        }
                } else if (ves->containers_per_sector == unused_count + uninitialized_count
                                                                                + dirty_count) {
                        /*
                         * No free space on sector, init sector. If there were dirty containers
                         * ves_init_sector will do erase. If there was not dirty containers
                         * ves_init_sector will check if erase is really needed or just containers
                         * initialization on clean sector will do.
                         */
                        ves_init_sector(ves, i, dirty_count == 0);
                        if (first_free_sector < 0) {
                                first_free_sector = i;
                        }
                } else {
                        ves->sector_dirty_count[i] = dirty_count;
                        /*
                         * Dirty containers per sector cannot exceed
                         * the max number of containers per sector.
                         */
                        OS_ASSERT(ves->sector_dirty_count[i] <= ves->containers_per_sector);
                        /* Found sector that is partially used, this is current sector */
                        if (unused_count > 0 && unused_count < ves->containers_per_sector) {
                                if (current_sector < 0) {
                                        current_sector = i;
                                        ves->current_sector = (sec_ix_t) i;
                                        ves->free_container = ves->containers_per_sector - unused_count;
                                } else {
                                        /*
                                         * Flash corruption detected, there are two sectors with
                                         * some unused containers, which can happen only if some
                                         * data was put in flash outside of driver. To recover
                                         * just mark unused sector as dirty on flash.
                                         */
                                        ves_mark_unused_as_dirty(ves, i);
                                }
                        }
                }
        }
        /* Current sector not found, set first free as current */
        if (current_sector_full(ves)) {
                if (first_free_sector >= 0) {
                        ves->current_sector = first_free_sector;
                        ves->free_container = 0;
                        ves_mark_used_sector(ves, first_free_sector);
                }
        }
        /* Make sure that there is at least one free sector */
        ves_gc(ves, 1);
}

/*
 * This function gets free container.
 * If there is sector with unused containers it is used.
 * It current sector is full, tries to get new clean sector to use.
 * In case there is no free sector, garbage collation is done on dirtiest sector.
 */
static void ves_get_free_container(ves_driver_data_t *ves, sec_ix_t *sector,
                                                                        con_ix_t *container)
{
        /* Prepare next free container */
        if (!current_sector_full(ves)) {
                /* Free container in current sector */
                *sector = ves->current_sector;
                *container = ves->free_container;
                ves->free_container++;
        } else {
                /* Find next free sector, there is at lease one kept for swap */
                ves->current_sector = ves_get_free_sector(ves);
                ves->free_container = 0;
                if (ves->free_sector_count == 0) {
                        ves_gc(ves, 1);
                }
                /* Free container from current sector */
                *sector = ves->current_sector;
                *container = ves->free_container++;
        }
}

/*
 * Write data to container designated by cat_ix
 * This function is called only if write is done to address that require allocation of new
 * container. It can happen that write is done to place that was never written, or to place
 * with that data that can't be update without erase.
 */
static int ves_container_write(ves_driver_data_t *ves, cat_ix_t cat_ix,
                                size_t offset_in_container, const uint8_t *buf, size_t size)
{
        sec_ix_t old_sector;
        con_ix_t old_container;
        sec_ix_t new_sector;
        con_ix_t new_container;
        uint32_t new_container_addr;
        uint32_t old_container_addr;
        uint16_t index = (uint16_t) cat_ix;

        ves_get_free_container(ves, &new_sector, &new_container);

        old_sector = ves->cat[cat_ix].sector;
        old_container = ves->cat[cat_ix].container;

        new_container_addr = container_data_addr(ves, new_sector, new_container, 0);
        /*
         * container index (starting with 0) cannot exceed
         * the max number of containers per sector.
         */
        OS_ASSERT(new_container < ves->containers_per_sector);

        /* Not whole container written, copy old data if there was old container */
        if (old_container != CAT_ENTRY_NONE && size < ves->container_data_size) {
                old_container_addr = container_data_addr(ves, old_sector, old_container, 0);
                const uint8_t *old_data = ad_flash_get_ptr(old_container_addr);

                /* Write data from start of old container */
                if (offset_in_container > 0) {
                        ad_flash_write(new_container_addr, old_data, offset_in_container);
                }

                /* Write data from end of old container */
                if (offset_in_container + size < ves->container_data_size) {
                        ad_flash_write(new_container_addr + offset_in_container + size,
                                old_data + offset_in_container + size,
                                ves->container_data_size - (offset_in_container + size));
                }
        }
        if (size > 0) {
                /* Write new data */
                ad_flash_write(new_container_addr + offset_in_container, buf, size);
        }

        /* Invalidate old container, index with no current flag */
        if (old_container != CAT_ENTRY_NONE) {
                ad_flash_write(container_addr(ves, old_sector, old_container),
                                                                (const uint8_t *) &index, 2);
        }

        /* Store new container index field */
        index = cat_ix | CONTAINER_CURRENT;
        ad_flash_write(container_addr(ves, new_sector, new_container), (const uint8_t *) &index, 2);

        /* Fully invalidate old container, by writing index 0 */
        if (old_container != CAT_ENTRY_NONE) {
                index = 0;
                ad_flash_write(container_addr(ves, old_sector, old_container),
                                                                (const uint8_t *) &index, 2);
                ves->sector_dirty_count[old_sector]++;
                /*
                 * Dirty containers per sector cannot exceed
                 * the max number of containers per sector.
                 */
                OS_ASSERT(ves->sector_dirty_count[old_sector] <= ves->containers_per_sector);
        }

        /* Store new container for this cat_ix */
        ves->cat[cat_ix].sector = new_sector;
        ves->cat[cat_ix].container = new_container;

        return size;
}

void ves_init(struct partition_t *part)
{
        ves_driver_data_t *ves = (ves_driver_data_t *) part->driver_data;

        ves->start_sector = part->data.start_sector;
        ves->sector_count = part->data.sector_count;
        ves->container_size = AD_NVMS_VES_CONTAINER_SIZE;
        ves->container_data_off = offsetof(container_t, data);
        ves->container_data_size = ves->container_size - ves->container_data_off;
        ves->containers_per_sector = FLASH_SECTOR_SIZE / ves->container_size;
        ves->current_sector = 0;
        ves->free_container = 0;
        ves->free_sector_count = 0;

        /* Have enough entries to cover virtual address space + 1 (for index 0 which is not used) */
        ves->cat_size = (ves->container_data_size - 1 + ves->sector_count * FLASH_SECTOR_SIZE) /
                                        AD_NVMS_VES_MULTIPLIER / ves->container_data_size + 1;
        ves->cat = OS_MALLOC(ves->cat_size * sizeof(cat_entry_t));
        OS_ASSERT(ves->cat);
        memset(ves->cat, CAT_ENTRY_NONE, ves->cat_size * sizeof(cat_entry_t));

        ves->free_sector_map = OS_MALLOC((ves->sector_count + 7) / 8);
        OS_ASSERT(ves->free_sector_map);
        memset(ves->free_sector_map, 0, (ves->sector_count + 7) / 8);

        ves->sector_dirty_count = OS_MALLOC(ves->sector_count * sizeof(ves->sector_dirty_count[0]));
        memset(ves->sector_dirty_count, 0, ves->sector_count);

        ves_read_cat(ves);
}

static int ad_nvms_ves_read(struct partition_t *part, uint32_t addr, uint8_t *buf,
                                                                                uint32_t size)
{
        cat_ix_t cat_ix;
        size_t offset = 0;
        con_ix_t container;
        sec_ix_t sector;
        size_t offset_in_container;
        size_t chunk;
        ves_driver_data_t *ves = (ves_driver_data_t *) part->driver_data;
        const size_t data_size = ves->container_data_size;
        const size_t part_size = ad_nvms_ves_get_size(part);

        /* Write outside virtual address space */
        if (addr >= part_size) {
                return 0;
        }

        /* Write outside virtual address space, reduce size */
        if (addr + size > part_size) {
                size = part_size - addr;
        }

        part_lock(part);

        while (offset < size) {
                size_t read;

                cat_ix = 1 + (addr + offset) / (data_size);
                container = ves->cat[cat_ix].container;
                sector = ves->cat[cat_ix].sector;
                offset_in_container = (addr + offset) % (data_size);
                chunk = data_size - offset_in_container > (size - offset) ? (size - offset) :
                                                                data_size - offset_in_container;
                if (container == CAT_ENTRY_NONE) {
                        memset(buf + offset, 0xFF, chunk);
                        read = chunk;
                } else {
                        read = ad_flash_read(container_data_addr(ves, sector, container, offset_in_container),
                                                                        buf + offset, chunk);
                }
                offset += read;
        }

        part_unlock(part);

        return (int) offset;
}

static void ad_nvms_container_update(ves_driver_data_t *ves, cat_ix_t cat_ix, const uint8_t *buf,
                                uint32_t offset_in_container, uint32_t chunk)
{
        int same;
        size_t written __attribute__((unused));
        const con_ix_t container = ves->cat[cat_ix].container;
        const sec_ix_t sector = ves->cat[cat_ix].sector;

        /* if there was container for this address check if update is possible */
        if (container != CAT_ENTRY_NONE) {
                same = ad_flash_update_possible(
                        container_data_addr(ves, sector, container, offset_in_container),
                                                                buf, chunk);
        } else {
                /* This address was not used at all, new container needed */
                same = -1;
        }

        if (same == chunk) {
                /* No write needed, same data */
        } else if (same >= 0) {
                written = ad_flash_write(container_data_addr(ves, sector, container, offset_in_container + same),
                                                        buf + same, chunk - same);
                /* check that the intended content-size is actually written in flash */
                OS_ASSERT(same + written == chunk);
        } else {
                /* Need new container */
                ves_container_write(ves, cat_ix, offset_in_container, buf, chunk);
        }
}

static int ad_nvms_ves_write(struct partition_t *part, uint32_t addr, const uint8_t *buf,
                                                                                uint32_t size)
{
        size_t offset = 0;
        size_t chunk;
        ves_driver_data_t *ves = (ves_driver_data_t *) part->driver_data;
        const size_t part_size = ad_nvms_ves_get_size(part);

        /* Write outside virtual address space */
        if (addr >= part_size) {
                return 0;
        }

        /* Write outside virtual address space, reduce size */
        if (addr + size > part_size) {
                size = part_size - addr;
        }

        part_lock(part);

        while (offset < size) {
                const cat_ix_t cat_ix = 1 + (addr + offset) / ves->container_data_size;
                const size_t offset_in_container = (addr + offset) % ves->container_data_size;
                chunk = ves->container_data_size - offset_in_container;
                if (chunk > size - offset) {
                        chunk = size - offset;
                }

                ad_nvms_container_update(ves, cat_ix, buf + offset, offset_in_container, chunk);
                offset += chunk;
        }

        part_unlock(part);

        return (int) offset;
}

static bool ad_nvms_ves_erase(struct partition_t *part, uint32_t addr, uint32_t size)
{
        return false;
}

static size_t ad_nvms_ves_get_ptr(struct partition_t *part, uint32_t addr, uint32_t size,
                                                                                const void **ptr)
{
        return 0;
}

static bool ad_nvms_ves_bind(struct partition_t *part)
{
        bool ret = false;

        switch (part->data.type) {
        case NVMS_GENERIC_PART:
                part->driver = &ad_nvms_ves_driver;
                part->driver_data = OS_MALLOC(sizeof(ves_driver_data_t));
                OS_ASSERT(part->driver_data);
                ves_init(part);
                ret = true;
                break;
        default:
                if (part->data.flags & PARTITION_FLAG_VES) {
                        /* Custom read-only VES partitions are not allowed */
                        OS_ASSERT((part->data.flags & PARTITION_FLAG_READ_ONLY) == 0);
                        part->driver_data = OS_MALLOC(sizeof(ves_driver_data_t));
                        OS_ASSERT(part->driver_data);
                        part->driver = &ad_nvms_ves_driver;
                        ves_init(part);
                        ret = true;
                        break;
                }
                break;
        }
        return ret;
}

static size_t ad_nvms_ves_get_size(struct partition_t *part)
{
        ves_driver_data_t *ves = (ves_driver_data_t *) part->driver_data;

        return (ves->cat_size - 1) * ves->container_data_size;
}

#endif /* dg_configNVMS_VES */
