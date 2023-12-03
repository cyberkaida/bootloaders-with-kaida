# Exercise 2-2 ASM Stub

In this exercise we will boot a simple infinite loop!
Most of the build infrastructure is provided.

We are using this exercise to get our build environment
installed and configured and to make sure we can flash
to the Raspberry Pi.

## Things to understand

- [The lecture notes](../../README.md#setting-up-the-cross-compilation-environment-and-an-asm-stub)
- The [Makefile](./Makefile) is set up for you, you should read through it and ask any questions!

## Reading

### ARM Assembly
- [ARM Cheatsheet](https://comp.anu.edu.au/courses/comp2300/resources/03-ARM-cheat-sheet/) 

### Linker scripting

- https://lld.llvm.org
- https://sourceware.org/binutils/docs/ld/Scripts.html
- https://sourceware.org/binutils/docs/ld/Simple-Example.html

## Changes to make
- We need to add a loop to [entry.s](./entry.s), so we have some code to execute!
- We need to tell the linker where to put our shellcode by changing [link.ld](./link.ld)

# Additional learning
- [Intro to ARM on OpenSecurityTraining](https://opensecuritytraining.info/IntroARM.html)
- [ARM Assembler in Raspberry Pi](https://thinkingeek.com/arm-assembler-raspberry-pi/) - A guide to ARM assembly using the Raspberry Pi
- [ARM Assembler in Raspberry Pi - Registers](https://thinkingeek.com/2013/01/10/arm-assembler-raspberry-pi-chapter-2/)
- [ARM Assembler in Raspberry Pi - Control Structures](https://thinkingeek.com/2013/01/20/arm-assembler-raspberry-pi-chapter-6/)
