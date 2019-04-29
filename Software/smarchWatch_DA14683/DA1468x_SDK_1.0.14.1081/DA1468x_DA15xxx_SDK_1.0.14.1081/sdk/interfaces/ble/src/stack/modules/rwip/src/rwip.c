/**
****************************************************************************************
*
* @file rwip.c
*
* @brief RW IP SW main module
*
* Copyright (C) RivieraWaves 2009-2014
*
*
****************************************************************************************
*/

/**
 ****************************************************************************************
 * @addtogroup RW IP SW main module
 * @ingroup ROOT
 * @brief The RW IP SW main module.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // RW SW configuration
#include "co_version.h"

#include <string.h>          // for mem* functions
#include "rwip.h"            // RW definitions
#include "arch.h"            // Platform architecture definition

#if (NVDS_SUPPORT)
#include "nvds.h"         // NVDS definitions
#endif // NVDS_SUPPORT

#if (BT_EMB_PRESENT)
#include "rwbt.h"            // rwbt definitions
#endif //BT_EMB_PRESENT

#if (BLE_EMB_PRESENT)
#include "rwble.h"           // rwble definitions
#endif //BLE_EMB_PRESENT

#if (BLE_HOST_PRESENT)
#include "rwble_hl.h"        // BLE HL definitions
#include "gapc.h"
#include "smpc.h"
#include "gattc.h"
#include "attc.h"
#include "atts.h"
#include "l2cc.h"
#endif //BLE_HOST_PRESENT

#if (BLE_APP_PRESENT)
#include "app.h"             // Application definitions
#endif //BLE_APP_PRESENT

#if (DEEP_SLEEP)
#if (BLE_EMB_PRESENT)
#include "lld_sleep.h"       // definitions for sleep mode
#endif //BLE_EMB_PRESENT
#endif //DEEP_SLEEP

#if (BLE_EMB_PRESENT)
#include "llc.h"
#endif //BLE_EMB_PRESENT

#if (DISPLAY_SUPPORT)
#include "display.h"         // display definitions
#endif //DISPLAY_SUPPORT

#if (EA_PRESENT)
#include "ea.h"              // Event Arbiter definitions
#endif //EA_PRESENT

#if (BT_EMB_PRESENT || BLE_EMB_PRESENT)
#include "plf.h"             // platform definition
#include "rf.h"              // RF definitions
#endif //BT_EMB_PRESENT || BLE_EMB_PRESENT

#if (TL_ITF)
#include "h4tl.h"
#endif //TL_ITF

#if (GTL_ITF)
#include "gtl.h"
#endif //GTL_ITF

#if (HCI_PRESENT)
#include "hci.h"             // HCI definition
#endif //HCI_PRESENT

#if (KE_SUPPORT)
#include "ke.h"              // kernel definition
#include "ke_event.h"        // kernel event
#include "ke_timer.h"        // definitions for timer
#include "ke_mem.h"          // kernel memory manager
#endif //KE_SUPPORT

#include "dbg.h"             // debug definition

#include "sys_clock_mgr.h"   // Needed for rcx_clock_hz

/*
 * DEFINES
 ****************************************************************************************
 */
volatile uint8_t ke_mem_heaps_used       __attribute__((section("exchange_mem_case1")));
#if (DEEP_SLEEP)
/// Sleep Duration Value in periodic wake-up mode
#define MAX_SLEEP_DURATION_PERIODIC_WAKEUP      rom_cfg_table[max_sleep_duration_periodic_wakeup_pos] //0x0320  // 0.5s
/// Sleep Duration Value in external wake-up mode
#define MAX_SLEEP_DURATION_EXTERNAL_WAKEUP      rom_cfg_table[max_sleep_duration_external_wakeup_pos]  //0x3E80  //10s
#endif //DEEP_SLEEP
#if (DISPLAY_SUPPORT)
///Table of HW image names for display
static const char* ip_type[6] =
{
        "HW: Backup Image",
        "HW: BTDM Ripple",
        "HW: BT Ripple",
        "HW: BLE Atlas",
        "HW: Unknown",
        "HW: BLE Ripple"
};

/// FW type display line
#if (BT_EMB_PRESENT && BLE_EMB_PRESENT)
#define FW_TYPE_DISPLAY   "FW: BTDM split emb"
#elif (BT_EMB_PRESENT)
#define FW_TYPE_DISPLAY   "FW: BT split emb"
#elif (BLE_EMB_PRESENT && BLE_HOST_PRESENT)
#define FW_TYPE_DISPLAY   "FW: BLE full"
#elif (BLE_EMB_PRESENT)
#define FW_TYPE_DISPLAY   "FW: BLE split emb"
#else
#define FW_TYPE_DISPLAY   "FW: ROM"
#endif // BT_EMB_PRESENT / BLE_EMB_PRESENT / BLE_HOST_PRESENT
#endif //DISPLAY_SUPPORT

 // Heap header size is 12 bytes
 #define RWIP_HEAP_HEADER             (12 / sizeof(uint32_t))
 // ceil(len/sizeof(uint32_t)) + RWIP_HEAP_HEADER
 #define RWIP_CALC_HEAP_LEN(len)      ((((len) + (sizeof(uint32_t) - 1)) / sizeof(uint32_t)) + RWIP_HEAP_HEADER)

/*
 * STRUCT DEFINITIONS
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
#if (DEEP_SLEEP)
/// RW SW environment
struct rwip_env_tag rwip_env;
#endif //DEEP_SLEEP
/// RF API
struct rwip_rf_api rwip_rf;
//VM

void crypto_init(void);

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */


#if (DEEP_SLEEP)
/**
 ****************************************************************************************
 * @brief  check If the fine counter is close the 3/4 of the slot boundary (624 >> 2) * 3 = 468
 *
 * @return when boundary wak-up is ok
 ****************************************************************************************
 */
__RETAINED_CODE bool rwip_check_wakeup_boundary(void)
{
    #if (BT_EMB_PRESENT)
    //Sample the base time count
    bt_slotclk_samp_setf(1);
    while (bt_slotclk_samp_getf());

    return  ((bt_finetimecnt_get() < 468) ? false : true);
    #elif (BLE_EMB_PRESENT)
    //Sample the base time count
    ble_sampleclk_set(BLE_SAMP_BIT);
    while (ble_sampleclk_get());
    return  ((ble_finetimecnt_get() < rom_cfg_table[wakeup_boundary_var_pos]/*468*/) ? false : true);
    #else
    return true;
    #endif
}

__RETAINED_CODE bool rwip_check_wakeup_boundary_rcx(void)
{
        uint32_t time;
        uint32_t high_limit, low_limit;

        //Sample the base time count
        ble_sampleclk_set(BLE_SAMP_BIT);

        // Until then, determine the proper window limits based on the RCX frequency
        if (rcx_clock_hz > 14286) {
                high_limit = 207;
                low_limit = 64;
        } else if (rcx_clock_hz > 11236) {
                high_limit = 217;
                low_limit = 140;
        } else if (rcx_clock_hz > 9175) {
                high_limit = 275;
                low_limit = 220;
        } else if (rcx_clock_hz > 8404) {
                high_limit = 295;
                low_limit = 260;
        } else if (rcx_clock_hz > 7752) {
                high_limit = 315;
                low_limit = 300;
        } else {
                high_limit = 339;
                low_limit = 308;
        }

        //Block until the value is ready
        while (ble_sampleclk_get());

        //Get the base time count now
        time = ble_finetimecnt_get();

        return  ((time > high_limit) || (time < low_limit) ? false : true);
}

#endif //DEEP_SLEEP


#if (DEEP_SLEEP && BLE_EMB_PRESENT)
/**
 ****************************************************************************************
 * @brief Converts a duration in slots into a number of low power clock cycles.
 * The function converts a duration in slots into a number of low power clock cycles.
 * Sleep clock runs at either 32768Hz or 32000Hz, so this function divides the value in
 * slots by 20.48 or 20 depending on the case.
 * To do this the following formulae are applied:
 *
 *   N = x * 20.48 = (2048 * x)/100 for a 32.768kHz clock or
 *   N = x * 20                     for a 32kHz clock
 *
 * @param[in] slot_cnt    The value in slot count
 *
 * @return The number of low power clock cycles corresponding to the slot count
 *
 ****************************************************************************************
 */
static uint32_t rwip_slot_2_lpcycles(uint32_t slot_cnt)
{
    uint32_t lpcycles;

    // Sanity check: The number of slots should not be too high to avoid overflow
    ASSERT_ERR(slot_cnt < 1000000);
//VM
    if (HZ32000)
    // Compute the low power clock cycles - case of a 32kHz clock
    lpcycles = slot_cnt * 20;
    else //HZ32000
    // Compute the low power clock cycles - case of a 32.768kHz clock
    lpcycles = (slot_cnt << 11)/100;
//    #endif //HZ32000

    // Corner case, Sleep duration is exactly on slot boundary so slot interrupt will not
    // be generated on expected slot (but next one).

    // So reduce little bit sleep duration in order to allow fine time compensation
    // Note compensation will be in range of [1 , 2[ lp cycles

    lpcycles--;

    return(lpcycles);
}
#endif //(DEEP_SLEEP && BLE_EMB_PRESENT)


#if (DISPLAY_SUPPORT)
/**
 ****************************************************************************************
 * @brief Display device configuration
 *
 * This function adds graphical display
 ****************************************************************************************
 */
static void display_add_config(void)
{
    uint8_t i;
    #if (NVDS_SUPPORT)
    uint8_t dev_name_length = NVDS_LEN_DEVICE_NAME;
    uint8_t dev_name_data[NVDS_LEN_DEVICE_NAME];
    uint8_t bd_ad_length = NVDS_LEN_BD_ADDRESS;
    uint8_t bd_ad_data[NVDS_LEN_BD_ADDRESS];
    #if (PLF_UART)
    uint8_t uart_length = NVDS_LEN_UART_BAUDRATE;
    uint32_t baudrate = 921600;
    #endif //PLF_UART
    #endif // NVDS_SUPPORT
    uint32_t fw_version;
    struct plf_version plfversion;
    struct plf_version plfversion_unkn;
    uint32_t hw_version;
    uint8_t digit;
    uint8_t value;
    char* ptr;
    #if defined(CFG_RF_RIPPLE)
    uint16_t rf_id;
    char scr_rf[DISPLAY_LINE_SIZE+1];
    #endif //CFG_RF_RIPPLE
    char scr_fw_time[DISPLAY_LINE_SIZE+1];
    char scr_bd_ad[DISPLAY_LINE_SIZE+1];
    char scr_fw_version[DISPLAY_LINE_SIZE+1];
    char scr_fpga_time[DISPLAY_LINE_SIZE+1];
    char scr_fpga_version[DISPLAY_LINE_SIZE+1];
    char scr_hw_version[DISPLAY_LINE_SIZE+1];
    #if (PLF_UART && NVDS_SUPPORT)
    char scr_uart[DISPLAY_LINE_SIZE+1];
    #endif //PLF_UART

    uint8_t s_type = display_screen_alloc();
    uint8_t s_fw = display_screen_alloc();
    uint8_t s_fw_time = display_screen_alloc();
    uint8_t s_fpga_version = display_screen_alloc();
    uint8_t s_fpga_time = display_screen_alloc();
    #if (BT_EMB_PRESENT)
    uint8_t s_bthw = display_screen_alloc();
    #endif //BT_EMB_PRESENT
    #if (BLE_EMB_PRESENT)
    uint8_t s_blehw = display_screen_alloc();
    #endif //BLE_EMB_PRESENT
    uint8_t s_name = display_screen_alloc();
    uint8_t s_bd = display_screen_alloc();
    #if (PLF_UART)
    uint8_t s_uart = display_screen_alloc();
    #endif //PLF_UART
    uint8_t s_rf = display_screen_alloc();

    // List all screens
    display_screen_insert(s_fpga_version, s_type);
    display_screen_insert(s_fpga_time, s_type);
    display_screen_insert(s_fw, s_type);
    display_screen_insert(s_fw_time, s_type);
    #if (BT_EMB_PRESENT)
    display_screen_insert(s_bthw, s_type);
    #endif //BT_EMB_PRESENT
    #if (BLE_EMB_PRESENT)
    display_screen_insert(s_blehw, s_type);
    #endif //BLE_EMB_PRESENT
    display_screen_insert(s_name, s_type);
    display_screen_insert(s_bd, s_type);
    #if (PLF_UART)
    display_screen_insert(s_uart, s_type);
    #endif //PLF_UART
    display_screen_insert(s_rf, s_type);

    // Get platform version, date, time ...
    plf_read_version(&plfversion);

    /************************************************************************/
    /*                              FW TYPE                                 */
    /************************************************************************/
    memset(&plfversion_unkn, 0, sizeof(plfversion_unkn));
    if(plfversion.ip_type > 6 || (memcmp(&plfversion, &plfversion_unkn, sizeof(plfversion_unkn)) == 0))
        display_screen_set(s_type, NULL, "HW: Unknown" ,FW_TYPE_DISPLAY);
    else
        display_screen_set(s_type, NULL, ip_type[plfversion.ip_type] ,FW_TYPE_DISPLAY);

    /************************************************************************/
    /*                             FW VERSION                               */
    /************************************************************************/
    rwip_version((uint8_t*)&fw_version, (uint8_t*)&hw_version);
    i = 11;
    *(scr_fw_version+i) = '\0';
    while(i-- > 0)
    {
        digit = fw_version&0xF;
        digit += (digit < 10) ? 48:55;
        *(scr_fw_version+i) = (char)digit;
        fw_version >>= 4;
        if(i == 3 || i==6 || i==9)
        {
            *(scr_fw_version+(--i)) = '.';
        }
    }
    display_screen_set(s_fw, NULL, "FW version:", scr_fw_version);

    /************************************************************************/
    /*                              FW TIME                                 */
    /************************************************************************/
    /* Build the FW type screen with:
     *  - type
     *  - build date "Mmm dd yyyy"
     *  - build time "hh:mm:ss"
     */
    strncpy(scr_fw_time, __DATE__, 7);
    strncpy(scr_fw_time+7, __TIME__, 8);
    scr_fw_time[DISPLAY_LINE_SIZE] = '0';
    display_screen_set(s_fw_time, NULL, "FW date:", scr_fw_time);

    /************************************************************************/
    /*                            FPGA VERSION                              */
    /************************************************************************/
    // Plf type
    scr_fpga_version[0] = plfversion.plf_type + ((plfversion.plf_type < 10) ? 48:55);
    scr_fpga_version[1] = '.';
    // Ip type
    scr_fpga_version[2] = plfversion.ip_type + ((plfversion.ip_type < 10) ? 48:55);
    scr_fpga_version[3] = '.';
    // Version
    i = 10;
    while(i-- > 4)
    {
        digit = plfversion.version &0xF;
        digit += (digit < 10) ? 48:55;
        *(scr_fpga_version+i) = (char)digit;
        plfversion.version >>= 4;
    }
    scr_fpga_version[10] = '\0';
    display_screen_set(s_fpga_version, NULL, "FPGA version:", scr_fpga_version);

    /************************************************************************/
    /*                           FPGA DATE/TIME                             */
    /************************************************************************/
    ptr = scr_fpga_time;
    // Month
    value = plfversion.month;
    digit = (value/10) + 48;
    *(ptr++) = (char)digit;
    digit = (value - 10*(value/10)) + 48;
    *(ptr++) = (char)digit;
    *(ptr++) = '_';
    // Day
    value = plfversion.day;
    digit = (value/10) + 48;
    *(ptr++) = (char)digit;
    digit = (value - 10*(value/10)) + 48;
    *(ptr++) = (char)digit;
    *(ptr++) = ' ';
    // Hours
    value = plfversion.hour;
    digit = (value/10) + 48;
    *(ptr++) = (char)digit;
    digit = (value - 10*(value/10)) + 48;
    *(ptr++) = (char)digit;
    *(ptr++) = ':';
    // Minutes
    value = plfversion.minute;
    digit = (value/10) + 48;
    *(ptr++) = (char)digit;
    digit = (value - 10*(value/10)) + 48;
    *(ptr++) = (char)digit;
    *(ptr++) = ':';
    // Seconds
    value = plfversion.second;
    digit = (value/10) + 48;
    *(ptr++) = (char)digit;
    digit = (value - 10*(value/10)) + 48;
    *(ptr++) = (char)digit;
    *(ptr++) = '\0';
    display_screen_set(s_fpga_time, NULL, "FPGA Date:", scr_fpga_time);

    #if BT_EMB_PRESENT
    /************************************************************************/
    /*                           BT HW VERSION                              */
    /************************************************************************/
    i = 11;
    *(scr_hw_version+i) = '\0';
    while(i-- > 0)
    {
        digit = hw_version&0xF;
        digit += (digit < 10) ? 48:55;
        *(scr_hw_version+i) = (char)digit;
        hw_version >>= 4;
        if(i == 3 || i==6 || i==9)
        {
            *(scr_hw_version+(--i)) = '.';
        }
    }
    display_screen_set(s_bthw, NULL, "BT HW version:", scr_hw_version);
    #endif //BT_EMB_PRESENT

    #if (BLE_EMB_PRESENT)
    /************************************************************************/
    /*                           BLE HW VERSION                             */
    /************************************************************************/
    rwip_version((uint8_t*)&fw_version, (uint8_t*)&hw_version);
    i = 11;
    *(scr_hw_version+i) = '\0';
    while(i-- > 0)
    {
        digit = hw_version&0xF;
        digit += (digit < 10) ? 48:55;
        *(scr_hw_version+i) = (char)digit;
        hw_version >>= 4;
        if(i == 3 || i==6 || i==9)
        {
            *(scr_hw_version+(--i)) = '.';
        }
    }
    display_screen_set(s_blehw, NULL, "BLE HW version:", scr_hw_version);
    #endif //BLE_EMB_PRESENT

    /************************************************************************/
    /*                            DEVICE NAME                               */
    /************************************************************************/
    #if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_DEVICE_NAME, &dev_name_length, dev_name_data) == NVDS_OK)
    {
        // Put end of line
        if(dev_name_length > 16)
        {
            dev_name_length = 16;
        }
        dev_name_data[dev_name_length] = '\0';
    }
    else
    {
        dev_name_data[0] = '\0';
    }
    display_screen_set(s_name, NULL, "Device name:", (char*)dev_name_data);
    #else // NVDS_SUPPORT
    display_screen_set(s_name, NULL, "Device name:", "");
    #endif // NVDS_SUPPORT

    /************************************************************************/
    /*                              BD ADDRESS                              */
    /************************************************************************/
    strcpy(scr_bd_ad, "0x");
    #if (NVDS_SUPPORT)
    if(nvds_get(NVDS_TAG_BD_ADDRESS, &bd_ad_length, bd_ad_data) == NVDS_OK)
    {
        // Encode to ASCII
        for(i = 0; i < NVDS_LEN_BD_ADDRESS; i++)
        {
            digit = bd_ad_data[NVDS_LEN_BD_ADDRESS-1-i]>>4;
            digit += (digit < 10) ? 48:55;
            *(scr_bd_ad+2+2*i) = (char)digit;
            digit = bd_ad_data[NVDS_LEN_BD_ADDRESS-1-i]&0xF;
            digit += (digit < 10) ? 48:55;
            *(scr_bd_ad+2+2*i+1) = (char)digit;
        }
    }
    scr_bd_ad[14] = '\0';
    display_screen_set(s_bd, NULL, "BD Address:", scr_bd_ad);
    #else // NVDS_SUPPORT
    display_screen_set(s_bd, NULL, "BD Address:", "");
    #endif // NVDS_SUPPORT


    #if (PLF_UART)
    /************************************************************************/
    /*                            UART BAUDRATE                             */
    /************************************************************************/

    #if (NVDS_SUPPORT)
    // Get UART baudrate from NVDS
    if(nvds_get(NVDS_TAG_UART_BAUDRATE, &uart_length, (uint8_t*) &baudrate) == NVDS_OK)
    {
        if(baudrate > 3500000 || baudrate < 9600)
        {
            baudrate = 921600;
        }
    }
    else
    {
        baudrate = 921600;
    }

    // Display UART baudrate on screen
    strcpy(scr_uart, "        bps");
    i = 0;
    while (baudrate > 0)
    {
        scr_uart[6-i++] = (baudrate - (10*(baudrate / 10))) + 48;
        baudrate = baudrate / 10;
    }
    display_screen_set(s_uart, NULL, "UART baudrate:", scr_uart);
    #else // NVDS_SUPPORT
    display_screen_set(s_uart, NULL, "UART baudrate:", "");
    #endif // NVDS_SUPPORT
    #endif //PLF_UART


    /************************************************************************/
    /*                               RF BOARD                               */
    /************************************************************************/

    #if defined(CFG_RF_RIPPLE)
    // Read board ID in platform
    rf_id = plf_read_rf_board_id();
    // Add screen to display RF board type
    strcpy(scr_rf, "Ripple #");
    scr_rf[8] = (rf_id/10) + 48;
    scr_rf[9] = (rf_id - (10*(rf_id/10))) + 48;
    scr_rf[10] = '\0';
    display_screen_set(s_rf, NULL, "RF board:", scr_rf);
    #elif defined (CFG_RF_BLUEJAY)
    display_screen_set(s_rf, NULL, "RF board:", "Bluejay");
    #elif defined(CFG_RF_ATLAS)
    display_screen_set(s_rf, NULL, "RF board:", "Atlas");
    #endif //CFG_RF


    // Start with FW type screen
    display_start(s_type);
}
#endif //DISPLAY_SUPPORT

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

void rwip_init(uint32_t error)
{
    #if (NVDS_SUPPORT && DEEP_SLEEP)
    uint8_t length = 1;
    uint8_t sleep_enable;
    uint8_t ext_wakeup_enable;
    #endif //NVDS_SUPPORT && DEEP_SLEEP
    #if (DEEP_SLEEP)
    // Reset RW environment
    memset(&rwip_env, 0, sizeof(rwip_env));
    #endif //DEEPSLEEP
		ke_mem_heaps_used = KE_MEM_BLOCK_MAX;

    #if (KE_SUPPORT)
    // Initialize kernel
    ke_init();

//ke_mem_init(KE_MEM_ENV,           (uint8_t*)rwip_heap_env,     RWIP_HEAP_ENV_SIZE);
ke_mem_init(KE_MEM_ENV,(uint8_t*)(rom_cfg_table[rwip_heap_env_pos]),     rom_cfg_table[rwip_heap_env_size]);
    #if (BLE_HOST_PRESENT)
    // Memory allocated for Attribute database
//ke_mem_init(KE_MEM_ATT_DB,        (uint8_t*)rwip_heap_db,      RWIP_HEAP_DB_SIZE);
ke_mem_init(KE_MEM_ATT_DB,(uint8_t*)(rom_cfg_table[rwip_heap_db_pos]),      rom_cfg_table[rwip_heap_db_size]);
    #endif // (BLE_HOST_PRESENT)
    // Memory allocated for kernel messages
//ke_mem_init(KE_MEM_KE_MSG,        (uint8_t*)rwip_heap_msg,     RWIP_HEAP_MSG_SIZE);
ke_mem_init(KE_MEM_KE_MSG,(uint8_t*)(rom_cfg_table[rwip_heap_msg_pos]),     rom_cfg_table[rwip_heap_msg_size]);
    // Non Retention memory block
//ke_mem_init(KE_MEM_NON_RETENTION, (uint8_t*)rwip_heap_non_ret, RWIP_HEAP_NON_RET_SIZE);
ke_mem_init(KE_MEM_NON_RETENTION, (uint8_t*)(rom_cfg_table[rwip_heap_non_ret_pos]), rom_cfg_table[rwip_heap_non_ret_size]);
//VM
#endif
    #if (GTL_ITF)
    // Initialize the Generic Transport Layer
    gtl_init(rwip_eif_get(RWIP_EIF_AHI));
    #endif //GTL_ITF

    // Initialize RF
    #if (BT_EMB_PRESENT || BLE_EMB_PRESENT)
    rf_init(&rwip_rf);
    #endif //BT_EMB_PRESENT || BLE_EMB_PRESENT

    #if (TL_ITF)
    // Initialize H4 TL
    h4tl_init(rwip_eif_get(RWIP_EIF_HCIC));
    #endif //(TL_ITF)

    #if (HCI_PRESENT)
    // Initialize the HCI
    hci_init();
    #endif //HCI_PRESENT

    #if (BT_EMB_PRESENT)
    // Initialize BT
    rwbt_init();
    #endif //BT_EMB_PRESENT

    #if (BLE_EMB_PRESENT)
    // Initialize BLE
    rwble_init();
    #endif //BLE_EMB_PRESENT

    #if (EA_PRESENT)
    //Initialize Event arbiter
    ea_init(false);
    #endif //(EA_PRESENT)

    #if (BLE_HOST_PRESENT)
    // Initialize BLE Host stack
    rwble_hl_init();
    #endif //BLE_HOST_PRESENT

    #if (DISPLAY_SUPPORT)
    // Initialize display module
    display_init();

    // Add some configuration information to display
    display_add_config();
    #endif //DISPLAY_SUPPORT

    #if (NVDS_SUPPORT && DEEP_SLEEP)
    // Activate deep sleep feature if enabled in NVDS
    if(nvds_get(NVDS_TAG_SLEEP_ENABLE, &length, &sleep_enable) == NVDS_OK)
    {
        if(sleep_enable != 0)
        {
            rwip_env.sleep_enable = true;

            // Set max sleep duration depending on wake-up mode
            if(nvds_get(NVDS_TAG_EXT_WAKEUP_ENABLE, &length, &ext_wakeup_enable) == NVDS_OK)
            {
                if(ext_wakeup_enable != 0)
                {
                    rwip_env.ext_wakeup_enable = true;
                }
            }
        }
    }
    else
    #endif //NVDS_SUPPORT && DEEP_SLEEP
    {
        rwip_env.sleep_enable = true;
        rwip_env.ext_wakeup_enable = true;
    }

    // If FW initializes due to FW reset, send the message to Host
    if(error != RESET_NO_ERROR)
    {
        #if (BT_EMB_PRESENT)
        rwbt_send_message(error);
        #elif (BLE_EMB_PRESENT)
        rwble_send_message(error);
        #elif (BLE_HOST_PRESENT && GTL_ITF)
        rwble_hl_send_message(error);
        #endif //BT_EMB_PRESENT / BLE_EMB_PRESENT
    }

    /*
     ************************************************************************************
     * Application initialization
     ************************************************************************************
     */
    #if (BLE_APP_PRESENT)
    // Initialize APP
    appm_init();
    #endif //BLE_APP_PRESENT
}

void rwip_reset(void)
{
    // Disable interrupts until reset procedure is completed
    GLOBAL_INT_DISABLE();

    #if (KE_SUPPORT)
    //Clear all message and timer pending
    ke_flush();
    #endif //KE_SUPPORT

    #if (HCI_PRESENT)
    // Reset the HCI
    hci_reset();
    #endif //HCI_PRESENT

    #if (BT_EMB_PRESENT)
    // Reset BT
    rwbt_reset();
    #endif //BT_EMB_PRESENT

    #if (BLE_EMB_PRESENT)
    // Reset BLE
    rwble_reset();
    #endif //BLE_EMB_PRESENT

    #if (EA_PRESENT)
    ea_init(true);
    #endif //(EA_PRESENT)

    // Reset the RF
    rwip_rf.reset();

    #if (DISPLAY_SUPPORT)
    // Restart display module
    display_resume();
    #endif //DISPLAY_SUPPORT

    crypto_init();

    // Restore interrupts once reset procedure is completed
    GLOBAL_INT_RESTORE();
}

void rwip_version(uint8_t* fw_version, uint8_t* hw_version)
{
    #if (BT_EMB_PRESENT)
    rwbt_version(fw_version , hw_version);
    #elif (BLE_EMB_PRESENT)
    rwble_version(fw_version , hw_version);
    #endif //BT_EMB_PRESENT / BLE_EMB_PRESENT
}

void rwip_schedule(void)
{
    #if (KE_SUPPORT)
    #if (DEEP_SLEEP)
    // If system is waking up, delay the handling
    if ((rwip_env.prevent_sleep & RW_WAKE_UP_ONGOING) == 0)
    #endif // DEEP_SLEEP
    {
        // schedule all pending events
        ke_event_schedule();
    }
    #endif //KE_SUPPORT
}
bool rwip_sleep(void)
{
    bool proc_sleep = false;
    #if (DEEP_SLEEP)
    uint32_t sleep_duration = MAX_SLEEP_DURATION_EXTERNAL_WAKEUP;
    #endif //DEEP_SLEEP

    DBG_SWDIAG(SLEEP, ALGO, 0);

    do
    {
        /************************************************************************
         **************            CHECK KERNEL EVENTS             **************
         ************************************************************************/
        // Check if some kernel processing is ongoing
        if (!ke_sleep_check())
            break;
        // Processor sleep can be enabled
//        proc_sleep = true;

        DBG_SWDIAG(SLEEP, ALGO, 1);
        #if (DEEP_SLEEP)
        /************************************************************************
         **************             CHECK ENABLE FLAG              **************
         ************************************************************************/
        // Check sleep enable flag
        if(!rwip_env.sleep_enable)
            break;
        /************************************************************************
         **************              CHECK RW FLAGS                **************
         ************************************************************************/
        // First check if no pending procedure prevent from going to sleep
        if (rwip_env.prevent_sleep != 0)
            break;
        DBG_SWDIAG(SLEEP, ALGO, 2);

        /************************************************************************
         **************           CHECK EXT WAKEUP FLAG            **************
         ************************************************************************/
        /* If external wakeup enable, sleep duration can be set to maximum, otherwise
         *  system should be woken-up periodically to poll incoming packets from HCI */
        if(!rwip_env.ext_wakeup_enable)
            sleep_duration = MAX_SLEEP_DURATION_PERIODIC_WAKEUP;

        /************************************************************************
         * Wait until there's enough time for SLP to restore clocks when the chip
         * wakes up.
         ************************************************************************/
        while(!rwip_check_wakeup_boundary());

        /************************************************************************
         **************            CHECK KERNEL TIMERS             **************
         ************************************************************************/
        // Compute the duration up to the next software timer expires
        if (!ke_timer_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;

        DBG_SWDIAG(SLEEP, ALGO, 3);

        #if (BT_EMB_PRESENT)
        /************************************************************************
         **************                 CHECK BT                   **************
         ************************************************************************/
        if (!rwbt_sleep_check())
            break;
        #endif //BT_EMB_PRESENT

        DBG_SWDIAG(SLEEP, ALGO, 4);

        /************************************************************************
         **************                 CHECK EA                   **************
         ************************************************************************/
        if (!ea_sleep_check(&sleep_duration, rwip_env.wakeup_delay))
            break;

        DBG_SWDIAG(SLEEP, ALGO, 4);

        #if (TL_ITF)
        /************************************************************************
         **************                 CHECK TL                   **************
         ************************************************************************/
        // Try to switch off TL
        if (!h4tl_stop())
        {
            proc_sleep = false;
            break;
        }
        #endif //TL_ITF
        #if (GTL_ITF)
        /************************************************************************
         **************                 CHECK TL                   **************
         ************************************************************************/
        // Try to switch off Transport Layer
        if (!gtl_enter_sleep())
        {
            proc_sleep = false;
            break;
        }
        #endif //GTL_ITF

        DBG_SWDIAG(SLEEP, ALGO, 5);
        // Processor sleep can be enabled
        proc_sleep = true;

        /************************************************************************
         **************          PROGRAM CORE DEEP SLEEP           **************
         ************************************************************************/
        #if (BT_EMB_PRESENT)
        // Put BT core into deep sleep
//        ld_sleep_enter(rwip_slot_2_lpcycles(sleep_duration), rwip_env.ext_wakeup_enable);
        #elif (BLE_EMB_PRESENT)
        // Put BT core into deep sleep
        lld_sleep_enter(rwip_slot_2_lpcycles(sleep_duration), rwip_env.ext_wakeup_enable);
        #endif //BT_EMB_PRESENT / BT_EMB_PRESENT

        DBG_SWDIAG(SLEEP, SLEEP, 1);

        /************************************************************************
         **************               SWITCH OFF RF                **************
         ************************************************************************/
        rwip_rf.sleep();

        #endif // DEEP_SLEEP
    } while(0);

    return proc_sleep;
}

#if (DEEP_SLEEP)
void rwip_wakeup(void)
{
    DBG_SWDIAG(SLEEP, SLEEP, 0);

    // Prevent going to deep sleep until a slot interrupt is received
    rwip_prevent_sleep_set(RW_WAKE_UP_ONGOING);

    #if (BT_EMB_PRESENT)
    // Wake-up BT core
//    ld_sleep_wakeup();
    #elif (BLE_EMB_PRESENT)
    // Wake-up BLE core
    lld_sleep_wakeup();
    #endif //BT_EMB_PRESENT / BLE_EMB_PRESENT

    #if (TL_ITF)
    // Restart the flow on the TL
    h4tl_start();
    #endif //TL_ITF

    #if (GTL_ITF)
    // Restart the flow on the GTL
    gtl_exit_sleep();
    #endif //GTL
}

void rwip_wakeup_end(void)
{
    if(rwip_env.prevent_sleep & RW_WAKE_UP_ONGOING)
    {
        #if (BT_EMB_PRESENT)
        // Wake-up BT core
//        ld_sleep_wakeup_end();
        #elif (BLE_EMB_PRESENT)
        // Wake-up BLE core
        lld_sleep_wakeup_end();
        #endif //BT_EMB_PRESENT / BLE_EMB_PRESENT

        // Schedule the kernel timers
        ke_event_set(KE_EVENT_KE_TIMER);

        // Wake up is complete now, so we allow the deep sleep again
        rwip_prevent_sleep_clear(RW_WAKE_UP_ONGOING);
    }
}

void rwip_wakeup_delay_set(uint16_t wakeup_delay)
{
    rwip_env.wakeup_delay = (uint8_t)(wakeup_delay / SLOT_SIZE);
}

void rwip_prevent_sleep_set(uint16_t prv_slp_bit)
{
    GLOBAL_INT_DISABLE();
    rwip_env.prevent_sleep |= prv_slp_bit;
    DBG_SWDIAG(SLEEP, PREVENT, rwip_env.prevent_sleep);
    GLOBAL_INT_RESTORE();
}

void rwip_prevent_sleep_clear(uint16_t prv_slp_bit)
{
    GLOBAL_INT_DISABLE();
    rwip_env.prevent_sleep &= ~prv_slp_bit;
    DBG_SWDIAG(SLEEP, PREVENT, rwip_env.prevent_sleep);
    GLOBAL_INT_RESTORE();
}

bool rwip_sleep_enable(void)
{
    return rwip_env.sleep_enable;
}

bool rwip_ext_wakeup_enable(void)
{
    return rwip_env.ext_wakeup_enable;
}
#endif// DEEP_SLEEP

///@} RW
