DA1468x QSPI Flash Support {#flash_support}
==========================

## Overview

This document describes the QSPI Flash support in DA14681 SDK and the steps required to add support for new Flash types.

## Modes of operation and configuration

The SDK supports two modes of operation: **Autodetect** mode and **Manual** mode. The *Autodetect* mode is able to detect the flash type on runtime, while the *Manual* mode involves explicitly declaring the used flash in the project.

**Warning**: The default and recommended mode is the **Manual Mode**. The Autodetect Mode will greatly increase code size and Retained RAM usage, up to the point that a project may possibly not fit in RAM anymore.

### Autodetect Mode

The Autodetect mode detects the flash that is used in runtime, and selects the proper flash driver to use. The Autodetect mode can only detect among the flash devices officially supported by the SDK. If no match is found, a default driver will be used (that may or may not work). 
Since the Autodetect mode needs to select the driver to use in runtime, it has to keep the code for all the drivers in the binary. Moreover, it has to keep the selected driver's configuration parameters in Retained RAM. Therefore, the Autodetect mode is **NOT recommended for production builds**. 

### Manual Mode

The Manual mode simply consists of a hardcoded declaration of the flash driver to use. Therefore, only the code of the selected driver needs to be compiled in the binary, and there is no need to retain the driver parameters in Retained RAM, since the compiler optimizes them out. This mode is **suitable for Production builds**.

### Flash Configuration

The Flash subsystem is configured using the following macros (that must be defined in custom_config.h of the project):

- dg_configFLASH_AUTODETECT (Default: 0): This macro, if set, enables the Autodetect Mode. Please note, that **use of this macro is NOT recommended.**
- dg_configFLASH_HEADER_FILE: This macro must be defined as a string named after the header file to use for the specific flash driver. E.g. \"qspi_w25q80ew.h\". This header file must be either one of the qspi_<part_nr>.h header files found under \<SDK ROOT\>/sdk/bsp/memory/include/, or a header file under the project's folder, **as long as this path is the compiler include path** (see the document section about adding support for new flash devices).
- dg_configFLASH_MANUFACTURER_ID: This macro must be defined to the Flash manufacturer ID, as defined in the respective driver header file (e.g. WINBOND_ID)
- dg_configFLASH_DEVICE_TYPE: This macro must be defined to the corresponding device type macro, as defined in the driver header file (e.g. W25Q80EW).
- dg_configFLASH_DENSITY: This macro must be defined to the corresponding device density macro, as defined in the driver header file (W25Q_8Mb_SIZE).

When the system is in *Manual Mode* (dg_configFLASH_AUTODETECT == 0), which is the default, all the three macros above are implicitly defined to enable the default flash used, which is the **Winbond W25Q80EW**.

## Code Structure

The QSPI Flash access functionality is implemented in files qspi_automode.[ch]. Common command definitions and functions needed for all devices are declared in qspi_common.h. Device specific code is defined in header files named qspi_<flash device name>.h.

The code in qspi_automode.c (and in some other parts of the SDK, as well), calls device-specific functions and uses device-specific values in order to properly initialize the flash device. Each driver header file provides an instance of the structure *qspi_flash_config_t* to the main driver, containing all the device-specific function pointers and variables.

### The flash configuration structure qspi_flash_config_t

Each driver header file must provide its own instance of qspi_flash_config_t. Please note that **this instance must be named with a distinctive name**, like flash_<device name>_config, since all the device header files are included by qspi_automode.c, and, therefore, there is a single, global namespace. Also, please note that the struct instance **must be declared as const** in order to be optimized by the compiler.

The *qspi_flash_config_t* structure has the following fields (see qspi_common.h for more information):

- *initialize*: Pointer to the flash-specific initialization function.
- *is_suspended*: Pointer to a flash-specific function that checks if flash is in erase/program suspend state.
- *deactivate_command_entry_mode*: Pointer to a flash-specific function that performs extra steps needed when command entry mode is deactivated
- *sys_clk_cfg*: Pointer to a flash-specific function that performs Flash configuration when system clock is changed (e.g. change dummy bytes or QSPIC clock divider).
- *get_dummy_bytes*: Pointer to a flash-specific function that returns the number of dummy bytes currently needed (they may e.g. change when the clock changes).
- *manufacturer_id*: The Flash JEDEC vendor ID (Cmd 0x9F, 1st byte). This (and the device_type/device_density) are needed for flash autodetection, when on Autodetect mode.
- *device_type*: The Flash JEDEC device type (Cmd 0x9F, 2nd byte).
- *device_density*: The Flash JEDEC device type (Cmd 0x9F, 3rd byte).
- *erase_opcode*: The Flash erase opcode to use.
- *erase_suspend_opcode*: The Flash erase suspend opcode to use.
- *erase_resume_opcode*: The Flash erase resume opcode to use.
- *page_program_opcode*: The Flash page program opcode to use.
- *quad_page_program_address*: If true, the address will be transmitted in QUAD mode when writing a page. Otherwise, it will be transmitted in serial mode.
- *read_erase_progress_opcode*: The opcode to use to check if erase is in progress (Usually the Read Status Reg opcode (0x5).
- *erase_in_progress_bit*: The bit to check when reading the erase progress.
- *erase_in_progress_bit_high_level*: The active state (true: high, false: low) of the bit above.
- *send_once*: If set to 1, the "Performance mode" (or burst, or continuous; differs per vendor) will be used for read accesses. In this mode, the read opcode is only sent once, and subsequent accesses only transfer the address.
- *extra_byte*: The extra byte to transmit, when in "Performance mode" (send once is 1), that tells the flash that it should stay in this continuous, performance mode.
- *address_size*: Whether the flash works in 24- or 32-bit addressing mode.
- *break_seq_size*: Whether the break sequence, that puts flash out of the continuous mode, is one or two bytes long (the break byte is 0xFF).
- *ucode_wakeup*: The QSPIC microcode to use to setup the flash on wakeup. This is automatically used by the QSPI Controller after wakeup, and before CPU starts code execution. This is different based on whether flash was active, in deep power down or completely off while the system was sleeping.
- *power_down_delay*: This is the time, in usec, needed for the flash to go to power down, after the Power Down command is issued.
- *release_power_down_delay*: This is the time, in usec, needed for the flash to exit the power down mode, after the Release Power Down command is issued.

When on *Autodetect mode*, these structures reside in the .rodata section of the code. As soon as the flash subsystem is initialized, it reads the flash JEDEC ID (command 0x9F) to find out which is the actual flash that is used. It then uses the JEDEC ID to select the corresponding flash_<flash device>_config structure, and copies it in the Retained RAM. It then uses it for all the flash operations that need it.

When on "Manual mode", no JEDED ID is read and no copy is performed to the Retained RAM. Instead, the constant pointer flash_config is directly initialized (inside the flash-specific driver file) to the specific (and constant) flash_<device name>_config structure. The compiler then optimizes out the entire structure.

## Adding support for a new flash device

The SDK driver subsystem currently supports a specific set of QSPI flash devices. It provides, however, the capability to add support for other flash devices as well.

Each device driver must reside on its own header file, that should be named qspi_<device name>.h. The programmer can either use the qspi_XXX_template.h, or start from an existing driver file. The new flash driver file should be placed inside the project's path, in a folder that is in the compiler's include path (an obvious choice is the config/ folder, but others can be used as well).

Common code among flash families or vendors can be factored out in common header file per family/vendor. There are currently such common header files, like qspi_macronix.h and qspi_winbond.h . However, this is NOT necessary; moreover, it is the responsibility of the device driver header file to include the common header file, if needed.

Please note that a custom flash driver **can ONLY be used in Manual mode**, that is, the four macros described in a previous section **MUST be defined in custom_config.h**.

The following steps are usually needed to create the new flash driver:

1. Copy and rename the template header file, or an existing driver file, into place.
2. Rename all the functions and variables accordingly. It is important to remember that **all the drivers reside in the same namespace, and, therefore, all function and variable names must be unique**.
3. Define the proper JEDEC ID values for the Manufacturer code, the device type and the device density
4. Verify that the suspend, resume, exit power-down, enter power-down, fast read, write enable, read status register are valid for the new device type.
5. Guard the header file using an \#if preprocessor macro that checks for the specific driver selection.
6. Define any other driver-specific macros that are needed (like timings etc).
7. Define the constant wakeup microcode arrays that will be needed, per configuration mode that will be supported (dg_configFLASH_POWER_OFF, dg_configFLASH_POWER_DOWN or none of them). The microcode will be copied during the driver initialization in a special memory in the QSPI controller, and will be used after system wakeup to initialize the QSPI (since the CPU isn't yet running code at this time). Please see the DA1468x Datasheet for the uCode format.
8. Declare the constant struct instance, of type qspi_flash_config_t, named flash_<device name>_config, and initialize it with proper values. Please note that this **must be declared as const**.
9. Extend the function flash_<device name>_initialize() if needed, e.g. to write some special QSPI configuration registers or the QUAD enable bit. Otherwise, leave empty.
10. Extend the function flash_<device name>_sys_clock_cfg() function if needed. This can included modifying the dummy bytes when the system (and hence the QSPI clock) changes, or changing the QSPI clock divider (if, for example, the flash device cannot cope with 96MHz). Otherwise, leave empty.
11. The function is_suspended() should read the flash Status Register and return true if Erase or Write is suspended on the device.
12. If *Continuous Read Mode* (sometimes referred to as *Performance Mode*) is used, make sure to set **send_once** to 1, and set **extra_byte** to a proper value for the flash to keep working in this mode. This is flash-specific
13. If the flash supports *32-bit addressing*, make sure to use the proper **ucode for wakeup**, as well as the corresponding **page_program_opcode**, **erase_opcode**, **break_seq_size** (this should also take into consideration whether the device will be working in Continuous Read mode as well) and **address_size**.
14. If the address, during write, will be provided in QUAD mode, set **quad_page_program_address** to true.

Please note that the SDK supports reading in QUAD I/O mode (where the address and data are read in QUAD mode, and only the command is transferred in serial mode), both in Continuous Read and normal mode. 
