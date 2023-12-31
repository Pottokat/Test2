/* $Id$ */
/* rp2040 with QSPI memory layout */
/* Linker script to configure memory regions. */
MEMORY
{
	ROM (rx) :	ORIGIN = 0x10000000, LENGTH = 2048k	/* W25Q16JVUXIQ */
	RAM (rwx) :	ORIGIN = 0x20000000, LENGTH = 256k	/* 264k */
	SRAM5 (rwx): ORIGIN = 0x20041000, LENGTH = 4k
}

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions ROM and RAM.
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
 *   __Vectors_End
 *   __Vectors_Size
 */
ENTRY(Reset_Handler)

SECTIONS
{
	.boot :
	{
		KEEP(*(.page0*))
		. = ALIGN(256);
	} > SRAM5 AT > ROM
	
	.text :
	{
		KEEP(*(.vectors))
		__Vectors_End = .;
		__Vectors_Size = __Vectors_End - __Vectors;

		*(.text*)
		*(.initfunc*)

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

		*(.rodata*)
		*(.initfunc*)
		*(.initdata*)

		KEEP(*(.eh_frame*))
		. = ALIGN(4);
	} > ROM

	.ARM.extab :
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > ROM

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > ROM
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
		LONG ((__data_end__ - __data_start__) / 4)
		
		__copy_table_end__ = .;
	} > ROM


	/* To clear multiple BSS sections,
	 * uncomment .zero.table section and,
	 * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_ARMCMx.S */

	.zero.table :
	{
		. = ALIGN(4);
		__zero_table_start__ = .;
		LONG (__bss_start__)
		LONG ((__bss_end__ - __bss_start__) / 4)
		__zero_table_end__ = .;
	} > ROM
	
	__etext = .;

	.data :
	{
		__data_start__ = .;
		*(.data*)

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

		KEEP(*(.jcr*))
		. = ALIGN(4);
		*(vtable*)

		. = ALIGN(4);
		/* All data end */
		__data_end__ = .;

	} > RAM AT > ROM

    . = __etext + __data_end__ - __data_start__;
    
 	.noinit (NOLOAD):
	{
		. = ALIGN(32);
		*(.noinit*)
		. = ALIGN(32);
	} > RAM
	
	.bss (NOLOAD):
	{
		. = ALIGN(32);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(32);
		__bss_end__ = .;
	} > RAM

	.heap (COPY):
	{
		__HeapBase = .;
		__end__ = .;
		end = __end__;
		KEEP(*(.heap*))
		__HeapLimit = .;
	} > RAM

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (COPY):
	{
		KEEP(*(.stack*))
	} > RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

	/* Check if data + heap + stack exceeds RAM limit */
	/* ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack") */
}
