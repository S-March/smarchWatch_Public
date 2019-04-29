/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief UART bootloader
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup Boot-loader
 * \{
 */
#include <stdio.h>
#include <string.h>
#include <sdk_defs.h>
#include <hw_gpio.h>
#include <hw_uart.h>
#include <hw_timer1.h>
#include <hw_otpc.h>
#include <hw_qspi.h>
#include <hw_watchdog.h>
#include <hw_cpm.h>
#include <ad_flash.h>
#include <ad_nvms.h>
#include <ad_nvms_ves.h>
#include <ad_nvms_direct.h>
#include <flash_partitions.h>
#include "crc16.h"
#include "uartboot_types.h"

#define BOOTUART (HW_UART2)
#define BOOTUART_STEP                   3

#define glue(a,b) a##b
#define BAUDRATE_CONST(b) glue(HW_UART_BAUDRATE_, b)
#define BAUDRATE_CFG BAUDRATE_CONST(BAUDRATE)

#if dg_configBLACK_ORCA_MB_REV == BLACK_ORCA_MB_REV_D
#define CFG_GPIO_BOOTUART_TX_PORT       HW_GPIO_PORT_1
#define CFG_GPIO_BOOTUART_TX_PIN        HW_GPIO_PIN_3
#define CFG_GPIO_BOOTUART_RX_PORT       HW_GPIO_PORT_2
#define CFG_GPIO_BOOTUART_RX_PIN        HW_GPIO_PIN_3
#else
#define CFG_GPIO_BOOTUART_TX_PORT       HW_GPIO_PORT_1
#define CFG_GPIO_BOOTUART_TX_PIN        HW_GPIO_PIN_0
#define CFG_GPIO_BOOTUART_RX_PORT       HW_GPIO_PORT_1
#define CFG_GPIO_BOOTUART_RX_PIN        HW_GPIO_PIN_5
#endif

#define VERSION (0x0003) // BCD

#define SOH '\x01'
#define STX '\x02'
#define ACK '\x06'
#define NAK '\x15'

#define TMO_COMMAND     (2)
#define TMO_DATA        (5)
#define TMO_ACK         (3)

/*
 * this is 'magic' address which can be used in some commands to indicate some kind of temporary
 * storage, i.e. command needs to store some data but does not care where as long as it can be
 * accessed later
 */
#define ADDRESS_TMP     (0xFFFFFFFF)

/*
 *
 */
#define VIRTUAL_BUF_ADDRESS   (0x80000000)
#define VIRTUAL_BUF_MASK      (0xFFFC0000)

/* compile-time assertion */
#define C_ASSERT(cond) typedef char __c_assert[(cond) ? 1 : -1] __attribute__((unused))

#define UARTBOOT_LIVE_MARKER            "Live"

extern uint8_t __inputbuffer_start; // start of .inputbuffer section
extern uint8_t __inputbuffer_end;

/*
 * a complete flow for transmission handling (including in/out data) is as follows:
 *
 * <= <STX> <SOH> (ver1) (ver2)
 * => <SOH>
 * => (type) (len1) (len2)
 * call HOP_INIT
 * <= <ACK> / <NAK>
 * if len > 0
 *      => (data...)
 *      call HOP_DATA
 *      <= <ACK> / <NAK>
 *      <= (crc1) (crc2)
 *      => <ACK> / <NAK>
 * call HOP_EXEC
 * call HOP_SEND_LEN
 * if len > 0
 *      <= (len1) (len2)
 *      => <ACK> / <NAK>
 *      call HOP_SEND_DATA
 *      <= (data...)
 *      => (crc1) (crc2)
 *      <= <ACK> / <NAK>
 */

/* call type for command handler */
typedef enum {
        HOP_INIT,       // command header is received, i.e. type and length of incoming data
                        //      return false to NAK
        HOP_HEADER,     // full header is received
                        //      return false to NAK
        HOP_DATA,       // command data is received
                        //      return false to NAK
        HOP_EXEC,       // complete command data is received
                        //      return false to NAK
        HOP_SEND_LEN,   // need to send outgoing data length - use xmit_data()
                        //      return false if no data to be sent
        HOP_SEND_DATA,  // called for handler send data back - use xmit_data()
                        //      return false to abort
} HANDLER_OP;

/* UART configuration */
static uart_config UART_INIT = {
                .baud_rate              = HW_UART_BAUDRATE_57600,
                .data                   = HW_UART_DATABITS_8,
                .parity                 = HW_UART_PARITY_NONE,
                .stop                   = HW_UART_STOPBITS_1,
                .auto_flow_control      = 0,
                .use_dma                = 0,
                .use_fifo               = 1,
                .tx_dma_channel         = 0,
                .rx_dma_channel         = 0,
};

static uint8_t uart_buf[32];                // buffer for incoming data (control data only)

static volatile bool timer1_soh_tmo = true; // timeout waiting for SOH flag

static volatile bool uart_soh = false;      // UART waiting for SOH flag

static bool uart_tmo = false;               // timeout waiting for data from UART

static volatile uint16_t tick = 0;          // 1s tick counter

static volatile uint16_t uart_data_len = 0; // length of data received from UART

static bool ad_nvms_init_called = false;    // ad_nvms_init() should be called once and only if needed

/**
 * \brief Default values for cm_sysclk, cm_ahbclk, used by hw_cpm_delay_usec()
 *
 */
sys_clk_t cm_sysclk = sysclk_XTAL16M;
ahb_div_t cm_ahbclk = apb_div1;

struct cmdhdr_send_to_ram {
        uint8_t *ptr;
} __attribute__((packed));

struct cmdhdr_read_from_ram {
        uint8_t *ptr;
        uint16_t len;
} __attribute__((packed));

struct cmdhdr_write_ram_to_qspi {
        uint8_t *ptr;
        uint16_t len;
        uint32_t addr;
} __attribute__((packed));

struct cmdhdr_erase_qspi {
        uint32_t addr;
        uint32_t len;
} __attribute__((packed));

struct cmdhdr_execute_code {
        uint32_t addr;
} __attribute__((packed));

struct cmdhdr_write_otp {
        uint32_t addr;
} __attribute__((packed));

struct cmdhdr_read_otp {
        uint32_t addr;
        uint16_t len;
} __attribute__((packed));

struct cmdhdr_read_qspi {
        uint32_t addr;
        uint16_t len;
} __attribute__((packed));

struct cmdhdr_read_partition {
        uint32_t addr;
        uint16_t len;
        nvms_partition_id_t id;
} __attribute__((packed));

struct cmdhdr_write_partition {
        uint8_t *ptr;
        uint16_t len;
        uint32_t addr;
        nvms_partition_id_t id;
} __attribute__((packed));

struct cmdhdr_get_version {
} __attribute__((packed));

struct cmdhdr_is_empty_qspi {
        uint32_t size;
        uint32_t start_address;
} __attribute__((packed));

/*
 * union of all cmdhdr structures, this is used to create buffer to which command header will be
 * loaded so we can safely use payload buffer to keep data between commands
 */
union cmdhdr {
        struct cmdhdr_send_to_ram send_to_ram;
        struct cmdhdr_read_from_ram read_from_ram;
        struct cmdhdr_write_ram_to_qspi write_ram_to_qspi;
        struct cmdhdr_erase_qspi erase_qspi;
        struct cmdhdr_execute_code execute_code;
        struct cmdhdr_write_otp write_otp;
        struct cmdhdr_read_otp read_otp;
        struct cmdhdr_read_qspi read_qspi;
        struct cmdhdr_read_partition read_partition;
        struct cmdhdr_write_partition write_partition;
        struct cmdhdr_get_version get_version;
        struct cmdhdr_is_empty_qspi is_empty_qspi;
};

/* state of incoming command handler */
static struct cmd_state {
        uint8_t type;                           // type of command being handled
        uint16_t len;                           // command length (header and payload)
        union cmdhdr hdr;                       // command header
        uint16_t hdr_len;                       // command header length
        uint8_t *data;                          // command payload
        uint16_t data_len;                      // command payload length
        bool (* handler) (HANDLER_OP);          // command handler;

        uint16_t crc;                           // CRC of transmitted data;
} cmd_state;

typedef struct {
        char magic[4];
        volatile uint32_t run_swd;     /* This is set to 1 by debugger to enter SWD mode */
        volatile uint32_t cmd_num;     /* Debugger command sequence number, this field is
                                          incremented by debugger after arguments in uart_buf have
                                          been set for new command. Bootloader starts interpreting
                                          command when this number changes. This will prevent
                                          executing same command twice by accident */
        uint8_t *cmd_hdr_buf;          /* buffer for header stored here for debugger to see */
        uint8_t *buf;                  /* Big buffer for data transfer */
        volatile uint32_t ack_nak;     /* ACK or NAK for swd command */
} swd_interface_t;

const swd_interface_t swd_interface __attribute__ ((section (".swd_section"))) = {
        "DBGP", /* This marker is for debugger to search for swd_interface structure in memory */
        0,
        0,
        uart_buf,
        &__inputbuffer_start
};

/**
 * \brief Translate 'magic' addresses into actual memory location
 *
 * \param [in,out] addr memory address
 *
 */
static inline void translate_ram_addr(uint32_t *addr)
{
        /*
         * ADDRESS_TMP will point to inputbuffer which is large enough to hold all received data
         * and it's not necessary to move data around since they are already received into this
         * buffer
         */
        if (*addr == ADDRESS_TMP) {
                *addr = (uint32_t) &__inputbuffer_start;
        } else if ((*addr & VIRTUAL_BUF_MASK) == VIRTUAL_BUF_ADDRESS) {
                *addr = (*addr & ~VIRTUAL_BUF_MASK) + (uint32_t) &__inputbuffer_start;
        }
}

static void timer1_soh_cb(void)
{
        hw_uart_abort_receive(BOOTUART);
        timer1_soh_tmo = true;
}

static void uart_soh_cb(uint8_t *data, uint16_t len)
{
        if (len == 1 && data[0] == SOH) {
                uart_soh = true;
        }
}

static void timer1_tick_cb(void)
{
        tick++;
}

static void uart_data_cb(void *user_data, uint16_t len)
{
        uart_data_len = len;
}

static inline void xmit_hello(void)
{
        static const uint8_t msg[] = {
                        STX, SOH,
                        (VERSION & 0xF0) >> 8, VERSION & 0x0F };

        hw_uart_send(BOOTUART, msg, sizeof(msg), NULL, NULL);
}

static inline void xmit_ack(void)
{
        if (swd_interface.run_swd) {
                *((uint32_t *) &swd_interface.ack_nak) = ACK;
                return;
        }
        hw_uart_write(BOOTUART, ACK);
}

static inline void xmit_nak(void)
{
        if (swd_interface.run_swd) {
                *((uint32_t *) &swd_interface.ack_nak) = NAK;
                return;
        }
        hw_uart_write(BOOTUART, NAK);
}

static inline void xmit_crc16(uint16_t crc16)
{
        hw_uart_send(BOOTUART, (void *) &crc16, sizeof(crc16), NULL, NULL);
}

static inline void xmit_data(const uint8_t *buf, uint16_t len)
{
        uint8_t byt;
        uint16_t i;

        for (i = 0; i < len; ++i) {
                byt = buf[i];
                hw_uart_write(BOOTUART, byt);
                crc16_update(&cmd_state.crc, &byt, 1);
        }
}

static bool recv_with_tmo(uint8_t *buf, uint16_t len, uint16_t tmo)
{
        if (!len) {
                return true;
        }

        tick = 0;
        uart_data_len = 0;
        uart_tmo = false;

        hw_timer1_register_int(timer1_tick_cb);
        hw_timer1_enable();

        hw_uart_receive(BOOTUART, buf, len, uart_data_cb, NULL);

        while (tick < tmo && uart_data_len == 0) {
                __WFI();
        }

        hw_timer1_disable();

        /* abort if no data received */
        if (uart_data_len == 0) {
                uart_tmo = true;
                hw_uart_abort_receive(BOOTUART);
        }

        return !uart_tmo;
}

static uint16_t push_partition_entry_name(uint8_t *ram, nvms_partition_id_t id)
{
#define _STR_(token) #token
#define _PUSH_ENUM_AS_STRING_2_RAM_(_enum_)      \
        do {                                     \
                len = strlen(_STR_(_enum_)) + 1; \
                memcpy(ram, _STR_(_enum_), len); \
        } while (0);
#define _ALIGN32_(size) (((size) + 3) & (~0x3))

        uint16_t len;

        switch (id) {
        case NVMS_FIRMWARE_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FIRMWARE_PART);
                break;

        case NVMS_PARAM_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PARAM_PART);
                break;

        case NVMS_BIN_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_BIN_PART);
                break;

        case NVMS_LOG_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_LOG_PART);
                break;

        case NVMS_GENERIC_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_GENERIC_PART);
                break;

        case NVMS_PLATFORM_PARAMS_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PLATFORM_PARAMS_PART);
                break;

        case NVMS_PARTITION_TABLE:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PARTITION_TABLE);
                break;

        case NVMS_FW_EXEC_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FW_EXEC_PART);
                break;

        case NVMS_FW_UPDATE_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FW_UPDATE_PART);
                break;

        case NVMS_PRODUCT_HEADER_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PRODUCT_HEADER_PART);
                break;

        case NVMS_IMAGE_HEADER_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_IMAGE_HEADER_PART);
                break;

        default:
                _PUSH_ENUM_AS_STRING_2_RAM_(UNKNOWN_PARTITION_ID);
        }

        /* len should be multiple of 4 to avoid unaligned loads/stores */
        return _ALIGN32_(len);
#undef _STR_
#undef _PUSH_ENUM_AS_STRING_2_RAM_
#undef _ALIGN32_
}

static uint16_t piggy_back_partition_entry(uint8_t *ram, const partition_entry_t *flash_entry)
{
        cmd_partition_entry_t *ram_entry = (cmd_partition_entry_t *)(ram);
        uint8_t *ram_str = &(ram_entry->name.str);
        ram_entry->start_sector = flash_entry->start_sector;
        ram_entry->sector_count = flash_entry->sector_count;
        ram_entry->type = flash_entry->type;
        ram_entry->name.len = push_partition_entry_name((ram_str) , flash_entry->type);
        return sizeof(cmd_partition_entry_t) + ram_entry->name.len;
}

static bool piggy_back_partition_table(uint8_t *ram)
{
        uint16_t entry_size = 0;
        cmd_partition_table_t *ram_table = (cmd_partition_table_t *)ram;
        cmd_partition_entry_t *ram_entry = &(ram_table->entry);
        partition_entry_t flash_entry;
        uint32_t flash_addr = PARTITION_TABLE_ADDR;
        ram_table->sector_size = FLASH_SECTOR_SIZE;
        ram_table->len = 0;

        do {
                ad_flash_read(flash_addr, (uint8_t *)&flash_entry, sizeof(partition_entry_t));
                if (flash_entry.type != 0xFF && flash_entry.type != 0 && flash_entry.magic == 0xEA &&
                                flash_entry.valid == 0xFF) {
                        entry_size = piggy_back_partition_entry((uint8_t *)ram_entry, &flash_entry);

                        ram_entry = (cmd_partition_entry_t *)((uint8_t *)ram_entry + entry_size);
                        ram_table->len += entry_size;
                }

                flash_addr += sizeof(partition_entry_t);
        } while (flash_entry.type != 0xFF);
        ram_table->len += sizeof(cmd_partition_table_t);

        return true;
}

/* handler for 'send data to RAM' */
static bool cmd_send_to_ram(HANDLER_OP hop)
{
        struct cmdhdr_send_to_ram *hdr = &cmd_state.hdr.send_to_ram;

        switch (hop) {
        case HOP_INIT:
                /* some payload is required, otherwise there's nothing to write */
                return cmd_state.data_len > 0;

        case HOP_HEADER:
                /*
                 * When data is written to RAM there is no need to store it in buffer
                 * and then copy to destination. Change address of data from buffer
                 * which is preset to what command wants;
                 */
                cmd_state.data = hdr->ptr;
                /*
                 * When address is explicitly set to ADDRESS_TMP or lays in range
                 * assigned for buffer, convert it to real address in RAM.
                 * hdr->ptr is not modified since it is needed for CRC calculation.
                 */
                translate_ram_addr((uint32_t *) &cmd_state.data);
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* Data was already put in correct place */
                xmit_ack();
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'read memory region from device' */
static bool cmd_read_from_ram(HANDLER_OP hop)
{
        struct cmdhdr_read_from_ram *hdr = &cmd_state.hdr.read_from_ram;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                /* nothing to do */
                return true;

        case HOP_SEND_LEN:
                xmit_data((void *) &hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                translate_ram_addr((uint32_t *) &hdr->ptr);
                xmit_data(hdr->ptr, hdr->len);
                return true;
        }
        return false;
}

/* handler for 'write RAM region to QSPI' */
static bool cmd_write_ram_to_qspi(HANDLER_OP hop)
{
        struct cmdhdr_write_ram_to_qspi *hdr = &cmd_state.hdr.write_ram_to_qspi;
#if dg_configVERIFY_QSPI_WRITE
        uint32_t read_buf_addr = ADDRESS_TMP;
#endif
        int ret;
        uint16_t len = hdr->len;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* check for 'magic' address */
                translate_ram_addr((uint32_t *) &hdr->ptr);
                return true;

        case HOP_EXEC:
                ret = ad_flash_update_possible(hdr->addr, hdr->ptr, hdr->len);

                if (ret < 0) {
                        if (!ad_flash_erase_region(hdr->addr, hdr->len)) {
                                xmit_nak();
                                return true;
                        }
                        ret = 0;
                } else {
                        len -= ret;
                }

                if (ad_flash_write(hdr->addr + ret, hdr->ptr + ret, len) != len) {
                        xmit_nak();
                        return true;
                }
#if dg_configVERIFY_QSPI_WRITE
                translate_ram_addr(&read_buf_addr); // get address to big buffer
                read_buf_addr += hdr->len;          // move after written data

                if (!ad_flash_read(hdr->addr, (uint8_t *) read_buf_addr, hdr->len)) {
                        xmit_nak();
                        return true;
                }

                if (memcmp(hdr->ptr, (uint8_t *) read_buf_addr, hdr->len)) {
                        xmit_nak();
                        return true;
                }
#endif
                xmit_ack();
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'erase region of flash' */
static bool cmd_erase_qspi(HANDLER_OP hop)
{
        struct cmdhdr_erase_qspi *hdr = &cmd_state.hdr.erase_qspi;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return hdr->len > 0;

        case HOP_EXEC:

                if (ad_flash_erase_region(hdr->addr, hdr->len)) {
                        xmit_ack();
                } else {
                        xmit_nak();
                }
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

void __attribute__((section("reboot_section"), noinline)) move_to_0_and_boot(void *start, size_t size)
{
        uint32_t *src = start;
        uint32_t *dst = NULL;
        int s = (int) ((size + 4) >> 2);
        int i;
        for (i = 0; i < s; ++i) {
                dst[i] = src[i];
        }
        REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, SW_RESET);
}

/* handler for 'execute code on device' */
static bool cmd_execute_code(HANDLER_OP hop)
{
        struct cmdhdr_execute_code *hdr = &cmd_state.hdr.execute_code;
        uint32_t addr;
        void (* func) (void);

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                addr = hdr->addr;

                // ack only if address in within mapped memory
                //
                //            start addr   end addr
                // Remapped   00000000     04000000
                // ROM        07F00000     07F40000
                // OTPC       07F40000     07F80000
                // OTP        07F80000     07FC0000
                // DataRAM    07FC0000     07FE0000
                // QSPI       08000000     0BF00000
                // Buffer     80000000     80024000
                return ((addr >= 0 && addr < 0x0400000) ||
                        (addr >= 0x07F00000 && addr < 0x07FE0000) ||
                        (addr >= 0x08000000 && addr < 0x0BF00000) ||
                        ((addr & VIRTUAL_BUF_MASK) == VIRTUAL_BUF_ADDRESS));

        case HOP_EXEC:
                xmit_ack();
                translate_ram_addr((uint32_t *) &hdr->addr);
                /* make sure lsb is 1 (thumb mode) */
                func = (void *) (hdr->addr | 1);
                if ((uint32_t) func == ((uint32_t)&__inputbuffer_start) + 1) {
                        move_to_0_and_boot(&__inputbuffer_start,
                                                        &__inputbuffer_end - &__inputbuffer_start);
                } else {
                        func();
                }
                return true; // we actually should never reach this

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'write to OTP' */
static bool cmd_write_otp(HANDLER_OP hop)
{
        struct cmdhdr_write_otp *hdr = &cmd_state.hdr.write_otp;

        switch (hop) {
        case HOP_INIT:
                /* make sure data to be written length is multiply of word size (4 bytes) */
                return (cmd_state.data_len > 0) && ((cmd_state.data_len & 0x03) == 0);

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* make sure cell address is valid */
                return hdr->addr < 0x2000;

        case HOP_EXEC:
                if (hw_otpc_dma_prog((uint32_t *) cmd_state.data, hdr->addr, HW_OTPC_WORD_LOW,
                                                                cmd_state.data_len >> 2, false)) {
                        xmit_ack();
                } else {
                        xmit_nak();
                }
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'read from OTP' */
static bool cmd_read_otp(HANDLER_OP hop)
{
        struct cmdhdr_read_otp *hdr = &cmd_state.hdr.read_otp;
        static uint16_t size;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                size = hdr->len * sizeof(uint32_t);
                return hdr->addr < 0x2000;

        case HOP_EXEC:
                /* there's no payload for this command so we can safely read into buffer */
                if (hw_otpc_fifo_read((void *)cmd_state.data, hdr->addr,
                        HW_OTPC_WORD_LOW, hdr->len, false)) {
                        xmit_ack();
                }
                else {
                        xmit_nak();
                }
                return true;

        case HOP_SEND_LEN:
                xmit_data((void *) &size, sizeof(size));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, size);
                return true;
        }

        return false;
}

/* handler for 'read QSPI' */
static bool cmd_read_qspi(HANDLER_OP hop)
{
        struct cmdhdr_read_qspi *hdr = &cmd_state.hdr.read_qspi;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* there's no payload for this command so we can safely read into buffer */
                ad_flash_read(hdr->addr, cmd_state.data, hdr->len);
                return true;

        case HOP_SEND_LEN:
                xmit_data((void *) &hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, hdr->len);
                return true;
        }

        return false;
}

/* handler for 'get_version on device' */
static bool cmd_get_version(HANDLER_OP hop)
{
        static const uint8_t msg[] = {(VERSION & 0xF0) >> 8, VERSION & 0x0F};

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                /* nothing to do */
                return true;

        case HOP_SEND_LEN:
                /* nothing to do */
                return true;
        case HOP_SEND_DATA:
                xmit_data((void*) msg, sizeof(msg));
                 return true;
        }

        return false;
}

static bool cmd_is_empty_qspi(HANDLER_OP hop)
{
        struct cmdhdr_is_empty_qspi *hdr = &cmd_state.hdr.is_empty_qspi;
        static int32_t return_val;
        uint32_t i = 0;
        uint32_t tmp_addr = ADDRESS_TMP;
        uint32_t tmp_addr2;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* something is wrong - do not execute command if size is zero */
                return hdr->size != 0;

        case HOP_EXEC:
                translate_ram_addr(&tmp_addr); // get address to big buffer
                memset((uint8_t *) tmp_addr, 0xFF, 2048);  // 2048 bytes of FF pattern
                tmp_addr2 = tmp_addr + 2048; // address for read values
                cmd_state.data_len = sizeof(return_val);
                cmd_state.data = (uint8_t *) &return_val;

                while (i < hdr->size) {
                        const uint32_t read_len = ((hdr->size - i) > 2048 ? 2048 : (hdr->size - i));

                        ad_flash_read(hdr->start_address + i, (uint8_t *) tmp_addr2, read_len);

                        if (memcmp((uint8_t *) tmp_addr, (uint8_t *) tmp_addr2, read_len)) {
                                uint32_t j;

                                for (j = 0; j < read_len; j++) {
                                        if (*(uint8_t *) (tmp_addr2 + j) != 0xFF) {
                                                break;
                                        }
                                }

                                return_val = (int32_t) (-1 * (i + j));
                                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                                xmit_ack();
                                return true;
                        }

                        i += read_len;
                }

                return_val = hdr->size;
                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                xmit_ack();
                return true;

        case HOP_SEND_LEN:
                xmit_data((uint8_t *) &cmd_state.data_len , sizeof(cmd_state.data_len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, cmd_state.data_len);
                return true;
        }

        return false;
}

static bool cmd_read_partition_table(HANDLER_OP hop)
{
        uint8 *ram = (uint8_t *)cmd_state.data;
        cmd_partition_table_t  *ram_table = (cmd_partition_table_t *)ram;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                return piggy_back_partition_table(cmd_state.data);

        case HOP_SEND_LEN:
                xmit_data((void *) &ram_table->len, sizeof(ram_table->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(ram, ram_table->len);
                return true;
        }

        return false;
}

static bool cmd_read_partition(HANDLER_OP hop)
{
        struct cmdhdr_read_partition *hdr = &cmd_state.hdr.read_partition;
        nvms_t nvms;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                if (!ad_nvms_init_called) {
                        ad_nvms_init_called = true;
                        ad_nvms_init();
                }
                nvms = ad_nvms_open(hdr->id);
                ad_nvms_read(nvms, hdr->addr, cmd_state.data, hdr->len);
                return true;

        case HOP_SEND_LEN:
                xmit_data((void *) &hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, hdr->len);
                return true;
        }

        return false;
}

static bool cmd_write_partition(HANDLER_OP hop)
{
        struct cmdhdr_write_partition *hdr = &cmd_state.hdr.write_partition;
        nvms_t nvms;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* check for 'magic' address */
                translate_ram_addr((uint32_t *) &hdr->ptr);
                return true;

        case HOP_EXEC:
                if (!ad_nvms_init_called) {
                        ad_nvms_init_called = true;
                        ad_nvms_init();
                }
                nvms = ad_nvms_open(hdr->id);

                if (ad_nvms_write(nvms, hdr->addr, hdr->ptr, hdr->len) >= 0) {
                        xmit_ack();
                } else {
                        xmit_nak();
                }
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

static bool cmd_chip_erase_qspi(HANDLER_OP hop)
{
        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                xmit_ack();

                if (ad_flash_chip_erase()) {
                        return true;
                } else {
                        return false;
                }

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* This command is needed only by GDB Server interface */
static bool cmd_dummy(HANDLER_OP hop)
{
        char live_str[] = UARTBOOT_LIVE_MARKER;
        uint32_t tmp_addr = ADDRESS_TMP;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                translate_ram_addr(&tmp_addr); // get address to big buffer
                memcpy((uint8_t *) tmp_addr, live_str, strlen(live_str));
                xmit_ack();
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                return false;
        }

        return false;
}

/* provided by linker script */
extern char __patchable_params;

static void init(void)
{
        timer1_config t1_cfg = {
                .clk_src = HW_TIMER1_CLK_SRC_EXT,
                .prescaler = 15999, // 16MHz / (15999 + 1) = 1kHz

                .timer = {
                        .direction = HW_TIMER1_DIR_UP,
                        .reload_val = 999, // interrupt every 1s
                },
        };
        uint32_t *pparams = (void*)&__patchable_params;
        HW_GPIO_PORT tx_port, rx_port;
        HW_GPIO_PIN tx_pin, rx_pin;

        /*
         * get UART parameters from patchable area, if their value is not 0xffffffff,
         * or else, use the CFG_* values
         */
        if (pparams[0] != 0xffffffff)
                tx_port = (HW_GPIO_PORT)pparams[0];
        else
                tx_port = CFG_GPIO_BOOTUART_TX_PORT;
        if (pparams[1] != 0xffffffff)
                tx_pin = (HW_GPIO_PIN)pparams[1];
        else
                tx_pin = CFG_GPIO_BOOTUART_TX_PIN;
        if (pparams[2] != 0xffffffff)
                rx_port = (HW_GPIO_PORT)pparams[2];
        else
                rx_port = CFG_GPIO_BOOTUART_RX_PORT;
        if (pparams[3] != 0xffffffff)
                rx_pin = (HW_GPIO_PIN)pparams[3];
        else
                rx_pin = CFG_GPIO_BOOTUART_RX_PIN;
        if (pparams[4] != 0xffffffff)
        {
                switch (pparams[4])
                {
                case 4800:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_4800;
                        break;
                case 9600:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_9600;
                        break;
                case 14400:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_14400;
                        break;
                case 19200:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_19200;
                        break;
                case 28800:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_28800;
                        break;
                case 38400:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_38400;
                        break;
                case 57600:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_57600;
                        break;
                case 115200:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_115200;
                        break;
                case 230400:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_230400;
                        break;
                case 500000:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_500000;
                        break;
                case 1000000:
                        UART_INIT.baud_rate = HW_UART_BAUDRATE_1000000;
                        break;
                }
        }

        hw_gpio_set_pin_function(tx_port, tx_pin, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_UART2_TX);
        hw_gpio_set_pin_function(rx_port, rx_pin, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_UART2_RX);

        hw_uart_init(BOOTUART, &UART_INIT);

        hw_otpc_init();
        hw_otpc_set_speed(HW_OTPC_SYS_CLK_FREQ_16);

        hw_timer1_init(HW_TIMER1_MODE_TIMER, &t1_cfg);

        ad_flash_init();

}

/* transmit announcement message every 1s and wait for <SOH> response */
static void wait_for_soh(void)
{
        uart_soh = false;
        timer1_soh_tmo = true;

        hw_timer1_register_int(timer1_soh_cb);
        hw_timer1_enable();

        while (!uart_soh) {
                if (timer1_soh_tmo) {
                        timer1_soh_tmo = false;
#if (dg_configSUPPRESS_HelloMsg == 0)
                        xmit_hello();
#endif
                        hw_uart_receive(BOOTUART, uart_buf, 1, (hw_uart_rx_callback) uart_soh_cb,
                                                                                        uart_buf);
                }

                __WFI();
        };

        hw_timer1_disable();
}

static void process_header(void)
{
        memset(&cmd_state, 0, sizeof(cmd_state));
        cmd_state.data = &__inputbuffer_start;

        cmd_state.type = uart_buf[1];
        cmd_state.len = uart_buf[2] | (uart_buf[3] << 8);

        switch (cmd_state.type) {
        case 0x01:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.send_to_ram);
                cmd_state.handler = cmd_send_to_ram;
                break;
        case 0x02:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_from_ram);
                cmd_state.handler = cmd_read_from_ram;
                break;
        case 0x03:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_ram_to_qspi);
                cmd_state.handler = cmd_write_ram_to_qspi;
                break;
        case 0x04:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.erase_qspi);
                cmd_state.handler = cmd_erase_qspi;
                break;
        case 0x05:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.execute_code);
                cmd_state.handler = cmd_execute_code;
                break;
        case 0x06:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_otp);
                cmd_state.handler = cmd_write_otp;
                break;
        case 0x07:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_otp);
                cmd_state.handler = cmd_read_otp;
                break;
        case 0x08:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_qspi);
                cmd_state.handler = cmd_read_qspi;
                break;
        case 0x0A:
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_read_partition_table;
                break;
        case 0x0B:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.get_version);
                cmd_state.handler = cmd_get_version;
                break;
        case 0x0C:
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_chip_erase_qspi;
                break;
        case 0x0D:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.is_empty_qspi);
                cmd_state.handler = cmd_is_empty_qspi;
                break;
        case 0x0E:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_partition);
                cmd_state.handler = cmd_read_partition;
                break;
        case 0x0F:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_partition);
                cmd_state.handler = cmd_write_partition;
                break;
        case 0xFF:
                /* Dummy command - only for GDB server interface */
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_dummy;
                break;
        }

        /* store length of payload (command data excluding command header) */
        cmd_state.data_len = cmd_state.len - cmd_state.hdr_len;
}

/* wait for command header (type + length) */
static bool wait_for_cmd(void)
{
        int soh_len = 1;

        /*
         * uart_soh is set when SOH was already received in response to announcement thus we won't
         * receive another one here. By resetting this flag we make sure that for next command we'll
         * expect SOH to be received here.
         */
        if (uart_soh) {
                soh_len = 0;
        }
        uart_soh = false;

        if (!recv_with_tmo(uart_buf + 1 - soh_len, 3 + soh_len, TMO_COMMAND)) {
                return false;
        }

        process_header();

        return true;
}


static bool load_data(void)
{
        bool ret;

        /* receive command header */
        if (!recv_with_tmo((uint8_t *) &cmd_state.hdr, cmd_state.hdr_len, TMO_DATA)) {
                return false;
        }

        cmd_state.handler(HOP_HEADER);

        /* receive command payload */
        if (!recv_with_tmo(cmd_state.data, cmd_state.data_len,
                                        1 + cmd_state.data_len * (UART_INIT.baud_rate / 10))) {
                return false;
        }

        crc16_init(&cmd_state.crc);
        crc16_update(&cmd_state.crc, (uint8_t *) &cmd_state.hdr, cmd_state.hdr_len);
        crc16_update(&cmd_state.crc, cmd_state.data, cmd_state.data_len);

        ret = cmd_state.handler(HOP_DATA);

        if (!ret) {
                xmit_nak();
                return false;
        }

        xmit_ack();
        xmit_crc16(cmd_state.crc);

        ret = recv_with_tmo(uart_buf, 1, TMO_ACK);
        ret &= (uart_buf[0] == ACK);
        if (ret) {
                ret &= cmd_state.handler(HOP_EXEC);
        }

        return ret;
}

static void load_header(void)
{
        process_header();
        memcpy(&cmd_state.hdr, uart_buf + 4, cmd_state.hdr_len);
}

/*
 * swd_interface.run_swd is constant value 0
 * Debugger will setup it to 1 when uartboot is to be controlled from debugger
 */
void swd_loop(void) {
        uint32_t last_num = swd_interface.cmd_num;
        uint32_t current_num;
        while (swd_interface.run_swd) {
                current_num = swd_interface.cmd_num;
                if (last_num != current_num) {
                        last_num = current_num;
                        /* Debugger put header in uart_buf, process it */
                        load_header();
                        if (cmd_state.handler) {
                                cmd_state.handler(HOP_INIT);
                                cmd_state.handler(HOP_DATA);
                                cmd_state.handler(HOP_EXEC);
                        }
                }
                __BKPT(12);
        }
}

int main()
{
        CRG_TOP->CLK_AMBA_REG = 0;
        hw_watchdog_freeze();
        hw_cpm_deactivate_pad_latches();
        ENABLE_DEBUGGER;
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP);

        /* qspi */
        hw_qspi_set_div(HW_QSPI_DIV_1);
        hw_qspi_enable_clock();


        init();

        swd_loop();

soh_loop:
        wait_for_soh();

cmd_loop:
        /* receive command header (type + length) */
        if (!wait_for_cmd()) {
                goto soh_loop;
        }

        /* NAK for commands we do not support or have faulty header, i.e. length is incorrect */
        if (!cmd_state.handler || !cmd_state.handler(HOP_INIT)) {
                xmit_nak();
                goto cmd_loop;
        }

        /* receive data from CLI */
        if (cmd_state.len) {
                xmit_ack();

                if (!load_data()) {
                        if (uart_tmo) {
                                goto soh_loop;
                        } else {
                                goto cmd_loop;
                        }
                }
        } else {
                if (!cmd_state.handler(HOP_EXEC)) {
                        xmit_nak();
                        goto cmd_loop;
                }
                xmit_ack();
        }

        /* send data length of response, if any */
        if (!cmd_state.handler(HOP_SEND_LEN)) {
                goto cmd_loop;
        }
        if (!recv_with_tmo(uart_buf, 1, 5) || uart_buf[0] != ACK) {
                goto soh_loop;
        }

        /* send response data */
        crc16_init(&cmd_state.crc);
        if (!cmd_state.handler(HOP_SEND_DATA)) {
                goto soh_loop;
        }

        /* receive and check CRC */
        if (!recv_with_tmo(uart_buf, 2, 5)) {
                goto soh_loop;
        }
        if (!memcmp(uart_buf, &cmd_state.crc, 2)) { // we're l-endian and CRC is transmitted lsb-first
                xmit_ack();
        } else {
                xmit_nak();
        }

        goto cmd_loop;

        return 0;
}

/*
 * \}
 * \}
 * \}
 */
