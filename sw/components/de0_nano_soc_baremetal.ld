OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm",
              "elf32-littlearm")

OUTPUT_ARCH(arm)
ENTRY(_socfpga_main)

MEMORY
{
    boot_rom (rx)  : ORIGIN = 0xfffd0000, LENGTH = 64K
    oc_ram   (rwx) : ORIGIN = 0xffff0000, LENGTH = 64K
    ram      (rwx) : ORIGIN = 0x00100000, LENGTH = 1023M
}

SECTIONS
{
    /*
     * Vector table
     */

    . = ORIGIN(ram);

    .vector ALIGN(4) :
    {
        __vector_start = .;
        KEEP(*(.vectors))
        KEEP(*(.vectors.*))
        __vector_end = .;
    } > ram


    /*
     * Code
     */

    .text ALIGN(4) :
    {
        __text_start = .;

        *(.text)
        *(.text.*)
        *(.stub)
        *(.gnu.linkonce.t.*)

        *(.glue_7t)
        *(.glue_7)
        *(.vfp11_veneer)

        __text_end = .;
    } > ram


    /*
     * ARM exception/unwind information
     */

    .ARM.extab ALIGN(4) :
    {
        *(.ARM.extab*)
        *(.gnu.linkonce.armextab.*)
    } > ram

    .ARM.exidx ALIGN(4) :
    {
        __exidx_start = .;

        KEEP(*(SORT(.ARM.exidx*)))
        KEEP(*(SORT(.gnu.linkonce.armexidx.*)))

        __exidx_end = .;
    } > ram


    /*
     * EH information
     */

    .eh_frame_hdr ALIGN(4) :
    {
        KEEP(*(.eh_frame_hdr))
    } > ram

    .eh_frame ALIGN(4) :
    {
        KEEP(*(.eh_frame))
        KEEP(*(.eh_frame.*))
    } > ram


    /*
     * Read-only data
     */

    .rodata ALIGN(4) :
    {
        __rodata_start = .;

        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)

        __rodata_end = .;
    } > ram


    /*
     * Exception tables
     */

    .gcc_except_table ALIGN(4) :
    {
        *(.gcc_except_table)
        *(.gcc_except_table.*)
    } > ram


    /*
     * C/C++ initialization
     */

    .init ALIGN(4) :
    {
        KEEP(*(.init))
    } > ram

    .fini ALIGN(4) :
    {
        KEEP(*(.fini))
    } > ram

    .preinit_array ALIGN(4) :
    {
        __preinit_array_start = .;
        KEEP(*(.preinit_array))
        __preinit_array_end = .;
    } > ram

    .init_array ALIGN(4) :
    {
        __init_array_start = .;

        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))

        __init_array_end = .;
    } > ram

    .fini_array ALIGN(4) :
    {
        __fini_array_start = .;

        KEEP(*(.fini_array))
        KEEP(*(SORT(.fini_array.*)))

        __fini_array_end = .;
    } > ram


    /*
     * Legacy C++ constructors
     */

    .ctors ALIGN(4) :
    {
        KEEP(*crtbegin.o(.ctors))
        KEEP(*(EXCLUDE_FILE(*crtend.o) .ctors))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*(.ctors))
    } > ram

    .dtors ALIGN(4) :
    {
        KEEP(*crtbegin.o(.dtors))
        KEEP(*(EXCLUDE_FILE(*crtend.o) .dtors))
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
    } > ram


    /*
     * Initialized data
     */

    .data ALIGN(8) :
    {
        __data_start = .;

        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d.*)

        *(.sdata)
        *(.sdata.*)
        *(.gnu.linkonce.s.*)

        *(.got.plt)
        *(.got)

        __data_end = .;
    } > ram

    _edata = .;
    PROVIDE(edata = .);


    /*
     * Uninitialized data
     */

    .bss ALIGN(8) :
    {
        __bss_start = .;
        __bss_start__ = .;

        *(.sbss)
        *(.sbss.*)
        *(.scommon)

        *(.bss)
        *(.bss.*)

        *(COMMON)

        . = ALIGN(8);

        __bss_end = .;
        __bss_end__ = .;
    } > ram

    _end = .;
    __end = .;
    __end__ = .;

    PROVIDE(end = .);


    /*
     * Stack at the end of RAM
     */

    _stack = ORIGIN(ram) + LENGTH(ram);


    /*
     * Debug information
     */

    .debug_info 0 :
    {
        *(.debug_info)
        *(.gnu.linkonce.wi.*)
    }

    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_line   0 : { *(.debug_line) }
    .debug_frame  0 : { *(.debug_frame) }
    .debug_str    0 : { *(.debug_str) }
    .debug_loc    0 : { *(.debug_loc) }
    .debug_ranges 0 : { *(.debug_ranges) }

    .stab 0 (NOLOAD) : { *(.stab) }
    .stabstr 0 (NOLOAD) : { *(.stabstr) }

    .comment 0 : { *(.comment) }

    .ARM.attributes 0 :
    {
        KEEP(*(.ARM.attributes))
    }

    /DISCARD/ :
    {
        *(.note.GNU-stack)
    }
}
