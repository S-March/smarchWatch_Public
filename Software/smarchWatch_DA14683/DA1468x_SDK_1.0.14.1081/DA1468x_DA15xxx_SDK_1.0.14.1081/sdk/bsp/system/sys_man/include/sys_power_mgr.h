/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup POWER_MANAGER
 * 
 * \brief Power Manager
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_power_mgr.h
 *
 * @brief Power manager header file.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SYS_POWER_MGR_H_
#define SYS_POWER_MGR_H_

#include <stdint.h>
#include <stdbool.h>
#include "sdk_defs.h"
#include "osal.h"

typedef void (*periph_init_cb)(void);
typedef int32_t pm_id_t;

typedef enum {
        pm_mode_active,
        pm_mode_idle,
        pm_mode_extended_sleep_no_mirror,
        pm_mode_extended_sleep,
        pm_mode_hibernation,
} sleep_mode_t;

typedef struct _adptCB {
        bool (*ad_prepare_for_sleep)(void);
        void (*ad_sleep_canceled)(void);
        void (*ad_wake_up_ind)(bool);
        void (*ad_xtal16m_ready_ind)(void);
        uint8_t ad_sleep_preparation_time;
} adapter_call_backs_t;

typedef enum system_state_type {
        sys_active = 0,
        sys_idle,
        sys_powered_down,
} system_state_t;

#define PM_BASE_ID      (1000)
#define PM_MAX_ID       (1001)

#ifdef CONFIG_USE_BLE
#define PM_BLE_ID       (1000)
#endif

#ifdef CONFIG_USE_FTDF
#define PM_FTDF_ID      (1001)
#undef PM_MAX_ID
#define PM_MAX_ID       (1002)
#endif


/*
 * Variables declarations
 */

extern uint32_t lp_last_trigger;
extern adapter_call_backs_t *pm_adapters_cb[dg_configPM_MAX_ADAPTERS_CNT];
extern uint16_t pm_wakeup_xtal16m_time;
extern bool adapters_wake_up_ind_called;
extern bool call_adapters_xtal16m_ready_ind;

/**
 * Initialization tree node.
 */
typedef void (*comp_init_func)(void *);
typedef struct comp_init_tree {
        comp_init_func init_fun;                /**< Initialization function */
        void *init_arg;                         /**< Argument for init_fun */
        const struct comp_init_tree * const *depend; /**< List of nodes this node depends on */
} comp_init_tree_t;

/**
 * \brief Component initialization declaration
 *
 * This macro declares component that depends on arbitrary number of other components.
 *
 * param [in] _comp Name of component, this name can be used in other component declarations
 *                     dependency.
 * param [in] _init Function to call during initialization of component.
 * param [in] _deps NULL terminated array of components that should be initialized before.
 *
 */
#define COMPONENT_INIT_WITH_DEPS(_comp, _init, _init_arg, _deps, _sect) \
        __USED \
        const comp_init_tree_t _comp = { _init, (void *)_init_arg, _deps }; \
        __USED \
        const comp_init_tree_t *const _comp##_ptr __attribute__ ((section (#_sect "_init_section"))) = &_comp; \

/**
 * \brief bus initialization declaration
 *
 * param [in] _id bus id
 * param [in] _init initialization function
 * param [in] _init_arg argument to pass to _init function during bus initialization
 *
 */
#define BUS_INIT(_id, _init, _init_arg) \
        COMPONENT_INIT_WITH_DEPS(_id, (comp_init_func)_init, _init_arg, NULL, bus)

/**
 * \brief device initialization declaration
 *
 * param [in] _id device id
 * param [in] _init initialization function
 * param [in] _init_arg argument to pass to _init function during device initialization
 *
 */
#define DEVICE_INIT(_id, _init, _init_arg) \
        COMPONENT_INIT_WITH_DEPS(_id, (comp_init_func)_init, _init_arg, NULL, device)

#define ADAPTER_INIT_WITH_DEPS(_adapter, _init, _deps) \
        COMPONENT_INIT_WITH_DEPS(_adapter, (comp_init_func)_init, NULL, _deps, adapter)

/**
 * \brief Adapter initialization declaration
 *
 * This macro declares adapter that does not depend on any other adapters. Initialization
 * function will be called during all adapters initialization time.
 *
 * param [in] _adapter Name of adapter, this name can be used as other adapter declarations
 *                     dependency.
 * param [in] _init Function to call during initialization of adapters.
 *
 */
#define ADAPTER_INIT(_adapter, _init) \
        ADAPTER_INIT_WITH_DEPS(_adapter, _init, NULL)

/**
 * \brief Declaration of adapter with one dependency
 *
 * This macro declares adapter that depends on other adapter. Initialization
 * function will be called during all adapters initialization time.
 *
 * param [in] _adapter Name of adapter, this name can be used as other adapter declarations
 *                     dependency.
 * param [in] _init Function to call during initialization of adapters.
 * param [in] _dep1 Adapter that must be initialized before.
 *
 */
#define ADAPTER_INIT_DEP1(_adapter, _init, _dep1) \
        extern const comp_init_tree_t _dep1; \
        __USED \
        const comp_init_tree_t *const _adapter##_dep[2] = { &_dep1, NULL }; \
        ADAPTER_INIT_WITH_DEPS(_adapter, _init, _adapter##_dep)

/**
 * \brief Declaration of adapter that depends on other two
 *
 * This macro declares adapter that depends on two other adapters. Initialization
 * function will be called during all adapters initialization time.
 *
 * param [in] _adapter Name of adapter, this name can be used as other adapter declarations
 *                     dependency.
 * param [in] _init Function to call during initialization of adapters.
 * param [in] _dep1 Adapter that must be initialized before.
 * param [in] _dep2 Adapter that must be initialized before.
 *
 * \note: Order of dependencies is undefined, if there is dependency between _dep2 and _dep1
 * it should be specified in respective adapter declaration.
 *
 */
#define ADAPTER_INIT_DEP2(_adapter, _init, _dep1, _dep2) \
        extern const comp_init_tree_t _dep1; \
        extern const comp_init_tree_t _dep2; \
        __USED \
        const comp_init_tree_t *_adapter##_dep[3] = { &_dep1, &_dep2, NULL }; \
        ADAPTER_INIT_WITH_DEPS(_adapter, _init, _adapter##_dep)

/*
 * Function declarations
 */

/**
 * \brief Initialize the system after power-up.
 *
 * \param[in] peripherals_initialization Call-back to an application function that handles
 *            the initialization of the GPIOs and the peripherals.
 *
 * \warning This function will change when the initialization of the GPIOs and the peripherals is
 *          moved to the Adapters (or wherever it is decided).
 *
 */
void pm_system_init(periph_init_cb peripherals_initialization);

/**
 * \brief Wait for the debugger to detach if sleep is used.
 *
 * \param[in] mode The sleep mode of the application. It must be different than pm_mode_active if
 *            the application intends to use sleep.
 *
 */
void pm_wait_debugger_detach(sleep_mode_t mode);

/**
 * \brief Sets the state of the 1v8 rail.
 *
 * \param[in] state true, the 1v8 rail is controlled via dg_config macros
 *                  false, the 1v8 rail is off
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
void pm_set_1v8_state(bool state);

/**
 * \brief Returns the state of the 1v8 rail (whether it is off or controlled via dg_config macros).
 *
 * \return false if the 1v8 rail is off, true if it is controlled via dg_config macros
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
bool pm_get_1v8_state(void);

/**
 * \brief Sets the wake-up mode of the system (whether the OS will be resumed with RC16 or XTAL16).
 *
 * \param[in] wait_for_xtal16m This is the new wake-up mode.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
void pm_set_wakeup_mode(bool wait_for_xtal16m);

/**
 * \brief Returns the wake-up mode of the system (whether the OS will be resumed with RC16 or XTAL16).
 *
 * \return The wake-up mode.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
bool pm_get_wakeup_mode(void);

/**
 * \brief Sets the sleep mode of the system.
 *
 * \param[in] mode This is the new sleep mode.
 *
 * \return The previous mode set.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
sleep_mode_t pm_set_sleep_mode(sleep_mode_t mode);

/**
 * \brief Returns the sleep mode of the system.
 *
 * \return The sleep mode used.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
sleep_mode_t pm_get_sleep_mode(void);

/**
 * \brief Sets the system in active mode.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
void pm_stay_alive(void);

/**
 * \brief Allows the system to go to idle mode.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
void pm_stay_idle(void);

/**
 * \brief Restores the sleep mode of the system, which has been blocked via a call to
 *        pm_stay_alive().
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
void pm_resume_sleep(void);

/**
 * \brief Registers an Adapter to the Power Manager.
 *
 * \param[in] cb The pointer to the set of the call-back functions of this Adapter.
 *
 * \return pm_id_t The registered Adapter's ID in the Power Manager state table.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
pm_id_t pm_register_adapter(const adapter_call_backs_t *cb);

/**
 * \brief Unregisters an Adapter with a specific ID from the Power Manager.
 *
 * \param[in] id The ID of the Adapter in the Power Manager.
 *
 * \warning The function will block if another task is accessing the Power Manager.
 *
 */
void pm_unregister_adapter(pm_id_t id);

/**
 * \brief Informs the PM when a MAC has planned to wake-up.
 *
 * \param[in] id The ID of the MAC.
 * \param[in] time_in_LP_cycles The offset from the current system time in (non-prescaled) Low Power
 *            clock cycles, when the caller has planned to be active. In other words, it is the time
 *            that the MAC will be sleeping, counting from this moment.
 *
 * \warning Cannot be called from Interrupt Context! Must be called with ALL interrupts disabled!
 *
 */
void pm_resource_sleeps_until(pm_id_t id, uint32_t time_in_LP_cycles);

/**
 * \brief Informs the PM when a MAC has waken-up.
 *
 * \param[in] id The ID of the MAC.
 *
 * \warning The function never blocks! Is is called from Interrupt Context!
 *
 */
void pm_resource_is_awake(pm_id_t id);

/**
 * \brief Called by an Adapter to ask the PM not to go to sleep for some short period.
 *
 * \param[in] id The ID of the Adapter.
 * \param[in] time_in_LP_cycles The offset from the current system time, in (non-prescaled) Low
 *            Power clock cycles, until when the caller requests the system to stay active.
 *
 * \warning Called from Interrupt Context! Must be called with ALL interrupts disabled!
 *
 */
void pm_defer_sleep_for(pm_id_t id, uint32_t time_in_LP_cycles);

/**
 * \brief Called to retrieve the next wakeup time for a MAC
 *
 * \param[in] id The ID of the MAC

 * \return The time when MAC with id needs to wakeup
 *
 * \warning If the returned value equals pm_wakeup_xtal16m_time, then this
 *          MAC will either not sleep, or sleep forever
 *
 */
uint64_t pm_get_mac_wakeup_time(pm_id_t id);

/*
 * \brief Put the system to idle or sleep or blocks in a WFI() waiting for the next tick, if neither
 *        idle nor sleep is possible.
 *
 * \detail Puts the system to idle or sleep, if possible. If an exit-from-idle or a wake-up is
 *         needed, it programs the Timer1 to generate an interrupt after the specified idle or sleep
 *         period. Else, the system stays forever in idle or sleep mode.
 *         If neither idle nor sleep is possible, it blocks in a WFI() call waiting for the next
 *         (already programmed) OS tick to hit.
 *
 * \param[in] xLowPowerPeriods The number of (prescaled) low power clock periods the OS will be
 *            idle. If it is 0 then the OS indicates that it can block forever waiting for an
 *            external event. If the system goes to sleep, then it can wake up only from an external
 *            event in this case.
 *
 * \warning Must be called with interrupts disabled!
 *
 */
__RETAINED_CODE void pm_sleep_enter(uint32_t low_power_periods);

/*
 * \brief When continuing from the WFI(), check if we were sleeping and, if so, power-up the system.
 *
 * \warning Must be called with interrupts disabled!
 *
 */
void pm_system_wake_up(void);

/**
 * \brief Advances time from the previous tick that hit.
 *
 * \details Calculate how many ticks have passed since the last tick.
 *
 * \return uint32_t The number of ticks passed.
 *
 */
__RETAINED_CODE uint32_t pm_advance_time(uint32_t prescaled_time);

#if (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)

typedef struct _pm_qspi_ops {
        OS_TASK handle;
        uint32_t addr;
        const uint8_t *buf;
        uint16_t *size;
        uint16_t written;
        bool op_type;           /* false: erase, true: program */
        bool suspended;
        struct _pm_qspi_ops *next;
} qspi_ops;

/**
 * \brief Register a program or erase QSPI operation to be executed by the CPM in background
 *
 * \param [in] handle the handle of the task that registers the operation
 * \param [in] addr starting offset
 * \param [in] buf the buffer containing the data to be written
 * \param [inout] len pointer to the length of the data in the buffer
 * \param [out] op pointer to a structure allocated by the CPM that must be freed later by the
 *             caller of this function
 *
 * \returns true if the operation was registered successfully, else false
 */
bool pm_register_qspi_operation(OS_TASK handle, uint32_t addr, const uint8_t *buf, uint16_t *len,
        void **op);

/**
 * \brief Execute the "active WFI" when Flash operations should be processed in the background
 *
 */
__RETAINED_CODE void pm_execute_active_wfi(void);

/**
 * \brief Notify tasks waiting for Flash operations that they have been completed
 *
 */
__RETAINED_CODE void pm_process_completed_qspi_operations(void);
#endif

#define pm_conv_ticks_2_prescaled_lpcycles( x )   ( (x) * TICK_PERIOD )
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
#define pm_conv_ms_2_prescaled_lpcycles( x )      ( (x) * configSYSTICK_CLOCK_HZ / (1 + dg_configTim1Prescaler) / 1000 )
#else
#define pm_conv_ms_2_prescaled_lpcycles( x )      ( (x) * configSYSTICK_CLOCK_HZ / 1000 )
#endif

#endif /* SYS_POWER_MGR_H_ */

/**
 \}
 \}
 \}
 */
