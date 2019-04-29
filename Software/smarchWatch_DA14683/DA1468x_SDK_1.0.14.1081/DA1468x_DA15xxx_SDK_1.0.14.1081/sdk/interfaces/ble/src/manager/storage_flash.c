/**
 ****************************************************************************************
 *
 * @file storage_flash.c
 *
 * @brief BLE Manager flash storage
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "FreeRTOS.h"
#include "osal.h"
#include "ble_config.h"
#include "storage.h"
#include "storage_flash.h"
#include "ad_nvms.h"

#ifdef CONFIG_BLE_STORAGE

#if !dg_configNVMS_ADAPTER
#error "dg_configNVMS_ADAPTER shall be defined for BLE Flash Storage!"
#endif

#if !dg_configNVMS_VES
#error "dg_configNVMS_VES shall be defined for BLE Flash Storage!"
#endif

// compile-time assertion
#define C_ASSERT(cond) typedef char __c_assert[(cond) ? 1 : -1] __attribute__((unused))


#ifndef CONFIG_BLE_STORAGE_KEY_PART_OFFSET
#define CONFIG_BLE_STORAGE_KEY_PART_OFFSET (0x00)
#endif

#ifndef CONFIG_BLE_STORAGE_APV_PART_OFFSET
#define CONFIG_BLE_STORAGE_APV_PART_OFFSET (0x500)
#endif

#ifndef CONFIG_BLE_STORAGE_APV_PART_LENGTH
#define CONFIG_BLE_STORAGE_APV_PART_LENGTH (1024)
#endif

#define PART_KEY_DATA_OFFSET        (CONFIG_BLE_STORAGE_KEY_PART_OFFSET)
#define PART_APV_DATA_OFFSET        (CONFIG_BLE_STORAGE_APV_PART_OFFSET)
#define PART_APV_DATA_LENGTH        (CONFIG_BLE_STORAGE_APV_PART_LENGTH)

#define PART_KEY_LENGTH             (sizeof(STORAGE_MAGIC_KEY) + sizeof(uint8_t) + \
                                     sizeof(stored_device_t) * defaultBLE_MAX_BONDED)

/*
 * Magic values to identify that partition area contains valid BLE data
 *
 * Two magic values are defined: for keys section and app values section.
 *
 * 0x00 is used for storage versioning - any change to this byte will cause existing data to be
 * considered invalid and won't be loaded from flash. This can be used in case storage format is
 * changed for some reason.
 */
static const uint8_t STORAGE_MAGIC_KEY[8] = { 'B', 'L', 'E', '_', 'K', 'E', 'Y', 0x01 };
static const uint8_t STORAGE_MAGIC_APV[8] = { 'B', 'L', 'E', '_', 'A', 'P', 'V', 0x01 };

enum {
        DEV_FLAG_FREE                   = 0x0001,
        DEV_FLAG_HAS_LTK                = 0x0002,
        DEV_FLAG_HAS_REMOTE_LTK         = 0x0004,
        DEV_FLAG_HAS_IRK                = 0x0008,
        DEV_FLAG_HAS_CSRK               = 0x0010,
        DEV_FLAG_HAS_REMOTE_CSRK        = 0x0020,
        DEV_FLAG_MITM                   = 0x0040,
        DEV_FLAG_SECURE                 = 0x0080,
};

enum {
        APV_TYPE_EMPTY          = 0,
        APV_TYPE_ADDRESS        = 1,
        APV_TYPE_INTEGER        = 2,
        APV_TYPE_BUFFER         = 3,
};

/*
 * Device structure stored onto flash
 *
 * This is dumped as-is thus should not contain any pointers and not be modified without good reason
 * to maintain compatibility. If this structure is changed, then STORAGE_MAGIC should be also changed
 * to invalidate any existing data.
 */
typedef struct {
        /*
         * flags field should be always 1st field in this structure since it's the only field
         * written to flash when removing device
         */
        uint32_t        flags;

        bd_address_t    addr;

        key_ltk_t       ltk;
        key_ltk_t       remote_ltk;
        key_irk_t       irk;
        key_csrk_t      csrk;
        key_csrk_t      remote_csrk;
} stored_device_t;

PRIVILEGED_DATA static nvms_t part;     // partition handle

/* Calculates partition offset for device at index */
static inline uint32_t get_addr(uint32_t index)
{
        return PART_KEY_DATA_OFFSET + sizeof(STORAGE_MAGIC_KEY) + sizeof(uint8_t) +
                                                                index * sizeof(stored_device_t);
}

#define DUPLICATE_KEY(KEY)                              \
                ({                                      \
                        void *buf;                      \
                        buf = OS_MALLOC(sizeof(KEY));   \
                        memcpy(buf, &KEY, sizeof(KEY)); \
                        buf;                            \
                })

static void convert_stored_dev_to_dev(const stored_device_t *s_dev, device_t *dev)
{
        memcpy(&dev->addr, &s_dev->addr, sizeof(dev->addr));

        dev->paired = true;
        dev->bonded = true;
        dev->mitm = s_dev->flags & DEV_FLAG_MITM;
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        dev->secure = s_dev->flags & DEV_FLAG_SECURE;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

        if (s_dev->flags & DEV_FLAG_HAS_LTK) {
                dev->ltk = DUPLICATE_KEY(s_dev->ltk);
        }

        if (s_dev->flags & DEV_FLAG_HAS_REMOTE_LTK) {
                dev->remote_ltk = DUPLICATE_KEY(s_dev->remote_ltk);
        }

        if (s_dev->flags & DEV_FLAG_HAS_IRK) {
                dev->irk = DUPLICATE_KEY(s_dev->irk);
        }

        if (s_dev->flags & DEV_FLAG_HAS_CSRK) {
                dev->csrk = DUPLICATE_KEY(s_dev->csrk);
        }

        if (s_dev->flags & DEV_FLAG_HAS_REMOTE_CSRK) {
                dev->remote_csrk = DUPLICATE_KEY(s_dev->remote_csrk);
        }
}

static void convert_dev_to_stored_dev(const device_t *dev, stored_device_t *s_dev)
{
        s_dev->flags = 0;

        memcpy(&s_dev->addr, &dev->addr, sizeof(s_dev->addr));

        if (dev->ltk) {
                s_dev->flags |= DEV_FLAG_HAS_LTK;
                memcpy(&s_dev->ltk, dev->ltk, sizeof(s_dev->ltk));
        }

        if (dev->remote_ltk) {
                s_dev->flags |= DEV_FLAG_HAS_REMOTE_LTK;
                memcpy(&s_dev->remote_ltk, dev->remote_ltk, sizeof(s_dev->remote_ltk));
        }

        if (dev->irk) {
                s_dev->flags |= DEV_FLAG_HAS_IRK;
                memcpy(&s_dev->irk, dev->irk, sizeof(s_dev->irk));
        }

        if (dev->csrk) {
                s_dev->flags |= DEV_FLAG_HAS_CSRK;
                memcpy(&s_dev->csrk, dev->csrk, sizeof(s_dev->csrk));
        }

        if (dev->remote_csrk) {
                s_dev->flags |= DEV_FLAG_HAS_REMOTE_CSRK;
                memcpy(&s_dev->remote_csrk, dev->remote_csrk, sizeof(s_dev->remote_csrk));
        }

        if (dev->mitm) {
                s_dev->flags |= DEV_FLAG_MITM;
        }

#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        if (dev->secure) {
                s_dev->flags |= DEV_FLAG_SECURE;
        }
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
}

/* Helper to read NVMS and advance address */
static inline uint32_t nvms_read_inc(uint32_t addr, void *ptr, size_t length)
{
        ad_nvms_read(part, addr, ptr, length);

        return addr + length;
}

/* Helper to write NVMS and advance address */
static inline uint32_t nvms_write_inc(uint32_t addr, const void *ptr, size_t length)
{
        ad_nvms_write(part, addr, ptr, length);

        return addr + length;
}

static void load_part_key(void)
{
        uint8_t magic[ sizeof(STORAGE_MAGIC_KEY) ];
        uint8_t bonded_count;
        uint32_t index;

        /*
         * First need to verify that magic value stored in partition matches, otherwise we don't
         * load anything.
         */
        ad_nvms_read(part, PART_KEY_DATA_OFFSET, magic, sizeof(magic));

        if (memcmp(magic, STORAGE_MAGIC_KEY, sizeof(magic))) {
                return;
        }

        ad_nvms_read(part, PART_KEY_DATA_OFFSET + sizeof(STORAGE_MAGIC_KEY), &bonded_count,
                                                                        sizeof(bonded_count));

        /*
         * If there are more bonded devices written on flash than supported in current configuratio
         * we just discard oldest entries from flash.
         */
        if (bonded_count > defaultBLE_MAX_BONDED) {
                bonded_count = defaultBLE_MAX_BONDED;
        }

        for (index = 0; index < bonded_count; index++) {
                stored_device_t s_dev;
                device_t *dev;
                uint32_t addr;

                addr = get_addr(index);

                ad_nvms_read(part, addr, (uint8_t *) &s_dev, sizeof(s_dev));

                if (s_dev.flags & DEV_FLAG_FREE) {
                        continue;
                }

                dev = find_device_by_addr(&s_dev.addr, true);
                if (!dev) {
                        OS_ASSERT(0);
                        continue;
                }

                /*
                 * Remove any existing pairing information for new device, this will be overwritten
                 * by data loaded from partition.
                 */
                device_remove_pairing(dev);

                convert_stored_dev_to_dev(&s_dev, dev);
        }
}

static void load_part_apv(void)
{
        uint8_t magic[ sizeof(STORAGE_MAGIC_APV) ];
        uint32_t addr;
        device_t *dev = NULL;

        /*
         * First need to verify that magic value stored in partition matches, otherwise we don't
         * load anything.
         */
        ad_nvms_read(part, PART_APV_DATA_OFFSET, magic, sizeof(magic));

        if (memcmp(magic, STORAGE_MAGIC_APV, sizeof(magic))) {
                return;
        }

        addr = PART_APV_DATA_OFFSET + sizeof(STORAGE_MAGIC_KEY);

        for (;;) {
                uint8_t apv_type;

                addr = nvms_read_inc(addr, &apv_type, sizeof(apv_type));

                if (apv_type == APV_TYPE_EMPTY) {
                        /*
                         * empty element indicates end of data so just return from here
                         */
                        return;
                }

                switch (apv_type) {
                case APV_TYPE_ADDRESS:
                {
                        bd_address_t bd_addr;

                        addr = nvms_read_inc(addr, &bd_addr, sizeof(bd_addr));

                        /*
                         * don't create if it does not exists - since apvs are loaded after bonded
                         * device were loaded, any bonded device should be already created, otherwise
                         * there are data for non-bonded device stored which we just should ignore
                         */
                        dev = find_device_by_addr(&bd_addr, false);
                        break;
                }
                case APV_TYPE_INTEGER:
                {
                        ble_storage_key_t key;
                        int val;

                        if (!dev) {
                                addr += sizeof(key) + sizeof(val);
                                break;
                        }

                        addr = nvms_read_inc(addr, &key, sizeof(key));
                        addr = nvms_read_inc(addr, &val, sizeof(val));

                        app_value_put(dev, key, 0, (void *) val, NULL, true);

                        break;
                }
                case APV_TYPE_BUFFER:
                {
                        ble_storage_key_t key;
                        uint16_t len;
                        void *ptr;

                        addr = nvms_read_inc(addr, &key, sizeof(key));
                        addr = nvms_read_inc(addr, &len, sizeof(len));

                        if (!dev) {
                                addr += len;
                                break;
                        }

                        ptr = OS_MALLOC_NORET(len);

                        addr = nvms_read_inc(addr, ptr, len);

                        app_value_put(dev, key, len, ptr, OS_FREE_NORET_FUNC, true);

                        break;
                }
                default:
                        /*
                         * we don't know how to handle this type so need to return because otherwise
                         * we'll probably read some garbage
                         */
                        OS_ASSERT(0);
                        return;
                }
        }
}

static void dump_device_key_func(device_t *dev, void *ud)
{
        uint32_t *index = ud;
        /*
         * saving data to flash is synchornized using mutex so it's safe to save some stack space
         * by making this variable static - structure is quite big.
         */
        static stored_device_t s_dev;
        uint32_t addr;

        /*
         * This is just in case somehow we have more bonded devices on list than allowed.
         */
        if (*index >= defaultBLE_MAX_BONDED) {
                return;
        }

        // we store only bonded devices
        if (!dev->bonded) {
                return;
        }

        addr = get_addr(*index);

        /*
         * to minimize writes, just read current device before writing new information there
         * it may happend that e.g. only flags has to be changed so there's no point in writing
         * keys again
         */
        ad_nvms_read(part, addr, (uint8_t *) &s_dev, sizeof(s_dev));

        convert_dev_to_stored_dev(dev, &s_dev);

        ad_nvms_write(part, addr, (const uint8_t *) &s_dev, sizeof(s_dev));

        (*index)++;
}

static void dump_apv_func(void *data, void *ud)
{
        const app_value_t *appval = data;
        uint32_t *addr_p = ud;
        uint32_t addr;
        uint8_t apv_type;
        size_t free_space;

        addr = *addr_p;

        // -1 byte because there should always be free space to hold 'empty' apv
        free_space = PART_APV_DATA_OFFSET + PART_APV_DATA_LENGTH - addr - 1;

        if (appval->length) {
                // ensure there's free space to write complete apv
                if (free_space < sizeof(apv_type) + sizeof(appval->key) +
                                                        sizeof(appval->length) + appval->length) {
                        return;
                }

                apv_type = APV_TYPE_BUFFER;
                addr = nvms_write_inc(addr, &apv_type, sizeof(apv_type));
                addr = nvms_write_inc(addr, &appval->key, sizeof(appval->key));
                addr = nvms_write_inc(addr, &appval->length, sizeof(appval->length));
                addr = nvms_write_inc(addr, appval->ptr, appval->length);
        } else {
                // ensure there's free space to write complete apv
                if (free_space < sizeof(apv_type) + sizeof(appval->key) + sizeof(appval->ptr)) {
                        return;
                }

                apv_type = APV_TYPE_INTEGER;
                addr = nvms_write_inc(addr, &apv_type, sizeof(apv_type));
                addr = nvms_write_inc(addr, &appval->key, sizeof(appval->key));
                addr = nvms_write_inc(addr, &appval->ptr, sizeof(appval->ptr));
        }

        *addr_p = addr;
}

static void dump_device_apv_func(device_t *dev, void *ud)
{
        uint32_t *addr_p = ud;
        uint32_t addr;
        uint8_t apv_type;
        size_t free_space;

        // we store only bonded devices
        if (!dev->bonded) {
                return;
        }

        addr = *addr_p;

        free_space = PART_APV_DATA_OFFSET + PART_APV_DATA_LENGTH - addr;

        // ensure there's at least free space to write complete address data
        if (free_space < sizeof(apv_type) + sizeof(dev->addr)) {
                return;
        }

        apv_type = APV_TYPE_ADDRESS;
        addr = nvms_write_inc(addr, &apv_type, sizeof(apv_type));
        addr = nvms_write_inc(addr, &dev->addr, sizeof(dev->addr));

        queue_foreach(&dev->app_value, dump_apv_func, &addr);

        *addr_p = addr;
}

static void save_part_key(void)
{
        uint32_t index = 0;
        uint8_t bonded_count = defaultBLE_MAX_BONDED;

        // write magic to indicate that storage is valid
        ad_nvms_write(part, PART_KEY_DATA_OFFSET, STORAGE_MAGIC_KEY, sizeof(STORAGE_MAGIC_KEY));

        // write all devices
        device_foreach(dump_device_key_func, &index);

        // clean remaining devices
        while (index < defaultBLE_MAX_BONDED) {
                stored_device_t s_dev;
                uint32_t addr;

                addr = get_addr(index);

                // to minimize writes, just read current flags for device and write-back new flags
                ad_nvms_read(part, addr, (uint8_t *) &s_dev, sizeof(s_dev.flags));

                s_dev.flags |= DEV_FLAG_FREE;

                ad_nvms_write(part, addr, (const uint8_t *) &s_dev, sizeof(s_dev.flags));

                index++;
        }

        /*
         * Write max bonded devices in current configuration to flash. This allows to increase this
         * number in future in backwards-compatible way. Also this number is written as last step
         * here so it's only updated if all entries in flash are updated.
         */
        ad_nvms_write(part, PART_KEY_DATA_OFFSET + sizeof(STORAGE_MAGIC_KEY), &bonded_count,
                                                                        sizeof(bonded_count));
}

static void save_part_apv(void)
{
        uint32_t addr;
        uint8_t apv_type;

        // write magic to indicate that storage is valid
        ad_nvms_write(part, PART_APV_DATA_OFFSET, STORAGE_MAGIC_APV, sizeof(STORAGE_MAGIC_APV));

        addr = PART_APV_DATA_OFFSET + sizeof(STORAGE_MAGIC_KEY);

        // write all devices
        device_foreach(dump_device_apv_func, &addr);

        // add marker at the end
        apv_type = APV_TYPE_EMPTY;
        ad_nvms_write(part, addr, &apv_type, sizeof(apv_type));
}
#endif // CONFIG_BLE_STORAGE

void storage_flash_init(void)
{
#ifdef CONFIG_BLE_STORAGE
        /* compile-time assertion in APV area overlaps KEY area (assuming APV is placed after KEY) */
        C_ASSERT(PART_KEY_DATA_OFFSET + PART_KEY_LENGTH < PART_APV_DATA_OFFSET);

        part = ad_nvms_open(NVMS_GENERIC_PART);
        if (!part) {
                OS_ASSERT(0);
                return;
        }
#endif // CONFIG_BLE_STORAGE
}

void storage_flash_load(void)
{
#ifdef CONFIG_BLE_STORAGE
        if (!part) {
                return;
        }

        load_part_key();
        load_part_apv();
#endif // CONFIG_BLE_STORAGE
}

void storage_flash_save(void)
{
#ifdef CONFIG_BLE_STORAGE
        if (!part) {
                return;
        }

        save_part_key();
        save_part_apv();
#endif // CONFIG_BLE_STORAGE
}
