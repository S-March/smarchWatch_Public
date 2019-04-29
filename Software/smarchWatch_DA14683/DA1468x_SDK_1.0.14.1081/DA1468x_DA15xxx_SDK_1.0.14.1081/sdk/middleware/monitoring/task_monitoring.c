/**
 * \addtogroup BSP
 * \{
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup TASK_MONITORING
 * \{
 */

/*
 *****************************************************************************************
 *
 * @file task_monitoring.c
 *
 * @brief Monitoring Task functions
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */


#include "osal.h"
#include "task_monitoring.h"
#include <stdio.h>

#if (dg_configENABLE_TASK_MONITORING == 1)

#if !(defined (CONFIG_RTT) || defined (CONFIG_RETARGET))
#error dg_configENABLE_TASK_MONITORING must be used only when CONFIG_RTT or CONFIG_RETARGET are defined.
#endif

#ifdef CONFIG_RETARGET
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

typedef struct {
        uint16_t used;
        uint16_t id;
        OS_TASK task;
} mon_task_t;

/* array used to store information about monitored task*/
__RETAINED static mon_task_t mon_stat[MAX_NUMBER_OF_MONITORED_TASKS];

static char * task_state(OS_TASK_STATE state)
{
        switch (state) {
        case OS_TASK_RUNNING:
                return "Running";
        case OS_TASK_READY:
                return "Ready";
        case OS_TASK_BLOCKED:
                return "Blocked";
        case OS_TASK_SUSPENDED:
                return "Suspended";
        case OS_TASK_DELETED:
                return "Deleted";
        default:
                return "Unknown";
        }
}

void tm_register_monitor_task(uint16_t id)
{
        uint16_t i = 0;
        OS_TASK task = OS_GET_CURRENT_TASK();
        for (i=0; i < MAX_NUMBER_OF_MONITORED_TASKS; i++) {
                if ( mon_stat[i].used == 0 ) {
                        mon_stat[i].used = 1;
                        mon_stat[i].id = id;
                        mon_stat[i].task = task;
                        return;
                }
        }

        OS_ASSERT(0); /*increase the number of monitored tasks */
}

void tm_unregister_monitor_task(uint16_t id)
{
        uint16_t i = 0;
        for (i=0; i < MAX_NUMBER_OF_MONITORED_TASKS; i++) {
                if (( mon_stat[i].id == id) && (mon_stat[i].used ))  {
                        mon_stat[i].used = 0;
                        return;
                }
        }
}

static inline void print_task_status(OS_TASK task, uint16_t id )
{
        printf(NEWLINE NEWLINE "id:%d Handler 0x%p",id, task);
        printf(NEWLINE "id:%d Name \"%s\"",id, OS_GET_TASK_NAME(task));
        printf(NEWLINE "id:%d State %s", id, task_state(OS_GET_TASK_STATE(task)));
        printf(NEWLINE "id:%d Priority %d", id, OS_GET_TASK_PRIOTITY(task));
        printf(NEWLINE "id:%d Stack high water mark %d", id, OS_GET_STACK_WATERMARK(task));
}

void tm_print_registered_tasks()
{
        uint16_t i =0;

        printf(NEWLINE "Printing monitored tasks");

        for (i=0; i<MAX_NUMBER_OF_MONITORED_TASKS; i++) {
                if (mon_stat[i].used) {
                        print_task_status(mon_stat[i].task, mon_stat[i].id);
                }
        }
        printf(NEWLINE "Available heap min watermark %d", OS_GET_HEAP_WATERMARK());
        printf(NEWLINE "Available current heap %d" NEWLINE, OS_GET_FREE_HEAP_SIZE());
}

void tm_print_tasks_status()
{

        uint16_t i = 0;
        uint16_t tasks_num = OS_GET_TASKS_NUMBER();
        OS_TASK_STATUS* TaskStatusArray = OS_MALLOC(tasks_num*(sizeof(OS_TASK_STATUS)));
        uint16_t tracked_tasks = OS_GET_TASKS_STATUS(TaskStatusArray, tasks_num);

        while (tracked_tasks - i) {
                printf(NEWLINE "Monitored task %d", i);
                printf(NEWLINE "Handler 0x%p", TaskStatusArray[i].xHandle);
                printf(NEWLINE "Name \"%s\"", TaskStatusArray[i].pcTaskName);
                printf(NEWLINE "State %s", task_state(TaskStatusArray[i].eCurrentState));
                printf(NEWLINE "Current priority %d", TaskStatusArray[i].uxCurrentPriority);
                printf(NEWLINE "Main priority %d", TaskStatusArray[i].uxBasePriority);
                printf(NEWLINE "Elapsed time %d", TaskStatusArray[i].ulRunTimeCounter);
                printf(NEWLINE "Stack high water mark %d" NEWLINE, TaskStatusArray[i].usStackHighWaterMark);

                i++;
        }
        OS_FREE(TaskStatusArray);
        printf(NEWLINE "Available heap min watermark %d", OS_GET_HEAP_WATERMARK());
        printf(NEWLINE "Available current heap %d" NEWLINE, OS_GET_FREE_HEAP_SIZE());
}
#endif /*dg_configENABLE_TASK_MONITORING*/

/**
 * \}
 * \}
 * \}
 */
