/**
 ****************************************************************************************
 *
 * @file prf.c
 *
 * @brief Entry point of profile source file.
 *
 * Used to manage life cycle of profiles
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup PRF
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_PROFILES)
#include "prf.h"


#if (BLE_HT_THERMOM)
#include "htpt.h"
#endif // (BLE_HT_THERMOM)

#if (BLE_HT_COLLECTOR)
#include "htpc.h"
#endif // (BLE_HT_COLLECTOR)

#if (BLE_DIS_SERVER)
#include "diss.h"
#endif // (BLE_HT_THERMOM)

#if (BLE_DIS_CLIENT)
#include "disc.h"
#endif // (BLE_DIS_CLIENT)

#if (BLE_BP_SENSOR)
#include "blps.h"
#endif // (BLE_BP_SENSOR)

#if (BLE_BP_COLLECTOR)
#include "blpc.h"
#endif // (BLE_BP_COLLECTOR)

#if (BLE_TIP_SERVER)
#include "tips.h"
#endif // (BLE_TIP_SERVER)

#if (BLE_TIP_CLIENT)
#include "tipc.h"
#endif // (BLE_TIP_CLIENT)

#if (BLE_HR_SENSOR)
#include "hrps.h"
#endif // (BLE_HR_SENSOR)

#if (BLE_HR_COLLECTOR)
#include "hrpc.h"
#endif // (BLE_HR_COLLECTOR)

#if (BLE_FINDME_LOCATOR)
#include "findl.h"
#endif // (BLE_FINDME_LOCATOR)

#if (BLE_FINDME_TARGET)
#include "findt.h"
#endif // (BLE_FINDME_TARGET)

#if (BLE_PROX_MONITOR)
#include "proxm.h"
#endif // (BLE_PROX_MONITOR)

#if (BLE_PROX_REPORTER)
#include "proxr.h"
#endif // (BLE_PROX_REPORTER)

#if (BLE_SP_CLIENT)
#include "scppc.h"
#endif // (BLE_SP_CLENT)

#if (BLE_SP_SERVER)
#include "scpps.h"
#endif // (BLE_SP_SERVER)

#if (BLE_BATT_CLIENT)
#include "basc.h"
#endif // (BLE_BATT_CLIENT)

#if (BLE_BATT_SERVER)
#include "bass.h"
#endif // (BLE_BATT_SERVER)

#if (BLE_HID_DEVICE)
#include "hogpd.h"
#endif // (BLE_HID_DEVICE)

#if (BLE_HID_BOOT_HOST)
#include "hogpbh.h"
#endif // (BLE_HID_BOOT_HOST)

#if (BLE_HID_REPORT_HOST)
#include "hogprh.h"
#endif // (BLE_HID_REPORT_HOST)

#if (BLE_GL_COLLECTOR)
#include "glpc.h"
#endif // (BLE_GL_COLLECTOR)

#if (BLE_GL_SENSOR)
#include "glps.h"
#endif // (BLE_GL_SENSOR)

#if (BLE_RSC_COLLECTOR)
#include "rscpc.h"
#endif // (BLE_RSC_COLLECTOR)

#if (BLE_RSC_SENSOR)
#include "rscps.h"
#endif // (BLE_RSC_COLLECTOR)

#if (BLE_CSC_COLLECTOR)
#include "cscpc.h"
#endif // (BLE_CSC_COLLECTOR)

#if (BLE_CSC_SENSOR)
#include "cscps.h"
#endif // (BLE_CSC_COLLECTOR)

#if (BLE_AN_CLIENT)
#include "anpc.h"
#endif // (BLE_AN_CLIENT)

#if (BLE_AN_SERVER)
#include "anps.h"
#endif // (BLE_AN_SERVER)

#if (BLE_PAS_CLIENT)
#include "paspc.h"
#endif // (BLE_PAS_CLIENT)

#if (BLE_PAS_SERVER)
#include "pasps.h"
#endif // (BLE_PAS_SERVER)

#if (BLE_CP_COLLECTOR)
#include "cppc.h"
#endif //(BLE_CP_COLLECTOR)

#if (BLE_CP_SENSOR)
#include "cpps.h"
#endif //(BLE_CP_SENSOR)

#if (BLE_LN_COLLECTOR)
#include "lanc.h"
#endif //(BLE_CP_COLLECTOR)

#if (BLE_LN_SENSOR)
#include "lans.h"
#endif //(BLE_CP_SENSOR)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * MACROS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
struct prf_env_tag prf_env;

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve profile interface
 ****************************************************************************************
 */
static const struct prf_task_cbs * prf_itf_get(uint16_t task_id)
{
    const struct prf_task_cbs* prf_cbs = NULL;

    switch(KE_TYPE_GET(task_id))
    {
        #if (BLE_HT_THERMOM)
        case TASK_ID_HTPT:
            prf_cbs = htpt_prf_itf_get();
            break;
        #endif // (BLE_HT_THERMOM)

        #if (BLE_HT_COLLECTOR)
        case TASK_ID_HTPC:
            prf_cbs = htpc_prf_itf_get();
            break;
        #endif // (BLE_HT_COLLECTOR)

        #if (BLE_DIS_SERVER)
        case TASK_ID_DISS:
            prf_cbs = diss_prf_itf_get();
            break;
        #endif // (BLE_DIS_SERVER)

        #if (BLE_DIS_CLIENT)
        case TASK_ID_DISC:
            prf_cbs = disc_prf_itf_get();
            break;
        #endif // (BLE_DIS_CLIENT)

        #if (BLE_BP_SENSOR)
        case TASK_ID_BLPS:
            prf_cbs = blps_prf_itf_get();
            break;
        #endif // (BLE_BP_SENSOR)

        #if (BLE_BP_COLLECTOR)
        case TASK_ID_BLPC:
            prf_cbs = blpc_prf_itf_get();
            break;
        #endif // (BLE_BP_COLLECTOR)

        #if (BLE_TIP_SERVER)
        case TASK_ID_TIPS:
            prf_cbs = tips_prf_itf_get();
            break;
        #endif // (BLE_TIP_SERVER)

        #if (BLE_TIP_CLIENT)
        case TASK_ID_TIPC:
            prf_cbs = tipc_prf_itf_get();
            break;
        #endif // (BLE_TIP_CLIENT)

        #if (BLE_HR_SENSOR)
        case TASK_ID_HRPS:
            prf_cbs = hrps_prf_itf_get();
            break;
        #endif // (BLE_HR_SENSOR)

        #if (BLE_HR_COLLECTOR)
        case TASK_ID_HRPC:
            prf_cbs = hrpc_prf_itf_get();
            break;
        #endif // (BLE_HR_COLLECTOR)

        #if (BLE_FINDME_LOCATOR)
        case TASK_ID_FINDL:
            prf_cbs = findl_prf_itf_get();
            break;
        #endif // (BLE_FINDME_LOCATOR)

        #if (BLE_FINDME_TARGET)
        case TASK_ID_FINDT:
            prf_cbs = findt_prf_itf_get();
            break;
        #endif // (BLE_FINDME_TARGET)

        #if (BLE_PROX_MONITOR)
        case TASK_ID_PROXM:
            prf_cbs = proxm_prf_itf_get();
            break;
        #endif // (BLE_PROX_MONITOR)

        #if (BLE_PROX_REPORTER)
        case TASK_ID_PROXR:
            prf_cbs = proxr_prf_itf_get();
            break;
        #endif // (BLE_PROX_REPORTER)

        #if (BLE_SP_SERVER)
        case TASK_ID_SCPPS:
            prf_cbs = scpps_prf_itf_get();
            break;
        #endif // (BLE_SP_SERVER)

        #if (BLE_SP_CLIENT)
        case TASK_ID_SCPPC:
            prf_cbs = scppc_prf_itf_get();
            break;
        #endif // (BLE_SP_CLIENT)

        #if (BLE_BATT_SERVER)
        case TASK_ID_BASS:
            prf_cbs = bass_prf_itf_get();
            break;
        #endif // (BLE_BATT_SERVER)

        #if (BLE_BATT_CLIENT)
        case TASK_ID_BASC:
            prf_cbs = basc_prf_itf_get();
            break;
        #endif // (BLE_BATT_CLIENT)

        #if (BLE_HID_DEVICE)
        case TASK_ID_HOGPD:
            prf_cbs = hogpd_prf_itf_get();
            break;
        #endif // (BLE_HID_DEVICE)

        #if (BLE_HID_BOOT_HOST)
        case TASK_ID_HOGPBH:
            prf_cbs = hogpbh_prf_itf_get();
            break;
        #endif // (BLE_HID_BOOT_HOST)

        #if (BLE_HID_REPORT_HOST)
        case TASK_ID_HOGPRH:
            prf_cbs = hogprh_prf_itf_get();
            break;
        #endif // (BLE_HID_REPORT_HOST)

        #if (BLE_GL_COLLECTOR)
        case TASK_ID_GLPC:
            prf_cbs = glpc_prf_itf_get();
            break;
        #endif // (BLE_GL_COLLECTOR)

        #if (BLE_GL_SENSOR)
        case TASK_ID_GLPS:
            prf_cbs = glps_prf_itf_get();
            break;
        #endif // (BLE_GL_SENSOR)

        #if (BLE_RSC_COLLECTOR)
        case TASK_ID_RSCPC:
            prf_cbs = rscpc_prf_itf_get();
            break;
        #endif // (BLE_RSC_COLLECTOR)

        #if (BLE_RSC_SENSOR)
        case TASK_ID_RSCPS:
            prf_cbs = rscps_prf_itf_get();
            break;
        #endif // (BLE_RSC_SENSOR)

        #if (BLE_CSC_COLLECTOR)
        case TASK_ID_CSCPC:
            prf_cbs = cscpc_prf_itf_get();
            break;
        #endif // (BLE_CSC_COLLECTOR)

        #if (BLE_CSC_SENSOR)
        case TASK_ID_CSCPS:
            prf_cbs = cscps_prf_itf_get();
            break;
        #endif // (BLE_CSC_SENSOR)

        #if (BLE_CP_COLLECTOR)
        case TASK_ID_CPPC:
            prf_cbs = cppc_prf_itf_get();
            break;
        #endif // (BLE_CP_COLLECTOR)

        #if (BLE_CP_SENSOR)
        case TASK_ID_CPPS:
            prf_cbs = cpps_prf_itf_get();
            break;
        #endif // (BLE_CP_SENSOR)

        #if (BLE_LN_COLLECTOR)
        case TASK_ID_LANC:
            prf_cbs = lanc_prf_itf_get();
            break;
        #endif // (BLE_LN_COLLECTOR)

        #if (BLE_LN_SENSOR)
        case TASK_ID_LANS:
            prf_cbs = lans_prf_itf_get();
            break;
        #endif // (BLE_LN_SENSOR)

        #if (BLE_AN_CLIENT)
        case TASK_ID_ANPC:
            prf_cbs = anpc_prf_itf_get();
            break;
        #endif // (BLE_AN_CLIENT)

        #if (BLE_AN_SERVER)
        case TASK_ID_ANPS:
            prf_cbs = anps_prf_itf_get();
            break;
        #endif // (BLE_AN_SERVER)

        #if (BLE_PAS_CLIENT)
        case TASK_ID_PASPC:
            prf_cbs = paspc_prf_itf_get();
            break;
        #endif // (BLE_PAS_CLIENT)

        #if (BLE_PAS_SERVER)
        case TASK_ID_PASPS:
            prf_cbs = pasps_prf_itf_get();
            break;
        #endif // (BLE_PAS_SERVER)

        default: /* Nothing to do */ break;
    }

    return prf_cbs;
}

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
void prf_init_sdk(bool reset)
{
    uint8_t i;
    if(!reset)
    {
        // FW boot profile initialization
        for(i = 0; i < BLE_NB_PROFILES ; i++)
        {
            prf_env.prf[i].env  = NULL;
            prf_env.prf[i].task = TASK_GAPC + i +1;
            prf_env.prf[i].id   = TASK_ID_INVALID;

            // Initialize Task Descriptor
            prf_env.prf[i].desc.default_handler = NULL;
            prf_env.prf[i].desc.state           = NULL;
            prf_env.prf[i].desc.idx_max         = 0;

            prf_env.prf[i].desc.state_max       = 0;
            prf_env.prf[i].desc.state_handler   = NULL;

            ke_task_create(prf_env.prf[i].task, &(prf_env.prf[i].desc));
        }
    }
    else
    {
        // FW boot profile destruction
        for(i = 0; i < BLE_NB_PROFILES ; i++)
        {
            // Get Profile API
            const struct prf_task_cbs * cbs = prf_itf_get(prf_env.prf[i].id);
            if(cbs != NULL)
            {
                // request to destroy profile
                cbs->destroy(&(prf_env.prf[i]));
            }
            // unregister profile
            prf_env.prf[i].id   = TASK_ID_INVALID;
            prf_env.prf[i].desc.default_handler = NULL;
            prf_env.prf[i].desc.state           = NULL;
            prf_env.prf[i].desc.idx_max         = 0;

            // Request kernel to flush task messages
            ke_task_msg_flush(KE_TYPE_GET(prf_env.prf[i].task));
        }
    }
}


uint8_t prf_add_profile_sdk(struct gapm_profile_task_add_cmd * params, ke_task_id_t* prf_task)
{
    uint8_t i;
    uint8_t status = GAP_ERR_NO_ERROR;

    // retrieve profile callback
    const struct prf_task_cbs * cbs = prf_itf_get(params->prf_task_id);
    if(cbs == NULL)
    {
        // profile API not available
        status = GAP_ERR_INVALID_PARAM;
    }

    // check if profile not already present in task list
    if(status == GAP_ERR_NO_ERROR)
    {
        for(i = 0; i < BLE_NB_PROFILES ; i++)
        {
            if(prf_env.prf[i].id == params->prf_task_id)
            {
                status = GAP_ERR_NOT_SUPPORTED;
                break;
            }
        }
    }

    if(status == GAP_ERR_NO_ERROR)
    {
        // find fist available task
        for(i = 0; i < BLE_NB_PROFILES ; i++)
        {
            // available task found
            if(prf_env.prf[i].id == TASK_ID_INVALID)
            {
                // initialize profile
                status = cbs->init(&(prf_env.prf[i]), &(params->start_hdl), params->app_task, params->sec_lvl, params->param);

                // initialization succeed
                if(status == GAP_ERR_NO_ERROR)
                {
                    // register profile
                    prf_env.prf[i].id = params->prf_task_id;
                    *prf_task = prf_env.prf[i].task;
                }
                break;
            }
        }

        if(i == BLE_NB_PROFILES)
        {
            status = GAP_ERR_INSUFF_RESOURCES;
        }
    }

    return status;
}



void prf_create_sdk(uint8_t conidx)
{
    uint8_t i;
    /* simple connection creation handler, nothing to do. */

    // execute create function of each profiles
    for(i = 0; i < BLE_NB_PROFILES ; i++)
    {
        // Get Profile API
        const struct prf_task_cbs * cbs = prf_itf_get(prf_env.prf[i].id);
        if(cbs != NULL)
        {
            // call create callback
            cbs->create(&(prf_env.prf[i]), conidx);
        }
    }
}


void prf_cleanup_sdk(uint8_t conidx, uint8_t reason)
{
    uint8_t i;
    /* simple connection creation handler, nothing to do. */

    // execute create function of each profiles
    for(i = 0; i < BLE_NB_PROFILES ; i++)
    {
        // Get Profile API
        const struct prf_task_cbs * cbs = prf_itf_get(prf_env.prf[i].id);
        if(cbs != NULL)
        {
            // call cleanup callback
            cbs->cleanup(&(prf_env.prf[i]), conidx, reason);
        }
    }
}


prf_env_t* prf_env_get(uint16_t prf_id)
{
    prf_env_t* env = NULL;
    uint8_t i;
    // find if profile present in profile tasks
    for(i = 0; i < BLE_NB_PROFILES ; i++)
    {
        // check if profile identifier is known
        if(prf_env.prf[i].id == prf_id)
        {
            env = prf_env.prf[i].env;
            break;
        }
    }

    return env;
}

ke_task_id_t prf_src_task_get(prf_env_t* env, uint8_t conidx)
{
    ke_task_id_t task = PERM_GET(env->prf_task, PRF_TASK);

    if(PERM_GET(env->prf_task, PRF_MI))
    {
        task = KE_BUILD_ID(task, conidx);
    }

    return task;
}

ke_task_id_t prf_dst_task_get(prf_env_t* env, uint8_t conidx)
{
    ke_task_id_t task = PERM_GET(env->app_task, PRF_TASK);

    if(PERM_GET(env->app_task, PRF_MI))
    {
        task = KE_BUILD_ID(task, conidx);
    }

    return task;
}


ke_task_id_t prf_get_id_from_task_sdk(ke_msg_id_t task)
{
    ke_task_id_t id = TASK_ID_INVALID;
    uint8_t idx = KE_IDX_GET(task);
    uint8_t i;
    task = KE_TYPE_GET(task);

    // find if profile present in profile tasks
    for(i = 0; i < BLE_NB_PROFILES ; i++)
    {
        // check if profile identifier is known
        if(prf_env.prf[i].task == task)
        {
            id = prf_env.prf[i].id;
            break;
        }
    }

    return KE_BUILD_ID(id, idx);
}

ke_task_id_t prf_get_task_from_id_sdk(ke_msg_id_t id)
{
    ke_task_id_t task = TASK_NONE;
    uint8_t idx = KE_IDX_GET(id);
    uint8_t i;
    id = KE_TYPE_GET(id);

    // find if profile present in profile tasks
    for(i = 0; i < BLE_NB_PROFILES ; i++)
    {
        // check if profile identifier is known
        if(prf_env.prf[i].id == id)
        {
            task = prf_env.prf[i].task;
            break;
        }
    }

    return KE_BUILD_ID(task, idx);
}


#endif // (BLE_PROFILES)

/// @} PRF
