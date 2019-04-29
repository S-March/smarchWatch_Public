/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief Boot loader for SUOTA
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <stdio.h>
#include <time.h>
#include "periph_setup.h"
#include "suota.h"
#include "hw_cpm.h"
#include "hw_uart.h"
#include "hw_watchdog.h"
#include "ad_nvms.h"
#include "flash_partitions.h"

/**
 * \brief Default values for cm_sysclk, cm_ahbclk, used by hw_cpm_delay_usec()
 *
 */
sys_clk_t cm_sysclk = sysclk_XTAL16M;
ahb_div_t cm_ahbclk = apb_div1;

/*
 * Buffer for sector needed during copy from one partition to the other.
 */
static uint8_t sector_buffer[FLASH_SECTOR_SIZE];

/*
 * Offset of image header inside partition.
 */
#define SUOTA_IMAGE_HEADER_OFFSET       0

static const uint32_t crc32_tab[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static uint32_t update_crc(uint32_t crc, const uint8_t *data, uint32_t len)
{
        while (len--) {
                crc = crc32_tab[(crc ^ *data++) & 0xff] ^ (crc >> 8);
        }
        return crc;
}

#if LOADER_UART

/*
 * If UART is configured for debugging intercept _write that is used by printf
 */
__LTO_EXT
int _write (int fd, char *ptr, int len)
{
        /* Write "len" of char from "ptr" to file id "fd"
         * Return number of char written. */
        hw_uart_write_buffer(UART_ID, ptr, len);
        return len;
}
#else

/*
 * In case of NO uart debugging just add empty implementations of printf and puts.
 */
int printf(const char *format, ...)
{
        return 0;
}

int puts(const char *format)
{
        return 0;
}

#endif

#ifdef RELEASE_BUILD
#define TRACE(...)
#else
#define TRACE(...) printf(__VA_ARGS__)
#endif

static void reboot(void)
{
        /*
         * Reset platform
         */
        __disable_irq();
        REG_SETF(CRG_TOP, SYS_CTRL_REG, SW_RESET, 1);
}

/**
 * \brief System Initialization
 *
 */
static void init(void)
{
        if (!hw_cpm_check_xtal16m_status()) {
                hw_cpm_enable_xtal16m();
                while (!hw_cpm_is_xtal16m_trimmed());
        }
        hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);
        hw_cpm_set_hclk_div(0);
        hw_cpm_set_pclk_div(0);

        hw_watchdog_freeze();                           // stop watchdog
        hw_cpm_deactivate_pad_latches();                // enable pads
        hw_cpm_power_up_per_pd();                       // exit peripheral power down
}

static bool valid_image(nvms_t header_part, nvms_t exec_part, size_t header_offset,
                                                                        bool force_crc_check)
{
        suota_1_1_image_header_t header;
        const uint8_t *mapped_ptr;
        uint32_t crc;

        if (header_part == NULL || exec_part == NULL) {
                return false;
        }

        if (sizeof(header) != ad_nvms_read(header_part, header_offset, (uint8_t *) &header,
                                                                        sizeof(header))) {
                return false;
        }

        /* Integrity check */
        if (0 == (header.flags & SUOTA_1_1_IMAGE_FLAG_VALID) ||
                                header.signature[0] != SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B1 ||
                                header.signature[1] != SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B2) {
                return false;
        }

        /*
         * Check CRC can be forced by image (then on every start CRC will be checked)
         * If it is not forced it will be checked anyway before image is copied to executable
         * partition.
         */
        if (!force_crc_check && !(header.flags & SUOTA_1_1_IMAGE_FLAG_FORCE_CRC)) {
                return true;
        }

        crc = ~0; /* Initial value of CRC prepared by mkimage */
        /*
         * Utilize QSPI memory mapping for CRC check, this way no additional buffer is needed.
         */
        if (header.code_size != ad_nvms_get_pointer(exec_part, 0, header.code_size,
                                                                (const void **) &mapped_ptr)) {
                return false;
        }
        crc = update_crc(crc, mapped_ptr, header.code_size);
        crc ^= ~0; /* Final XOR */

        return crc == header.crc;
}

static inline bool read_image_header(nvms_t part, size_t offset,
                                                                suota_1_1_image_header_t *header)
{
        return sizeof(*header) == ad_nvms_read(part, offset, (uint8_t *) header, sizeof(*header));
}

static bool update_image(nvms_t update_part, nvms_t exec_part, nvms_t header_part)
{
        suota_1_1_image_header_t new_header;
        size_t left;
        size_t src_offset;
        size_t dst_offset;
        bool result = false;

        /*
         * Erase header partition. New header will be written after executable is copied.
         */
        if (!ad_nvms_erase_region(header_part, 0, sizeof(suota_1_1_image_header_t))) {
                goto done;
        }

        if (!read_image_header(update_part, SUOTA_IMAGE_HEADER_OFFSET, &new_header)) {
                goto done;
        }

        /*
         * Erase executable partition.
         */
        if (!ad_nvms_erase_region(exec_part, 0, new_header.code_size)) {
                goto done;
        }

        left = new_header.code_size;    /* Whole image to copy */
        dst_offset = 0;                 /* Write from the beginning of executable partition */
        src_offset = SUOTA_IMAGE_HEADER_OFFSET + new_header.exec_location;

        while (left > 0) {
                size_t chunk = left > FLASH_SECTOR_SIZE ? FLASH_SECTOR_SIZE : left;

                if (chunk != ad_nvms_read(update_part, src_offset, sector_buffer, chunk)) {
                        goto done;
                }
                if (chunk != ad_nvms_write(exec_part, dst_offset, sector_buffer, chunk)) {
                        goto done;
                }

                left -= chunk;
                src_offset += chunk;
                dst_offset += chunk;
        }

        /*
         * Header is in different partition than executable.
         * Executable is at the beginning of partition, change location to 0.
         */
        new_header.exec_location = 0;

        /*
         * Write image header, so it can be used later and in subsequent reboots.
         */
        if (sizeof(new_header) != ad_nvms_write(header_part, 0, (uint8_t *) &new_header,
                                                                        sizeof(new_header))) {
                goto done;
        }

        /*
         * Invalidate image header in update partition.
         */
        new_header.flags &= ~SUOTA_1_1_IMAGE_FLAG_VALID;
        new_header.signature[0] = 0;
        new_header.signature[1] = 0;
        if (sizeof(new_header) == ad_nvms_write(update_part, SUOTA_IMAGE_HEADER_OFFSET,
                                                (uint8_t *) &new_header, sizeof(new_header))) {
                result = true;
        }
done:
        return result;
}

/*
 * Reboot platform.
 */
static void trigger_reboot(void)
{
        /*
         * Custom boot loaders should initiate recovery procedure at this point, there is no
         * valid image to run.
         */

        /* Reboot using watch dog */
        hw_watchdog_set_pos_val(1);
        hw_watchdog_gen_RST();
        hw_watchdog_unfreeze();
        for (;;) {
        }
}

static bool image_sanity_check(const int32_t *image_address)
{
        /*
         * Test reset vector for sanity:
         * - greater than image address
         * - address is odd for THUMB instruction
         */
        if (image_address[1] < (int32_t) image_address || (image_address[1] & 1) == 0) {
                return false;
        }
        return true;
}

int main(void)
{
        nvms_t update_part;
        nvms_t exec_part;
        nvms_t header_part;
        int32_t *int_vector_table = (int32_t *) 0;
        const int32_t *image_address;

        /* Initialize clocks, debugger, pad latches */
        init();

        /* Setup GPIO */
        periph_init();

        printf("\r\nBootloader started\r\n");

        /* Init VNMS, this will read partitions needed for further processing */
        ad_nvms_init();

        update_part = ad_nvms_open(NVMS_FW_UPDATE_PART);
        exec_part = ad_nvms_open(NVMS_FW_EXEC_PART);
        header_part = ad_nvms_open(NVMS_IMAGE_HEADER_PART);

        TRACE("Checking update image...\r\n");
        /* Check if there is valid image for update, check CRC */
        if (valid_image(update_part, update_part, SUOTA_IMAGE_HEADER_OFFSET, true)) {
                TRACE("Updating image...\r\n");
                if (!update_image(update_part, exec_part, header_part)) {
                        TRACE("Image update failed, rebooting\r\n");
                        trigger_reboot();
                }
        }

        /*
         * Check if current image is valid, CRC can be forced by image header but it is not
         * forced here.
         */
        if (!valid_image(header_part, exec_part, 0, false)) {
                TRACE("No valid image, rebooting\r\n");
                trigger_reboot();
        }

        /*
         * Following code assumes that code will be executed from QSPI mapped FLASH
         *
         * Binary image that is stored in QSPI flash must be compiled for specific address,
         * this address should not be 0 since this is where boot loader is stored.
         * Image stored in QSPI (except for boot loader image) does not need to be modified
         * in any way before it is flashed.
         * This image starts from initial stack pointer, and reset handler.
         * Those two value will not be copied to RAM. All other vectors will be copied from
         * image location to RAM.
         */
        if (256 != ad_nvms_get_pointer(exec_part, 0, 256, (const void **) &image_address)) {
                trigger_reboot();
        }

        /* Check sanity of image */
        if (!image_sanity_check(image_address)) {
                TRACE("Current executable is insane, rebooting\r\n");
                trigger_reboot();
        }

        TRACE("Starting image at 0x%X, reset vector 0x%X.\r\n", (unsigned int) image_address,
                                                                (unsigned int) image_address[1]);

        __disable_irq();

        /* Copy interrupt vector table from image */
        memcpy(int_vector_table, image_address, 0x100);

        /*
         * If bootloader changed any configuration (GPIO, clocks) it should be uninitialized here
         */
        periph_deinit();

        /*
         * Reset platform
         */
        reboot();
        for (;;) {
        }

        return 0;
}
