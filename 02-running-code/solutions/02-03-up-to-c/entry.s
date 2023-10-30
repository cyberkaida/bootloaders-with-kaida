.section ".text.boot"  // Make sure the linker puts this at the start of the kernel image

.global _start  // Execution starts here

_start:
    // Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, main_core
    // We're not on the main core, so hang in an infinite wait loop
park_loop:  wfe
    b       park_loop 
main_core:  // We're on the main core!

    // Set stack to start below our code
    ldr     x1, =_start
    mov     sp, x1

    // Clean the BSS section
    ldr     x1, =__bss_start     // Start address
    ldr     w2, =__bss_size      // Size of the section
bss_loop:  cbz     w2, bootloader_main_call  // Quit loop if zero
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, bss_loop               // Loop if non-zero

    // Jump to our main() routine in C (make sure it doesn't return)
bootloader_main_call:  bl      bootloader_main

    // In case it does return, halt the master core too
    b       park_loop
