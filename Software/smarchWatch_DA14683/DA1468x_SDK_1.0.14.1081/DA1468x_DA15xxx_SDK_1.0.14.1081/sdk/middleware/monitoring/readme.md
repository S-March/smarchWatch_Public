Task monitoring for debugging purposes {#monitor_task}
===================================

## Overview 

This tool improves task debugging providing information about the stack size of a task and the heap usage.
In order to be used `CONFIG_RTT` or `CONFIG_RETARGET` should be configured first.

## Installation procedure
1. Create a link folder out of `sdk/middleware/monitoring/` and update the included headers of the project.
2. Import the feature RETARGET or RTT
3. Enable in custom configuration the `dg_configENABLE_TASK_MONITORING` 

## Suggested Configurable parameters

- The value of `MAX_NUMBER_OF_MONITORED_TASKS` defines the maximun number of all monitored tasks.
Its default value is 5 and is placed in `task_monitoring.h` file.

## Usage

This tool can be used by two different ways: 
- All tasks mode: Provides information from all the tasks of the project.
- Register task mode: User registers the task which is going to be monitored.

The debugging information is printed in the RTT terminal, if `CONFIG_RTT` is set, or to UART if `CONFIG_RETARGET` is set.

#### All tasks mode
User calls the function `tm_print_tasks_status()` and the debugging information is printed to the coresponding terminal.

#### Register task mode
User calls the function `tm_print_registered_tasks()` and the debugging information or the registered task is printed to the coresponding terminal.
A task is register using the function `tm_register_monitor_task(uint16_t id)` where id is user defined and helps to track the output.
A task is unregistered through the function `tm_unregister_monitor_task()`

User can determine to call the printing functions through an already created task, through a dedicated task, by a timer or even add them to `vApplicationIdleHook`

## Limitations
Both modes have the limitation that report smaller values in the stack size information for the task that calls the printing functions.
The reason is that they use space from stack in order to print the debugging information.

In all task mode the reported available heap is smaller that the actual because this mode uses space from heap in order to store the debugging info of the task into heap. 
  

