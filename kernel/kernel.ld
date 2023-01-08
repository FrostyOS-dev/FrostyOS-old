OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
 
ENTRY(_start)
 
/* Define the program headers we want so the bootloader gives us the right */
/* MMU permissions */
PHDRS
{
    text    PT_LOAD    FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata  PT_LOAD    FLAGS((1 << 2)) ;            /* Read only */
    data    PT_LOAD    FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
}
 
SECTIONS
{
    /* We wanna be placed in the topmost 2GiB of the address space, for optimisations */
    /* and because that is what the Limine spec mandates. */
    /* Any address in this region will do, but often 0xffffffff80000000 is chosen as */
    /* that is the beginning of the region. */
    . = 0xffffffff80000000;
  
    .text : {
        __text_start = .;
        *(.text .text.*)
        __text_end = .;
    } :text
 
    /* Move to the next memory page for .rodata */
    . += 0x1000;
 
    .rodata : {
        __rodata_start = .;
        *(.rodata .rodata.*)
        __rodata_end = .;
    } :rodata
 
    /* Move to the next memory page for .data */
    . += 0x1000;
 
    .data : {
        __data_start = .;
        *(.data .data.*)
        __data_end = .;
    } :data
 
    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    } :data
 
    /* Discard notes since they may cause issues on some hosts. */
    /DISCARD/ : {
        *(.note .note.*)
    }
}