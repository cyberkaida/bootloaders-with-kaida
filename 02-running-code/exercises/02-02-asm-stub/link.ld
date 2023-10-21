SECTIONS
{
    . = /* TODO: fill in the address where the Raspberry Pi expects code to be loaded */;
    .text : { KEEP(*(.text)) *(.text .text.*) }
}
