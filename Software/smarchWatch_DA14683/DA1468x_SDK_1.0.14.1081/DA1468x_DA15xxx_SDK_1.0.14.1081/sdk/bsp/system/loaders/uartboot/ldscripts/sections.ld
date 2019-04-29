/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions RAM, RetRAM0 and BUFFER.
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
 */
ENTRY(Reset_Handler)

SECTIONS
{
        .text :
        {
                KEEP(*(.isr_vector))
                /* make sure that IVT doesn't cross 0xC0 */
                . = 0xC0;

                KEEP(*(.patch_table))

                /* patchable parameters, at specific offset (+0x200) */
                . = 0x200;
                PROVIDE_HIDDEN (__patchable_params = .);
                /* UART Tx port @0x200 */
                LONG(0xffffffff)
                /* UART Tx pin  @0x204 */
                LONG(0xffffffff)
                /* UART Rx port @0x208 */
                LONG(0xffffffff)
                /* UART Rx pin  @0x20c */
                LONG(0xffffffff)
                /* baud rate    @0x210 */
                LONG(0xffffffff)
                . = ALIGN(4);

                KEEP(*(.swd_section))
                . = ALIGN(4);

                *(text_reset*)
                *(.text*)
                *(.text_retained)

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
        } > RAM

        .ARM.extab :
        {
                *(.ARM.extab* .gnu.linkonce.armextab.*)
        } > RAM

        __exidx_start = .;
        .ARM.exidx :
        {
                *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        } > RAM
        __exidx_end = .;

        /* To copy multiple ROM to RAM sections,
         * uncomment .copy.table section and,
         * define __STARTUP_COPY_MULTIPLE in startup_ARMCMx.S */
        .copy.table :
        {
                . = ALIGN(4);
                __copy_table_start__ = .;

                LONG (__etext)
                LONG (__data_start__)
                LONG (__data_end__ - __data_start__)
                LONG (LOADADDR(REBOOT_SECTION))
                LONG (ADDR(REBOOT_SECTION))
                LONG (__REBOOT_SECTION_end - __REBOOT_SECTION_start)

                __copy_table_end__ = .;
        } > RAM

        /* To clear multiple BSS sections,
         * uncomment .zero.table section and,
         * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_ARMCMx.S */
        .zero.table :
        {
                . = ALIGN(4);
                __zero_table_start__ = .;

                LONG (__zero_init_start)
                LONG (__zero_init_end - __zero_init_start)

                __zero_table_end__ = .;
                . = ALIGN(16);
        } > RAM

        /* 16 byte alignment is required. Please do not add anything between the .zero.table
        definition and the __etext assignment! */
        __etext = .;

        /* The initialised data section is stored immediately
        at the end of the text section */
        .data : AT (__etext)
        {
                . = ALIGN(16);
                __data_start__ = .;

                *(vtable)
                *(.data*)
                *(retention_mem_init)
                *(privileged_data_init)

                /* init_array/fini_array moved to flash, align preserved */
                . = ALIGN(4);
                KEEP(*(.jcr*))

                . = ALIGN(16);
                __data_end__ = .;
        } > RAM

        .bss :
        {
                *(nmi_info)
                *(hard_fault_info)
                *(retention_mem_uninit)

                . = ALIGN(32);
                __zero_init_start = .;
                *(privileged_data_zi)
                *(retention_mem_zi)
                . = ALIGN(32);
                __zero_init_end = .;

                __bss_start__ = .;

                *(.bss*)
                *(COMMON)

                *(os_heap)
                *(ble_exchange_table_area)
                *(os_heap)
                *(ble_msg_heap)
                *(ble_env_heap)
                *(ble_db_heap)

                . = ALIGN(16);
                __bss_end__ = .;
        } > RAM

        REBOOT_SECTION :
        {
                . = ALIGN(16);
                __REBOOT_SECTION_start = .;
                *(reboot_section)
                . = ALIGN(16);
                __REBOOT_SECTION_end = .;
        } >REBOOT AT >RAM

        .heap (COPY):
        {
                __end__ = .;
                PROVIDE(end = .);
                *(.heap*)
                __HeapLimit = .;
        } > RAM

        /* .stack_dummy section doesn't contains any symbols. It is only
         * used for linker to calculate size of stack sections, and assign
         * values to stack symbols later */
        .stack_dummy (COPY):
        {
                *(.stack*)
        } > RAM

        /* Set stack top to end of RAM, and stack limit move down by
         * size of stack_dummy section */
        __StackTop = ORIGIN(RAM) + LENGTH(RAM);
        __StackLimit = __StackTop - SIZEOF(.stack_dummy);
        PROVIDE(__stack = __StackTop);

        /* Check if data + heap + stack exceeds RAM limit */
        ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")

        .inputbuffer :
        {
                . = ALIGN(4);
                __inputbuffer_start = .;
                . = ORIGIN(BUFFER) + LENGTH(BUFFER);
                __inputbuffer_end = .;
        } > BUFFER
}
