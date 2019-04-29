/**
 ****************************************************************************************
 *
 * @file ad_nvparam.c
 *
 * @brief NV-Parameters adapter
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configNVPARAM_ADAPTER

#include <sys_power_mgr.h>
#include <osal.h>
#include <ad_nvms.h>
#include <ad_nvparam.h>

enum {
        FLAG_VARIABLE_LEN = 0x01,       // parameter has variable length
};

typedef struct {
        uint8_t tag;                    // unique parameter tag, user has to ensure the unique value
        uint8_t flags;                  // parameter flags
        uint16_t length;                // parameter max length
        uint32_t offset;                // parameter offset inside area
} parameter_t;

typedef struct {
        const char *name;               // unique area name, user has to ensure the unique name
        nvms_partition_id_t partition;  // area partition
        uint32_t offset;                // area offset inside partition
        const parameter_t *parameters;  // list of area parameters
        size_t num_parameters;          // number of area parameters
} area_t;

typedef struct {
        const area_t *area;             // attached area configuration
        nvms_t nvms_h;                  // NVMS handle
} nvparam_data_t;

/* Create nvparam configuration from ad_nvparam_defs.h */
#define IN_AD_NVPARAM_C
#include <ad_nvparam_defs.h>

static const parameter_t *find_parameter(const area_t *area, uint8_t tag)
{
        int i;

        for (i = 0; i < area->num_parameters; i++) {
                if (area->parameters[i].tag == tag) {
                        return &area->parameters[i];
                }
        }

        return NULL;
}

nvparam_t ad_nvparam_open(const char *area_name)
{
        int i;
        nvms_t *nvms_h;
        const area_t *area = NULL;
        nvparam_data_t *nv_data;

        for (i = 0; i < num_areas; i++) {
                if (!strcmp(areas[i].name, area_name)) {
                        area = &areas[i];
                        break;
                }
        }

        if (!area) {
                return NULL;
        }

        nvms_h = ad_nvms_open(area->partition);
        if (!nvms_h) {
                return NULL;
        }

        nv_data = OS_MALLOC(sizeof(*nv_data));
        OS_ASSERT(nv_data);

        nv_data->area = area;
        nv_data->nvms_h = nvms_h;

        return (nvparam_t) nv_data;
}

void ad_nvparam_close(nvparam_t nvparam)
{
        if (nvparam) {
                OS_FREE(nvparam);
        }
}

void ad_nvparam_erase_all(nvparam_t nvparam)
{
        nvparam_data_t *nv_data = nvparam;
        size_t size;
        uint8_t *write_buf;
        int i;

        if (!nvparam) {
                return;
        }

        for (i = 0; i < nv_data->area->num_parameters; i++) {
                uint32_t addr = nv_data->area->offset + nv_data->area->parameters[i].offset;
                size = nv_data->area->parameters[i].length;
                write_buf = OS_MALLOC(size * sizeof(uint8_t));
                OS_ASSERT(write_buf);
                memset(write_buf, 0xFF, size);

                ad_nvms_write(nv_data->nvms_h, addr, write_buf, size);

                OS_FREE(write_buf);
        }
}

void ad_nvparam_erase(nvparam_t nvparam, uint8_t tag)
{
        nvparam_data_t *nv_data = nvparam;
        const parameter_t *param;
        size_t size;
        uint8_t *write_buf;

        if (!nvparam) {
                return;
        }

        param = find_parameter(nv_data->area, tag);
        if (!param) {
                return;
        }

        size = param->length;
        write_buf = OS_MALLOC(size * sizeof(uint8_t));
        OS_ASSERT(write_buf);
        memset(write_buf, 0xFF, size);

        ad_nvms_write(nv_data->nvms_h, nv_data->area->offset + param->offset, write_buf, size);

        OS_FREE(write_buf);
}

uint16_t ad_nvparam_read_offset(nvparam_t nvparam, uint8_t tag, uint16_t offset,
                                                                uint16_t length, void *data)
{
        nvparam_data_t *nv_data = nvparam;
        const parameter_t *param;
        uint32_t param_offset;
        uint16_t max_length;

        if (!nvparam) {
                return 0;
        }

        param = find_parameter(nv_data->area, tag);
        if (!param) {
                return 0;
        }

        param_offset = nv_data->area->offset + param->offset;
        max_length = param->length;

        if (param->flags & FLAG_VARIABLE_LEN) {
                uint16_t stored_len;
                int read_len;

                /* read current parameter length first */
                read_len = ad_nvms_read(nv_data->nvms_h, param_offset, (uint8_t *) &stored_len, 2);

                /*
                 * 1) check if read was successful (2 bytes were read)
                 * 2) in case of 0xFFFF is read as parameter length, we assume this is due to clean
                 *    flash and treat it as 0x0000
                 */
                if (read_len != 2 || stored_len == 0xffff) {
                        return 0;
                }

                param_offset += read_len;
                max_length = stored_len;
        }

        /* Do not allow reading past max_length */
        if (offset >= max_length) {
                return 0;
        }

        /* truncate read to maximum length, including offset */
        max_length -= offset;
        if (length > max_length) {
                length = max_length;
        }

        param_offset += offset;
        return ad_nvms_read(nv_data->nvms_h, param_offset, (uint8_t *) data, length);
}

uint16_t ad_nvparam_write(nvparam_t nvparam, uint8_t tag, uint16_t length, const void *data)
{
        nvparam_data_t *nv_data = nvparam;
        const parameter_t *param;
        uint32_t offset;

        if (!nvparam) {
                return 0;
        }

        param = find_parameter(nv_data->area, tag);
        if (!param) {
                return 0;
        }

        offset = nv_data->area->offset + param->offset;

        /* truncate write to maximum length */
        if (length > param->length) {
                length = param->length;
        }

        if (param->flags & FLAG_VARIABLE_LEN) {
                int written;

                /* maximum length must be subtracted by 2 which are occupied by length parameter */
                if (length > param->length - 2) {
                        length = param->length - 2;
                }

                /* write parameter length first */
                written = ad_nvms_write(nv_data->nvms_h, offset, (uint8_t *) &length, 2);
                if (written != 2) {
                        return 0;
                }

                offset += 2;
        }

        return ad_nvms_write(nv_data->nvms_h, offset, data, length);
}

uint16_t ad_nvparam_get_length(nvparam_t nvparam, uint8_t tag, uint16_t *max_length)
{
        nvparam_data_t *nv_data = nvparam;
        const parameter_t *param;
        uint32_t offset;
        uint16_t length;
        int read;

        if (!nvparam) {
                return 0;
        }

        param = find_parameter(nv_data->area, tag);
        if (!param) {
                return 0;
        }

        if (max_length) {
                *max_length = param->length;
                /*
                 * for variable length parameters max_length should be shorter about 2 bytes which
                 * are intend for parameter length
                 */
                if (param->flags & FLAG_VARIABLE_LEN) {
                        *max_length -= 2;
                }
        }

        /* for non-variable length parameters length is just the same as max length */
        if (!(param->flags & FLAG_VARIABLE_LEN)) {
                return param->length;
        }

        offset = nv_data->area->offset + param->offset;

        read = ad_nvms_read(nv_data->nvms_h, offset, (uint8_t *) &length, 2);

        /*
         * 1) check if read was successful (2 bytes were read)
         * 2) in case of 0xFFFF is read as parameter length, we assume this is due to clean flash
         *    and treat it as 0x0000
         */
        if (read != 2 || length == 0xFFFF) {
                return 0;
        }

        /*
         * check if read length is grater than parameter length defined by user reduced by 2 bytes
         * which are intended for its length
         */
        if (length > param->length - 2) {
                OS_ASSERT(0);
                /* in Release mode return 0 to avoid reading garbage */
                return 0;
        }

        return length;
}

ADAPTER_INIT_DEP1(ad_nvparam_adapter, NULL, ad_nvms_adapter);

#endif /* dg_configNVPARAM_ADAPTER */
