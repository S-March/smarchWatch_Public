/**
 ****************************************************************************************
 *
 * @file llm.h
 *
 * @brief Main API file for the Link Layer manager
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 ****************************************************************************************
 */

#ifndef LLM_H_
#define LLM_H_

/**
 ****************************************************************************************
 * @addtogroup CONTROLLER
 * @ingroup ROOT
 * @brief BLE Lower Layers
 *
 * The CONTROLLER contains the modules allowing the physical link establishment,
 * maintenance and management.
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup LLM LLM
 * @ingroup CONTROLLER
 * @brief Link Layer Manager
 *
 * The link layer manager contains the modules allowing the physical link establishment,
 * and all the non-connected states.
 * @{
 ****************************************************************************************
 */




/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "co_bt.h"
#include "co_math.h"
#include "co_utils.h"
#include "co_buf.h"
#include "co_version.h"
#include "llm_task.h"
#include "rwip.h"
#include "ea.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#if (BLE_CHNL_ASSESS)
/// Default Channel Assessment Timer duration (5s - Multiple of 10ms)
#define LLM_CHNL_ASSESS_DFLT_TIMER_DUR      (500)
/// Default Channel Reassessment Timer duration (Multiple of Channel Assessment Timer duration)
#define LLM_CHNL_REASSESS_DFLT_TIMER_DUR    (8)
/// Default Minimal RSSI Threshold - -60dBm
#define LLM_CHNL_ASSESS_DFLT_MIN_RSSI       (-60)
/// Default number of packets to receive for statistics
#define LLM_CHNL_ASSESS_DFLT_NB_PKT         (20)
/// Default number of bad packets needed to remove a channel
#define LLM_CHNL_ASSESS_DFLT_NB_BAD_PKT     (LLM_CHNL_ASSESS_DFLT_NB_PKT / 2)
#endif //(BLE_CHNL_ASSESS)

#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define LE_LENGTH_EXT_OCTETS_MIN            27
#define LE_LENGTH_EXT_OCTETS_MAX            251
#define LE_LENGTH_EXT_TIME_MIN              328
#define LE_LENGTH_EXT_TIME_MAX              2120

#define LE_LENGTH_EXT_SUPPORTED_MAXRXOCTETS (REG_BLE_EM_RX_BUFFER_SIZE - 11)
#define LE_LENGTH_EXT_SUPPORTED_MAXTXOCTETS (REG_BLE_EM_TX_BUFFER_SIZE - 11)
#define LE_LENGTH_EXT_SUPPORTED_MAXRXTIME   ((REG_BLE_EM_RX_BUFFER_SIZE+3)*8)
#define LE_LENGTH_EXT_SUPPORTED_MAXTXTIME   ((REG_BLE_EM_TX_BUFFER_SIZE+3)*8)

//#define LE_LENGTH_EXT_INITIAL_MAXTXOCTETS (REG_BLE_EM_TX_BUFFER_SIZE - 11)
//#define LE_LENGTH_EXT_INITIAL_MAXTXTIME   ((REG_BLE_EM_TX_BUFFER_SIZE-1)*8)
#define LE_LENGTH_EXT_INITIAL_MAXTXOCTETS   LE_LENGTH_EXT_OCTETS_MIN
#define LE_LENGTH_EXT_INITIAL_MAXTXTIME     LE_LENGTH_EXT_TIME_MIN

//RPA default 0x384 = 900 secs or 15 minutes
#define RPA_TIMEOUT_DEFAULT                 0x0384
//RPA min 1 sec
#define RPA_TIMEOUT_MIN                     0x0001
//RPA max 0xA1B8 approx 11.5 hours
#define RPA_TIMEOUT_MAX                     0xA1B8

#define RPA_INUSE_PEER                      0x0f
#define RPA_INUSE_LOCAL                     0xf0

#define LLM_P256_STATE_IDLE      0
#define LLM_P256_STATE_TRNG      1
#define LLM_P256_STATE_PKMULT    2
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// Advertising parameters
struct advertising_pdu_params
{
    /// Pointer on the data adv request
    struct ke_msg * adv_data_req;
    /// Connection interval min
    uint16_t intervalmin;
    /// Connection interval max
    uint16_t intervalmax;
    /// Channel mapping
    uint8_t channelmap;
    /// Filtering policy
    uint8_t filterpolicy;
    /// Advertising type
    uint8_t type;
    /// Data length
    uint8_t datalen;
    /// Scan RSP length
    uint8_t scanrsplen;
    /// Local address type
    uint8_t own_addr_type;
    /// Advertising periodicity: true for low duty cycle, false for high duty cycle
    bool adv_ldc_flag;
#if (RWBLE_SW_VERSION_MINOR >= 1)
    /// Peer address type: public=0x00 / random = 0x01
    uint8_t peer_addr_type;
    /// Peer Bluetooth device address used for IRK selection
    struct bd_addr peer_addr;
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
};

///Scanning parameters
struct scanning_pdu_params
{
    /// Scan interval
    uint16_t interval;
    /// Scan window
    uint16_t window;
    /// Filtering policy
    uint8_t filterpolicy;
    /// Scanning type
    uint8_t type;
    /// Duplicate the advertising report
    uint8_t filter_duplicate;
    /// Local address type
    uint8_t own_addr_type;
};

///Access address generation structure
struct access_addr_gen
{
    /// random
    uint8_t intrand;
    /// index 1
    uint8_t ct1_idx;
    /// index 2
    uint8_t ct2_idx;
};

/// Advertising report list
struct adv_device_list
{
    /// Header
    struct co_list_hdr hdr;
    /// Advertising type
    uint8_t adv_type;
    /// Advertising device address
    struct bd_addr adv_addr;
};

//advertising pdu
///structure adv undirected
struct llm_pdu_adv
{
    /// advertising address
    struct bd_addr  adva;
    /// advertising data
    uint8_t         *adva_data;
};
///structure adv directed
struct llm_pdu_adv_directed
{
    /// advertising address
    struct bd_addr  adva;
    /// initiator address
    struct bd_addr  inita;
};

//scanning pdu
///structure scan request
struct llm_pdu_scan_req
{
    /// scanning address
    struct bd_addr  scana;
    /// advertising address
    struct bd_addr  adva;
};
///structure scan response
struct llm_pdu_scan_rsp
{
    /// advertising address
    struct bd_addr  adva;
    /// scan response data
    uint8_t         *scan_data;

};
///initiating pdu
///structure connection request reception
struct llm_pdu_con_req_rx
{
    /// initiator address
    struct bd_addr      inita;
    /// advertiser address
    struct bd_addr      adva;
    /// access address
    struct access_addr  aa;
    /// CRC init
    struct crc_init     crcinit;
    /// window size
    uint8_t             winsize;
    /// window offset
    uint16_t            winoffset;
    /// interval
    uint16_t            interval;
    /// latency
    uint16_t            latency;
    /// timeout
    uint16_t            timeout;
    /// channel mapping
    struct le_chnl_map  chm;
    /// hopping
    uint8_t             hop_sca;
};
///structure connection request transmission
struct llm_pdu_con_req_tx
{
    /// access address
    struct access_addr  aa;
    /// CRC init
    struct crc_init     crcinit;
    /// window size
    uint8_t             winsize;
    /// window offset
    uint16_t            winoffset;
    /// interval
    uint16_t            interval;
    /// latency
    uint16_t            latency;
    /// timeout
    uint16_t            timeout;
    /// channel mapping
    struct le_chnl_map  chm;
    /// hopping
    uint8_t             hop_sca;
};

///structure for the test mode
struct llm_test_mode
{
    /// flag indicating the end of test
    bool end_of_tst;
    /// Direct test type
    uint8_t  directtesttype;
};

/// LLM environment structure to be saved
struct llm_le_env_tag
{
    /// List of encryption requests
    struct co_list enc_req;

    #if (BLE_CENTRAL || BLE_OBSERVER)
    /// Advertising reports filter policy
    struct co_list adv_list;

    /// Scanning parameters
    struct scanning_pdu_params *scanning_params;
    #endif //(BLE_CENTRAL || BLE_OBSERVER)

    #if (BLE_BROADCASTER || BLE_PERIPHERAL)
    /// Advertising parameters
    struct advertising_pdu_params *advertising_params;
    #endif //(BLE_BROADCASTER || BLE_PERIPHERAL)

    #if (BLE_CENTRAL || BLE_PERIPHERAL)
    /// Connected bd address list
    struct co_list cnx_list;
    #endif //(BLE_CENTRAL || BLE_PERIPHERAL)

    /// Event mask
    struct evt_mask eventmask;

    /// Access address
    struct access_addr_gen aa;

    ///protection for the command
    bool llm_le_set_host_ch_class_cmd_sto;

    /// conhdl_allocated
    uint16_t conhdl_alloc;

    #if (BLE_CHNL_ASSESS)
    /// Duration of channel assessment timer
    uint16_t chnl_assess_timer;
    /// Max number of received packets
    uint16_t chnl_assess_nb_pkt;
    /// Max number of received bad packets
    uint16_t chnl_assess_nb_bad_pkt;
    #endif // (BLE_CHNL_ASSESS)

    /// Element
    struct ea_elt_tag *elt;

    ///encryption pending
    bool enc_pend;

    ///test mode
    struct llm_test_mode test_mode;

    /// Active link counter
    uint8_t cpt_active_link;

    /// Current channel map
    struct le_chnl_map ch_map;

    /// random bd_address
    struct bd_addr rand_add;

    /// public bd_address
    struct bd_addr public_add;

    /// current @type in the register
    uint8_t curr_addr_type;

    #if (BLE_CHNL_ASSESS)
    /// Minimal received signal strength
    int8_t chnl_assess_min_rssi;
    /// Counter value used for channel reassessment
    uint8_t chnl_reassess_cnt_val;
    /// Counter used for channel reassessment
    uint8_t chnl_reassess_cnt;
    #endif //(BLE_CHNL_ASSESS)

#if (RWBLE_SW_VERSION_MAJOR >= 8)
    uint16_t connInitialMaxTxOctets;
    uint16_t connInitialMaxTxTime;
    uint16_t supportedMaxTxOctets;
    uint16_t supportedMaxTxTime;
    uint16_t supportedMaxRxOctets;
    uint16_t supportedMaxRxTime;
    
    uint8_t address_resolution_enable;
    struct co_list llm_resolving_list;
    uint16_t rpa_timeout;
    
    /// Local address type
    uint8_t own_addr_type;
    /// Resolving list being used for AIR_OP
    struct ll_resolving_list *rl;
    /// Resolving list being used for own address
    struct ll_resolving_list *rlown;
    /// bitfiled for timer usage for local/peer RPA
    uint8_t timer;
    /// Peer address type in Initiating state
    uint8_t peer_addr_type;
    /// Peer bd_address in Initiating state
    struct bd_addr peer_addr;
    
    uint8_t llm_resolving_list_index;
    struct co_list resolve_pending_events;
    
    /// List of P256 requests
    struct co_list p256_req;
    uint8_t llm_p256_private_key[ECDH_KEY_LEN];
    uint8_t llm_p256_state;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/// Flow cntl structure
struct flow_control_ble
{
    /// flow control enabled
    bool flow_cntl_en;
    /// host packet size max
    uint16_t pkt_size;
    /// host packet number max
    uint16_t pkt_nb;
    /// current packet available
    uint16_t curr_pkt_nb;
};

///LLM environment tag BT
struct llm_bt_env_tag
{
    struct flow_control_ble flow_cntl;
};

#if (RWBLE_SW_VERSION_MAJOR >= 8)
/// Tx buffer tag
struct ll_pending_events
{
    struct co_list_hdr hdr;
    bool direct_adv;
    struct bd_addr inita_addr;
    struct ll_resolving_list *rl;
    void * event;
};

struct ll_resolving_list
{
    /// List element for chaining in the resolving list
    struct co_list_hdr  hdr;
    
    uint8_t     inuse;
    ///16-byte array for local IRK value
    uint8_t     local_irk[KEY_LEN];
    ///16-byte array for peer IRK value
    uint8_t     peer_irk[KEY_LEN];
    /// Peer device Identiry Address
    struct bd_addr identity_addr;
    /// Address type of the device 0=public/1=private random
    uint8_t identity_addr_type;	
    /// Local device Address
    struct bd_addr local_addr;
    /// Peer device Address
    struct bd_addr peer_addr;
#if (RWBLE_SW_VERSION_MINOR >= 1)
    /// Peer device Privacy Mode
    uint8_t privacy_mode;
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
};

#define LLM_RESOLVING_LIST_SIZE 10
extern struct ll_resolving_list llm_resolving_list[];
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */



extern const struct supp_cmds llm_local_cmds;
extern const struct le_features llm_local_le_feats;
extern const struct le_states llm_local_le_states;

/*
 * DEFINES
 ****************************************************************************************
 */
#if (RWBLE_SW_VERSION_MAJOR >= 8)
/// Length of resolvable random address prand part
#define LLM_RAND_ADDR_PRAND_LEN            (3)
/// Length of resolvable random address hash part
#define LLM_RAND_ADDR_HASH_LEN             (3)

/// Random Address type
enum llm_rnd_addr_type
{
    /// Static random address           - 11 (MSB->LSB)
    LLM_STATIC_ADDR     = 0xC0,
    /// Private non resolvable address  - 00 (MSB->LSB)
    LLM_NON_RSLV_ADDR   = 0x00,
    /// Private resolvable address      - 01 (MSB->LSB)
    LLM_RSLV_ADDR       = 0x40,
};
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/// Advertising channel TX power
#define LLM_ADV_CHANNEL_TXPWR                   rwip_rf.txpwr_max

/// Advertising set parameters range min
#define LLM_ADV_INTERVAL_MIN                    32//(0x20)
/// Advertising set parameters range max
#define LLM_ADV_INTERVAL_MAX                    16384//(0x4000)

/// Scanning set parameters range min
#define LLM_SCAN_INTERVAL_MIN                    4//(0x4)
/// Scanning set parameters range max
#define LLM_SCAN_INTERVAL_MAX                    16384//(0x4000)

/// Scanning set parameters range min
#define LLM_SCAN_WINDOW_MIN                    4//(0x4)
/// Scanning set parameters range max
#define LLM_SCAN_WINDOW_MAX                    16384//(0x4000)


/// In case where the adv_type is set to non connectable or discoverable
#if (RWBLE_SW_VERSION_MINOR < 1)
#define LLM_ADV_INTERVAL_MIN_NONCON_DISC        160 //(0xA0)
#else
#define LLM_ADV_INTERVAL_MIN_NONCON_DISC        rom_cfg_table[llm_adv_interval_min_noncon_disc_pos] //160 (0xA0) - 5.0 NonConnAdv
#endif /* (RWBLE_SW_VERSION_MINOR < 1) */

/// Time out value for the advertising direct event
#define LLM_LE_ADV_TO_DIRECTED                  1024  //10,24 s -> 1024 ticks(10ms)

/// Default value for the number of advertising report
#define LLM_LE_ADV_REPORT_DFT                   0x1

/// Frequency max for the receiver test mode
#define RX_TEST_FREQ_MAX                        39

/// Length max for the receiver test mode
#define TX_TEST_LEN_MAX                         37

/// Number max of good channel
#define LE_NB_CH_MAP_MAX                        37

/// default irq interrupt threshold
#define RX_THR_DFT                              1

#if (RWBLE_SW_VERSION_MINOR < 1)
/// Index dedicated for the advertising pdu
enum
{
    LLM_LE_ADV_DUMMY_IDX = (BLE_TX_DESC_DATA + BLE_TX_DESC_CNTL - 1),
    #if (BLE_OBSERVER || BLE_PERIPHERAL || BLE_CENTRAL)
    LLM_LE_SCAN_CON_REQ_ADV_DIR_IDX,
    #endif // BLE_OBSERVER || BLE_PERIPHERAL || BLE_CENTRAL
    #if (BLE_BROADCASTER || BLE_PERIPHERAL)
    LLM_LE_SCAN_RSP_IDX,
    #endif // BLE_BROADCASTER || BLE_PERIPHERAL
    LLM_LE_ADV_IDX
};
#else
extern unsigned int LLM_LE_ADV_DUMMY_IDX;
extern unsigned int LLM_LE_SCAN_CON_REQ_ADV_DIR_IDX;
extern unsigned int LLM_LE_SCAN_RSP_IDX;
extern unsigned int LLM_LE_ADV_IDX;

/// Index dedicated for the advertising pdu
enum
{
    _LLM_LE_ADV_DUMMY_IDX = (_BLE_TX_DESC_DATA + _BLE_TX_DESC_CNTL - 1),
    #if (BLE_OBSERVER || BLE_PERIPHERAL || BLE_CENTRAL)
    _LLM_LE_SCAN_CON_REQ_ADV_DIR_IDX,
    #endif // BLE_OBSERVER || BLE_PERIPHERAL || BLE_CENTRAL
    #if (BLE_BROADCASTER || BLE_PERIPHERAL)
    _LLM_LE_SCAN_RSP_IDX,
    #endif // BLE_BROADCASTER || BLE_PERIPHERAL
    _LLM_LE_ADV_IDX
};
#endif /* (RWBLE_SW_VERSION_MINOR < 1) */

/// Advertising Access Address
#define LLM_LE_ADV_AA    0x8E89BED6

/// Scanning default interval (10ms)
#define LLM_LE_SCAN_INTERV_DFLT       16 //(0X10)

/// Advertising default interval (1,28s)
#define LLM_LE_ADV_INTERV_DFLT        2048 //(0X800)

/// Advertising default channel map (ch37, ch38, ch39)
#define LLM_LE_ADV_CH_MAP_DFLT        0X7


/// Features byte 0
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define BLE_FEATURES_BYTE0  (BLE_ENC_FEATURE | BLE_CON_PARAM_REQ_PROC_FEATURE | BLE_REJ_IND_EXT_FEATURE | BLE_SLAVE_INIT_EXCHG_FEATURE | BLE_PING_FEATURE \
        | BLE_LE_LENGTH_FEATURE | BLE_LL_PRIVACY_FEATURE | BLE_SCANNER_FILT_FEATURE)
#else
#define BLE_FEATURES_BYTE0  (BLE_ENC_FEATURE | BLE_CON_PARAM_REQ_PROC_FEATURE | BLE_REJ_IND_EXT_FEATURE | BLE_SLAVE_INIT_EXCHG_FEATURE | BLE_PING_FEATURE)
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
/// Features byte 1
#define BLE_FEATURES_BYTE1  0x00
/// Features byte 2
#define BLE_FEATURES_BYTE2  0x00
/// Features byte 3
#define BLE_FEATURES_BYTE3  0x00
/// Features byte 4
#define BLE_FEATURES_BYTE4  0x00
/// Features byte 5
#define BLE_FEATURES_BYTE5  0x00
/// Features byte 6
#define BLE_FEATURES_BYTE6  0x00
/// Features byte 7
#define BLE_FEATURES_BYTE7  0x00

/// States byte 0
#define BLE_STATES_BYTE0    ( BLE_NON_CON_ADV_STATE | BLE_DISC_ADV_STATE\
                            | BLE_CON_ADV_STATE | BLE_HDC_DIRECT_ADV_STATE\
                            | BLE_PASS_SCAN_STATE | BLE_ACTIV_SCAN_STATE\
                            | BLE_INIT_MASTER_STATE | BLE_CON_SLAVE_STATE)
/// States byte 1
#define BLE_STATES_BYTE1    ( BLE_NON_CON_ADV_PASS_SCAN_STATE | BLE_DISC_ADV_PASS_SCAN_STATE\
                            | BLE_CON_ADV_PASS_SCAN_STATE | BLE_HDC_DIRECT_ADV_PASS_SCAN_STATE\
                            | BLE_NON_CON_ADV_ACTIV_SCAN_STATE | BLE_DISC_ADV_ACTIV_SCAN_STATE\
                            | BLE_CON_ADV_ACTIV_SCAN_STATE | BLE_HDC_DIRECT_ADV_ACTIV_SCAN_STATE)

/// States byte 2
#define BLE_STATES_BYTE2    ( BLE_NON_CON_ADV_INIT_STATE | BLE_DISC_ADV_INIT_STATE\
                            | BLE_NON_CON_ADV_MASTER_STATE | BLE_DISC_ADV_MASTER_STATE\
                            | BLE_NON_CON_ADV_SLAVE_STATE | BLE_DISC_ADV_SLAVE_STATE\
                            | BLE_PASS_SCAN_INIT_STATE | BLE_ACTIV_SCAN_INIT_STATE)

/// States byte 3
#define BLE_STATES_BYTE3    ( BLE_PASS_SCAN_MASTER_STATE | BLE_ACTIV_SCAN_MASTER_STATE\
                            | BLE_PASS_SCAN_SLAVE_STATE | BLE_ACTIV_SCAN_SLAVE_STATE\
                            | BLE_INIT_MASTER_MASTER_STATE | BLE_LDC_DIRECT_ADV_STATE\
                            | BLE_LDC_DIRECT_ADV_PASS_SCAN_STATE | BLE_LDC_DIRECT_ADV_ACTIV_SCAN_STATE)

/// States byte 4
#define BLE_STATES_BYTE4    ( BLE_CON_ADV_INIT_MASTER_SLAVE_STATE | BLE_HDC_DIRECT_ADV_INIT_MASTER_SLAVE_STATE\
                            | BLE_LDC_DIRECT_ADV_INIT_MASTER_SLAVE_STATE | BLE_CON_ADV_MASTER_SLAVE_STATE\
                            | BLE_HDC_DIRECT_ADV_MASTER_SLAVE_STATE | BLE_LDC_DIRECT_ADV_MASTER_SLAVE_STATE\
                            | BLE_CON_ADV_SLAVE_SLAVE_STATE | BLE_HDC_DIRECT_ADV_SLAVE_SLAVE_STATE)

/// States byte 5
#define BLE_STATES_BYTE5    ( BLE_LDC_DIRECT_ADV_SLAVE_SLAVE_STATE |  BLE_INIT_MASTER_SLAVE_STATE)
/// States byte 6
#define BLE_STATES_BYTE6    0x0
/// States byte 7
#define BLE_STATES_BYTE7    0x0

///Le Initiating enables
enum
{
    INIT_DIS                  = 0x00,
    INIT_EN,
    INIT_EN_END
};
///Le Direct test types
enum
{
    TEST_TX                  = 0x00,
    TEST_RX,
    TEST_RXTX,
    TEST_END
};


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct llm_le_env_tag llm_le_env;
extern struct llm_bt_env_tag llm_bt_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the BLE LLM task
 *
 * This function initializes the the LLC task, as well as the environment of the LLM
 *
 ****************************************************************************************
 */
void llm_init(bool reset);

/**
 ****************************************************************************************
 * @brief Sends the NOP event.
 *
 * This function sends the No Operation command completed event to the host when all the
 * Initializations are done.
 *
 ****************************************************************************************
 */
void llm_ble_ready(void);

/**
 ****************************************************************************************
 * @brief Handle the command clear the white list.
 *
 * This function clear the public and private white lists.
 *
 ****************************************************************************************
 */
void llm_wl_clr(void);

/**
 ****************************************************************************************
 * @brief Handle the command set advertising parameters.
 *
 * This function checks the parameters , fulfill the advertising packet with the useful
 * parameters
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to setup the advertising mode.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_set_adv_param(struct hci_le_set_adv_param_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command set advertising data.
 *
 * This function checks the parameters , fulfill the advertising packet with the data
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to setup the advertising data.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
#if (BLE_BROADCASTER || BLE_PERIPHERAL)
uint8_t llm_set_adv_data(struct hci_le_set_adv_data_cmd const *param);
#endif

/**
 ****************************************************************************************
 * @brief Handle the command set advertising enable.
 *
 * This function checks the parameters , starts or stops the requested advertising mode.
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to start or stop the advertising mode.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_set_adv_en(struct hci_le_set_adv_en_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command set scanning parameters.
 *
 * This function checks the parameters , fulfill the scanning packet with the useful
 * parameters
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to setup the scanning mode.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_set_scan_param(struct hci_le_set_scan_param_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command set scanning enable.
 *
 * This function checks the parameters , starts or stops the requested scanning mode.
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to start or stop the scanning mode.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_set_scan_en(struct hci_le_set_scan_en_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command set scanning response data.
 *
 * This function checks the parameters , fulfill the scanning packet with the data
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to setup the scanning response data.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_set_scan_rsp_data(struct hci_le_set_scan_rsp_data_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command start transmit test mode.
 *
 * This function checks the parameters , set the transmit mode parameters, turn on the
 * mode and set the LLM state.
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to setup the transmit test mode.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_test_mode_start_tx(struct hci_le_tx_test_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command start receive test mode.
 *
 * This function checks the parameters , set the receive mode parameters, turn on the
 * mode and set the LLM state.
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to setup the receive test mode.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_test_mode_start_rx(struct hci_le_rx_test_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command add device in the white list.
 *
 * This function adds the device in the white list, according to the type (public or
 * private)
 *
 * @param[in] param        Pointer on the structure which contains the type and the
 *                         address of the device to add.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_wl_dev_add(struct hci_le_add_dev_to_wlst_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command remove device in the white list.
 *
 * This function removes the device in the white list, according to the type (public or
 * private)
 *
 * @param[in] param        Pointer on the structure which contains the type and the
 *                         address of the device to remove.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
uint8_t llm_wl_dev_rem(struct hci_le_add_dev_to_wlst_cmd const *param);

/**
 ****************************************************************************************
 * @brief Handle the command create connection.
 *
 * This function checks the parameters, fulfill the connect_req pdu with the useful
 * parameters, changes the state of the LLM task and requests the LLD to schedule the
 * connection initiation.
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to create the link.
 *
 * @return The status of the parameters validity
 *
 ****************************************************************************************
 */
#if (RWBLE_SW_VERSION_MAJOR >= 8)
uint8_t llm_create_con(struct hci_le_create_con_cmd *param);
#else
uint8_t llm_create_con(struct hci_le_create_con_cmd const *param);
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/**
 ****************************************************************************************
 * @brief Handle the command start encryption.
 *
 * This function sets the key and the data to be encrypted in the encryption engine and
 * start it.
 *
 * @param[in] param        Pointer on the structure which contains all the parameters
 *                         needed to star tthe encryption engine.
 *
 ****************************************************************************************
 */
void llm_encryption_start(struct llm_enc_req const *param);

/**
 ****************************************************************************************
 * @brief Handle the end of the encryption process.
 *
 * This function provide to the host or the LLC the data encrypted by the engine.
 *
 ****************************************************************************************
 */
void llm_encryption_done(void);

/**
 ****************************************************************************************
 * @brief Sends the command complete event.
 *
 * This function notify the host that the command is completed.
 *
 * @param[in] opcode        Command opcode
 * @param[in] status        Status on the completion of the command.
 ****************************************************************************************
 */
void llm_common_cmd_complete_send(uint16_t opcode, uint8_t status);

/**
 ****************************************************************************************
 * @brief Sends the command status event.
 *
 * This function notify the host that the command is understood.
 *
 * @param[in] opcode        Command opcode
 * @param[in] status        Status on the completion of the command.
 ****************************************************************************************
 */
void llm_common_cmd_status_send(uint16_t opcode, uint8_t status);

/**
 ****************************************************************************************
 * @brief Sets the value for the advertising report.
 *
 * This function gets the information from the received advertising packet and sets the
 * values in the advertising report event.
 *
 * @param[out] desc        Pointer on the received advertising.
 * @param[out] event       Pointer on the advertising report event.
 *
 ****************************************************************************************
 */
void llm_adv_report_set(struct hci_le_adv_report_evt *event, struct co_buf_rx_desc *desc);

#if (RWBLE_SW_VERSION_MAJOR >= 8)
void llm_direct_adv_report_set(struct hci_le_direct_adv_report_evt *event, struct co_buf_rx_desc *desc);
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/**
 ****************************************************************************************
 * @brief Handles the connection request pdu.
 *
 * This function extracts the parameters from the packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] rxdesc         Pointer on the received pdu packet.
 *
 ****************************************************************************************
 */
void llm_con_req_ind(struct co_buf_rx_desc *rxdesc);

#if (BLE_CENTRAL || BLE_OBSERVER)
/**
 ****************************************************************************************
 * @brief Handles the advertising packet.
 *
 * This function extracts the parameters from the advertising packet received and
 * generates the event if needed
 *
 * @param[in] rxdesc         Pointer on the received advertising packet.
 *
 ****************************************************************************************
 */
void llm_le_adv_report_ind(struct co_buf_rx_desc *rxdesc);
#endif // BLE_CENTRAL || BLE_OBSERVER

/**
 ****************************************************************************************
 * @brief Handles the transmission confirmation of the packet.
 *
 * This function sends the connection complete event to the host and start the LLC task
 * for the dedicated conhdl.
 *
 * @param[in] rxdesc         Pointer on the received advertising packet who has generated
 *                           the sending of the connect_req pdu.
 *
 ****************************************************************************************
 */
void llm_con_req_tx_cfm(struct co_buf_rx_desc *rxdesc);

#if (BLE_CHNL_ASSESS)
/**
 ****************************************************************************************
 * @brief This function returns the number of packets to receive on a channel before being
 * able to disable a channel.
 *
 ****************************************************************************************
 */
uint16_t llm_get_chnl_assess_nb_pkt(void);

/**
 ****************************************************************************************
 * @brief This function returns the number of bad packets to receive on a channel before being
 * able to disable a channel.
 *
 ****************************************************************************************
 */
uint16_t llm_get_chnl_assess_nb_bad_pkt(void);

/**
 ****************************************************************************************
 * @brief This function returns the minimal RSSI value used in the Channel Assessment
 * mechanism.
 *
 ****************************************************************************************
 */
int8_t llm_get_min_rssi(void);
#endif //(BLE_CHNL_ASSESS)

#if (RWBLE_SW_VERSION_MAJOR >= 8)
extern uint8_t ble_duplicate_filter_max;
extern bool ble_duplicate_filter_found;

extern bool set_adv_data_discard_old;

extern uint8_t llm_resolving_list_max;
uint8_t llm_gen_rand_addr(struct bd_addr *addr, uint8_t addr_type, uint8_t *irk, bool inplace);
void llm_alter_conn(struct ll_resolving_list *rl, bool direct_adv);
void llm_le_scan_report_ind(struct co_buf_rx_desc *rxdesc);
void llm_resolv_addr(struct bd_addr* addr, uint8_t *irk, struct ll_pending_events *pevent);
bool llm_resolv_addr_inplace(struct bd_addr* addr, uint8_t *irk);
void llm_wl_from_rl_restore(void);

void llm_p256_start(struct ke_msg * msg);
uint8_t llm_create_p256_key(uint8_t state, uint8_t *A, uint8_t *priv);
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/// @} LLM

#endif // LLM_H_
