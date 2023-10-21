// We'll place our code in a special section
// and our linker script will place this in the
// right spot
.section .text.boot
.global _start
.type _start, @function
_start:
    // TODO: Your assembly code goes here
    // You'll want to create an infinite loop here
    // since we don't have anywhere to return to!
