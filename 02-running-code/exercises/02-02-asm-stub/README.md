# Exercise 2-2 ASM Stub

In this exercise we will boot a simple infinite loop!
Most of the build infrastructure is provided.

## Things to understand

- The [Makefile](./Makefile) is set up for you, you should read through it and ask any questions!

## Changes to make
- We need to add a loop to [entry.s](./entry.s), so we have some code to execute!
- We need to tell the linker where to put our shellcode by chanhing [link.ld](./link.ld)
