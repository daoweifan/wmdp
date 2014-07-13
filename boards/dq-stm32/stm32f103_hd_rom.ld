/* Program Entry, set to mark it as "used" and avoid gc */
MEMORY
{
    RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 48K  /* 48K sram */
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 256K /* 256KB flash */
}
ENTRY(Reset_Handler)

/* STM specific definition */
__stack = ORIGIN(RAM) + LENGTH(RAM);
_estack = __stack;
_system_stack_size = 0x1000;

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        _stext = .;
        KEEP(*(.isr_vector))            /* Startup code */
        . = ALIGN(4);
        *(.text)                        /* remaining code */
        *(.text.*)                      /* remaining code */
        *(.rodata)                      /* read-only data (constants) */
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.gnu.linkonce.t*)

        /* section information for special section */
        . = ALIGN(4);
        __shell_cmd_start = .;
        KEEP(*(shell_cmd))
        __shell_cmd_end = .;
        
        . = ALIGN(4);
        __FSymTab_start = .;
        KEEP(*(FSymTab))
        __FSymTab_end = .;
        
        . = ALIGN(4);
        __VSymTab_start = .;
        KEEP(*(VSymTab))
        __VSymTab_end = .;
        
        . = ALIGN(4);
        __board_init_start = .;
        KEEP(*(board_init))
        __board_init_end = .;
        
        . = ALIGN(4);
        __component_init_start = .;
        KEEP(*(component_init))
        __component_init_end = .;
        
        . = ALIGN(4);
        __task_init_start = .;
        KEEP(*(task_init))
        __task_init_end = .;
        
        . = ALIGN(4);
        __os_bg_start = .;
        KEEP(*(os_bg))
        __os_bg_end = .;
        
        . = ALIGN(4);
        __os_bg_1ms_start = .;
        KEEP(*(os_bg_1ms))
        __os_bg_1ms_end = .;
        
        . = ALIGN(4);
        __os_bg_4ms_start = .;
        KEEP(*(os_bg_4ms))
        __os_bg_4ms_end = .;
        
        . = ALIGN(4);
        __os_bg_8ms_start = .;
        KEEP(*(os_bg_8ms))
        __os_bg_8ms_end = .;
        
        . = ALIGN(4);
        __os_bg_16ms_start = .;
        KEEP(*(os_bg_16ms))
        __os_bg_16ms_end = .;
        
        . = ALIGN(4);
        __os_fg_1ms_start = .;
        KEEP(*(os_fg_1ms))
        __os_fg_1ms_end = .;
        
        . = ALIGN(4);
        __os_fg_4ms_start = .;
        KEEP(*(os_fg_4ms))
        __os_fg_4ms_end = .;
        
        . = ALIGN(4);
        __os_fg_8ms_start = .;
        KEEP(*(os_fg_8ms))
        __os_fg_8ms_end = .;
        
        . = ALIGN(4);
        __os_fg_16ms_start = .;
        KEEP(*(os_fg_16ms))
        __os_fg_16ms_end = .;
        
        . = ALIGN(4);
        libc.a

        /* section information for initial. */
        . = ALIGN(4);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;
        . = ALIGN(4);

        . = ALIGN(4);
        _etext = .;
    } > FLASH = 0

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)

        /* This is used by the startup in order to initialize the .data secion */
        _sidata = .;
    } > FLASH
    __exidx_end = .;

    /* .data section which is used for initialized data */

    .data : AT (_sidata)
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;

        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _edata = . ;
    } >RAM

    __bss_start = .;
    .bss(NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start__ = .;      /* standard newlib definition */
        _sbss = .;              /* STM specific definition */

        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        __bss_end__ = .;        /* standard newlib definition */
        _ebss = . ;             /* STM specific definition */
        *(.bss.init)
    } > RAM
    __bss_end = .;

    .noinit (NOLOAD) :
    {
        . = ALIGN(4);
        _noinit = .;
        
        *(.noinit .noinit.*) 
        
         . = ALIGN(4) ;
        _end_noinit = .;   
    } > RAM

    /* Mandatory to be word aligned, _sbrk assumes this */
    PROVIDE ( end = _end_noinit ); /* was _ebss */
    PROVIDE ( _end = _end_noinit );
    PROVIDE ( __end = _end_noinit );
    PROVIDE ( __end__ = _end_noinit );

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
     * Symbols in the DWARF debugging sections are relative to the beginning
     * of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}

/*
 * Default heap definitions.
 * The heap start immediately after the last statically allocated 
 * .sbss/.noinit section, and extends up to the main stack limit.
 */
PROVIDE ( _Heap_Begin = __end ) ;
PROVIDE ( _Heap_Limit = __stack - _system_stack_size ) ;
