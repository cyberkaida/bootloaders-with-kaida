.section .text.boot
.global _start
.type _start, @function
_start:
    // Loop forever
    b _start
