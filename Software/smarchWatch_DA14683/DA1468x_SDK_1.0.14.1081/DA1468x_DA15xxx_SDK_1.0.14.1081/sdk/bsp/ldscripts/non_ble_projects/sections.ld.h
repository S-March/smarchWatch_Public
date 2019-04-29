/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions ROM, RetRAM0 and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __image_size
 *   __mirrored_image_size
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __RetRAM0_code_start__
 *   __RetRAM0_code_end__
 *   __RetRAM0_data_end__
 *   __RetRAM0_start
 *   __RetRAM0_size
 *   __RetRAM1_start
 *   __RetRAM1_end
 */

#if (dg_configEXEC_MODE == MODE_IS_CACHED)
#define INIT_DATA_LOCATION      RAM
#else
#define INIT_DATA_LOCATION      ROM
        /* ROM is actually RAM in mirrored mode and we save memory space doing this */
#endif

ENTRY(Reset_Handler)

SECTIONS
{
        .init_text :
        {
                KEEP(*(.isr_vector))
                /* make sure that IVT doesn't cross 0xC0 */
                . = 0xC0;

                KEEP(*(.patch_table))
                . = 0x100; /* make sure that patch_table entries are exactly 16 - else fill */

                KEEP(*(.patch_table_flash))
                . = 0x130; /* make sure that patch_table_flash entries are exactly 12 - else fill */

                KEEP(*(.default_patch_code_handler_section))

#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A)
                . = 0x200; /* area 0x0 - 0x1FF is remapped to SysRAM */
#endif
                *(text_reset*)
        } > ROM


        /* To copy multiple ROM to RAM sections, uncomment .copy.table section and
         * define __STARTUP_COPY_MULTIPLE in startup_ARMCMx.S */
        .copy.table :
        {
                . = ALIGN(4);
                __copy_table_start__ = .;
#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
                LONG (__stext)
                LONG (__text_start)
                LONG ((__etext - __text_start) + (__data_end__ - __data_start__))
#endif
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                /* This will NOT work for mirrored mode but it's not needed anyway */
                LONG (__etext + (__data_end__ - __data_start__))
                LONG (__RetRAM0_code_start__)
                LONG (__RetRAM0_code_end__ - __RetRAM0_code_start__)
                LONG (__etext)
                LONG (__data_start__)
                LONG (__data_end__ - __data_start__)
#endif
                __copy_table_end__ = .;
                . = ALIGN(16);
        } > ROM

        /* To clear multiple BSS sections, uncomment .zero.table section and
         * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_ARMCMx.S */
        .zero.table :
        {
                . = ALIGN(4);
                __zero_table_start__ = .;
                LONG (__bss_start__)
                LONG (__bss_end__ - __bss_start__)
                LONG (__RetRAM0_data_start)
                LONG (__RetRAM0_size)
                LONG (__RetRAM1_start__)
                LONG (__RetRAM1_end__ - __RetRAM1_start__)
                __zero_table_end__ = .;
                . = ALIGN(16);
        } > ROM

        PROVIDE_HIDDEN (__stext = .);

#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
        ASSERT(__text_start + (__etext - __text_start) + (__data_end__ - __data_start__) < 0x7FE4000,
                "ROM size exceeded available RAM space!")

        ASSERT(__RetRAM0_data_start > __zero_table_end__,
               "Zeroing RetRAM0 data will affect the startup code!")
#endif

#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
        .text (ORIGIN(RAM) + LENGTH(RAM)) : AT (__stext)
#else
        .text :
#endif
        {
#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
                PROVIDE_HIDDEN (__text_start = .);
#endif

#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                *(EXCLUDE_FILE(*libnosys.a:sbrk.o
                               *libgcc.a:_aeabi_uldivmod.o
                               *libgcc.a:_muldi3.o
                               *libgcc.a:_dvmd_tls.o
                               *libgcc.a:bpabi.o
                               *libgcc.a:_udivdi3.o
                               *libgcc.a:_clzdi2.o
                               *libgcc.a:_clzsi2.o) .text*)
#else
                *(.text*)
                *(text_retained)
#endif

                . = ALIGN(4);

                __start_adapter_init_section = .;
                KEEP(*(adapter_init_section))
                __stop_adapter_init_section = .;

                __start_bus_init_section = .;
                KEEP(*(bus_init_section))
                __stop_bus_init_section = .;

                __start_device_init_section = .;
                KEEP(*(device_init_section))
                __stop_device_init_section = .;

                KEEP(*(.init))
                KEEP(*(.fini))

                /* .ctors */
                *crtbegin.o(.ctors)
                *crtbegin?.o(.ctors)
                *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
                *(SORT(.ctors.*))
                *(.ctors)

                /* .dtors */
                *crtbegin.o(.dtors)
                *crtbegin?.o(.dtors)
                *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
                *(SORT(.dtors.*))
                *(.dtors)

                . = ALIGN(4);
                /* preinit data */
                PROVIDE_HIDDEN (__preinit_array_start = .);
                KEEP(*(.preinit_array))
                PROVIDE_HIDDEN (__preinit_array_end = .);

                . = ALIGN(4);
                /* init data */
                PROVIDE_HIDDEN (__init_array_start = .);
                KEEP(*(SORT(.init_array.*)))
                KEEP(*(.init_array))
                PROVIDE_HIDDEN (__init_array_end = .);

                . = ALIGN(4);
                /* finit data */
                PROVIDE_HIDDEN (__fini_array_start = .);
                KEEP(*(SORT(.fini_array.*)))
                KEEP(*(.fini_array))
                PROVIDE_HIDDEN (__fini_array_end = .);

                *(.rodata*)

                KEEP(*(.eh_frame*))
        } > ROM

#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
        .ARM.extab : AT ( LOADADDR(.text) + SIZEOF(.text) )
#else
        .ARM.extab :
#endif
        {
                *(.ARM.extab* .gnu.linkonce.armextab.*)
        } > ROM

        __exidx_start = .;
#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
        .ARM.exidx : AT ( LOADADDR(.ARM.extab) + SIZEOF(.ARM.extab) )
#else
        .ARM.exidx :
#endif
        {
                *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        } > ROM
        __exidx_end = .;

        /* 16 byte alignment is required. Please do not add anything until the __etext
         * assignment! */
#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
        .align_s : AT ( LOADADDR(.ARM.exidx) + SIZEOF(.ARM.exidx) )
#else
        .align_s :
#endif
        {
                . = ALIGN(16);
        } > ROM
        __etext = .;

        /* The initialised data section is stored immediately at the end of the text section */
#if (dg_configBLACK_ORCA_IC_REV != BLACK_ORCA_IC_REV_A) && (dg_configEXEC_MODE != MODE_IS_CACHED)
        .data : AT ( LOADADDR(.align_s) + SIZEOF(.align_s) )
#else
        .data : AT (__etext)
#endif
        {
                . = ALIGN(16);
                __data_start__ = .;

                *(vtable)
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                *(EXCLUDE_FILE(*\libg_nano.a:* *\libnosys.a:*) .data*)
#else
                *(retention_mem_init)
                *(retention_mem_const)
                *(privileged_data_init)
                *(.retention)
                *(.data*)
#endif

                /* init_array/fini_array moved to flash, align preserved */
                . = ALIGN(4);
                KEEP(*(.jcr*))

                /* All data end */
                . = ALIGN(16);
                __data_end__ = .;
        } > INIT_DATA_LOCATION

        .bss :
        {
                . = ALIGN(32);
                __bss_start__ = .;
                KEEP(*(.bss.ecc_buffer))
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                *(EXCLUDE_FILE(*\libg_nano.a:* *\libnosys.a:*) .bss*)
#else
                *(.bss*)
#endif
                *(COMMON)
                . = ALIGN(32);
                __bss_end__ = .;
        } > RAM

#if !defined(dg_configOPTIMAL_RETRAM) \
        || (defined(dg_configOPTIMAL_RETRAM) && (dg_configOPTIMAL_RETRAM == 0))
        /* System heap area */
        .heap (COPY):
        {
                __end__ = .;
                PROVIDE(end = .);
                *(.heap*)
                __HeapLimit = .;
        } > RAM
#endif

        /* .stack_dummy section doesn't contains any symbols. It is only used for linker to
         * calculate size of stack sections, and assign values to stack symbols later */
        .stack_dummy (COPY):
        {
                *(.stack*)
        } > RAM

        /* Set stack top to end of RAM, and stack limit move down by size of stack_dummy section */
        __StackTop = ORIGIN(RAM) + LENGTH(RAM);
        __StackLimit = __StackTop - SIZEOF(.stack_dummy);
        PROVIDE(__stack = __StackTop);

#if defined(dg_configOPTIMAL_RETRAM) && (dg_configOPTIMAL_RETRAM == 0)
        /* Check if data + heap + stack exceeds RAM limit */
        ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
#endif

#if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_B))
        #if defined(dg_configOPTIMAL_RETRAM) && (dg_configOPTIMAL_RETRAM == 1) \
                && defined(__HEAP_IS_LESS_THAN_0x200) && (__HEAP_IS_LESS_THAN_0x200 == 1)
        /* System heap area: the size is assumed to be less than 0x200; no need to be zero
         * initialized */
        RETENTION_HEAP 0x07FC0200 (COPY) :
        {
                __end__ = .;
                PROVIDE(end = .);
                *(.heap*)
                __HeapLimit = .;
        } > RetRAM0
        #endif
#endif

        /* Retention RAM - Code and Initialized data section
         *
         * The code and initialised data section is stored immediately at the end of the
         * (initialized) data section.
         *
         * Put any code that has to be retained during pm_mode_extended_sleep
         * in this area. Any initialized variables are also placed in this area.
         *
         * WARNING: THE IMAGE SIZE WILL BE INCREASED BY THE SIZE OF THIS AREA. Put ONLY the
         *          ABSOLUTELY NECESSARY IN HERE!
         */
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
        RETENTION_INIT0 : AT (__etext + (__data_end__ - __data_start__))
        {
                . = ALIGN(16);
                __RetRAM0_code_start__ = .;
                *(text_retained)
                *libgcc.a:_aeabi_uldivmod.o (.text*)
                *libgcc.a:_muldi3.o (.text*)
                *libgcc.a:_dvmd_tls.o (.text*)
                *libgcc.a:bpabi.o (.text*)
                *libgcc.a:_udivdi3.o (.text*)
                *libgcc.a:_clzdi2.o (.text*)
                *libgcc.a:_clzsi2.o (.text*)
                . = ALIGN(4);

                *(retention_mem_init)
                *(retention_mem_const)
                *(privileged_data_init)
                *(.retention)
                *\libg_nano.a:* (.data*)
                *\libnosys.a:* (.data*)
                . = ALIGN(16);
                __RetRAM0_code_end__ = .;
                . = ALIGN(4);
        } > RetRAM0
#endif

        /* Retention RAM 0 - Non-init and Zero-init data section */
        RETENTION_RAM0 (COPY) :
        {
                . = ALIGN(4);
                __RetRAM0_start = .;
                *(nmi_info)
                *(hard_fault_info)
                *(retention_mem_uninit)
                . = ALIGN(32);

                __RetRAM0_data_start = .;
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                *\libg_nano.a:* (.bss*)
                *\libnosys.a:* (.bss*)
#endif
                *(privileged_data_zi)
                *(retention_mem_zi)

                *(os_heap)

#if (defined(dg_configOPTIMAL_RETRAM) && (dg_configOPTIMAL_RETRAM == 1))
        #if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A) \
            ||                                                   \
            ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B) \
            && ((defined(__HEAP_IS_LESS_THAN_0x200) && (__HEAP_IS_LESS_THAN_0x200 == 0)))))
        #endif
                __end__ = .;
                PROVIDE(end = .);
                *(.heap*)
                __HeapLimit = .;
#endif
                . = ALIGN(32);
                __RetRAM0_data_end__ = .;
        } > RetRAM0

        /* Retention RAM 1 - Zero-init only section */
        RETENTION_RAM1 (COPY) :
        {
                . = ALIGN(32);
                __RetRAM1_start__ = .;
                *(privileged_data_1_zi)
                *(retention_mem_1_zi)
                . = ALIGN(32);
                __RetRAM1_end__ = .;
        } > RetRAM1

#if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_E))
#elif ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_B))
#else
        ASSERT(0, "Unsupported chip version")
#endif

        /* Set start and size of RetRAMs */
#if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_B))
        __RetRAM0_size = (__RetRAM0_data_end__ - __RetRAM0_data_start);
#else
        __RetRAM0_size = LENGTH(RetRAM0) - (__RetRAM0_data_start - ORIGIN(RetRAM0));
#endif

        /* Set the image size */
        __image_size = __etext + (__data_end__ - __data_start__)
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                           + (__RetRAM0_code_end__ - __RetRAM0_code_start__)
#endif
                           - ORIGIN(ROM);

        /* Set the mirrored image size.
         * The RETENTION_INIT0 area does not have to be copied again from the image. */
        __mirrored_image_size = __etext + (__data_end__ - __data_start__) - ORIGIN(ROM);
}
