/**
 ****************************************************************************************
 *
 * @file resmgmt.c
 *
 * @brief Resource management API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
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
 * \addtogroup OS_RELATED
 * \{
 */

#if !defined(OS_BAREMETAL)

#include <stdbool.h>
#include <osal.h>
#include <resmgmt.h>
#include <sdk_defs.h>
#include <interrupts.h>

/**
 * \brief Bit-mask that holds all allocated resources
 *
 * Every 1 in this says that resource is acquired.
 *
 */
__RETAINED static resource_mask_t acquired_resources;

/**
 * \brief Structure to hold pending resource requests
 *
 */
typedef struct resource_request {
        struct resource_request  *next;       /**< Next node in list */
        resource_mask_t           mask;       /**< Requested resource mask */
        uint8_t                   granted;    /**< Set to 1 when requested resource are granted */
        OS_EVENT                  wait_event; /**< Synchronization primitive to use for waiting */
} resource_request;

/**
 * \brief Initial size of table holding resource requests.
 *
 * Resource requests internally use OS events for waiting. In some OS'es creating event
 * can be costly operation so resource management reserves some events for tasks to use.
 * In final solution MAX_RESOURCE_REQUEST should be trimmed to value that is enough for
 * application.
 * For development phase best way to use is to define CONFIG_RESOURCE_MANAGEMENT_DYNAMIC_MEMORY,
 * this will allow dynamic allocation of more requests in runtime (at cost of additional time
 * needed when all other request are waiting).
 *
 */
#if !defined(MAX_RESOURCE_REQUEST)

#if defined(CONFIG_RESOURCE_MANAGEMENT_DYNAMIC_MEMORY)
#define MAX_RESOURCE_REQUEST 4
#else
#define MAX_RESOURCE_REQUEST 10
#endif

#endif // MAX_RESOURCE_REQUEST

/**
 * \brief Pre-allocated request table
 *
 */
__RETAINED static resource_request requests[MAX_RESOURCE_REQUEST];

/**
 * \brief List holds all requests that are not used
 *
 * After resource_init(), all requests from request table are added to this list.
 *
 */
__RETAINED static resource_request *free_list;

/**
 * \brief List holds all requests that are currently waiting
 *
 */
__RETAINED static resource_request *waiting_list;

/**
 * \brief Remove element from list
 *
 * \param [in,out] list pointer to address of first element in list
 * \param [in] item pointer to element to remove from list
 *
 */
static void list_remove(resource_request **list, resource_request *item)
{
        /* Cannot remove from an empty list */
        ASSERT_ERROR(list != NULL);

        while (*list != item && *list != NULL) {
                list = &(*list)->next;
        }

        /* Check whether the item was actually found in the list */
        ASSERT_WARNING(*list != NULL);

        if (*list != NULL) {
                *list = item->next;
        }
}

void resource_init(void)
{
        int i;

        free_list = NULL;
        waiting_list = NULL;

        for (i = MAX_RESOURCE_REQUEST - 1; i >= 0; --i) {
                OS_EVENT_CREATE(requests[i].wait_event);
                requests[i].granted = 0;
                requests[i].next = free_list;
                free_list = &requests[i];
        }
}

resource_mask_t resource_acquire(resource_mask_t resource_mask, uint32_t timeout)
{
        resource_mask_t ret = 0;
        bool timed_out;

        OS_ENTER_CRITICAL_SECTION();
        if ((resource_mask & acquired_resources) == 0) {
                // Requested resources are not taken, just take them and leave.
                acquired_resources |= resource_mask;
                ret = acquired_resources;
        } else if (timeout != 0) {
                resource_request *request;
                if (free_list == NULL) {
#if !defined(CONFIG_RESOURCE_MANAGEMENT_DYNAMIC_MEMORY)
                        ASSERT_ERROR(0);
                        request = NULL; // To satisfy the compiler
#else
                        OS_LEAVE_CRITICAL_SECTION();
                        request = OS_MALLOC(sizeof(*request));
                        ASSERT_ERROR(request);
                        OS_EVENT_CREATE(request->wait_event);
                        OS_ENTER_CRITICAL_SECTION();
#endif
                } else {
                        request = (resource_request *) free_list;
                        free_list = free_list->next;
                }
                request->mask = resource_mask;
                request->granted = 0;
                request->next = waiting_list;
                waiting_list = request;
                OS_LEAVE_CRITICAL_SECTION();

                timed_out = OS_EVENT_WAIT(request->wait_event, timeout) != OS_EVENT_SIGNALED;
                // Even if timeout happened, check whether access was granted
                // this will remove races
                OS_ENTER_CRITICAL_SECTION();
                list_remove(&waiting_list, request);
                if (request->granted) {
                        ret = resource_mask;
                        // If timeout occurred yet access was granted one additional wait event
                        // is needed to clear event so next time this event is used it starts
                        // in non-signaled state.
                        if (timed_out) {
                                OS_EVENT_WAIT(request->wait_event, 0);
                        }
                }
                request->next = free_list;
                free_list = request;
        }
        OS_LEAVE_CRITICAL_SECTION();
        return ret;
}

void resource_release(resource_mask_t resource_mask)
{
        resource_request *request;

        /* Must provide a valid resource mask, and the resource must be
         * already acquired */
        ASSERT_ERROR(resource_mask != 0);
        ASSERT_ERROR((resource_mask & acquired_resources) == resource_mask);

        OS_ENTER_CRITICAL_SECTION();

        acquired_resources &= ~resource_mask;
        for (request = waiting_list; request != NULL; request = request->next) {
                if ((request->mask & acquired_resources) != 0) {
                        continue;
                }
                request->granted = 1;
                acquired_resources |= request->mask;
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(request->wait_event);
                } else {
                        OS_EVENT_SIGNAL(request->wait_event);
                }
        }

        OS_LEAVE_CRITICAL_SECTION();
}

#ifndef CONFIG_NO_DYNAMIC_RESOURCE_ID

__RETAINED_RW static uint8_t max_resource_id = RES_ID_COUNT;

int resource_add(void)
{
        uint8_t id;

        OS_ENTER_CRITICAL_SECTION();
        id = max_resource_id++;

        /*
         * Assertion to make sure new id produces non-zero resource mask. In case it does not, the
         * configuration option CONFIG_LARGE_RESOURCE_ID can be used to increase limit for number
         * of ids allowed.
         */
        ASSERT_WARNING(RES_MASK(id));

        OS_LEAVE_CRITICAL_SECTION();

        return id;
}

#endif

#endif /* !defined(OS_BAREMETAL) */

/**
 * \}
 * \}
 * \}
 */
