# Assembling simple programs with llvm

There are two main tools for assembling programs with clang:

- `llc` - Compiles LLVM intermediate representation into `.s` files
- `llvm-mc` - Actually assembles `.s` files into object files

If we want to, we can write our programs in the LLVM
[intermediate representation](https://llvm.org/docs/LangRef.html)
and then have the toolchain generate the assembly for our
target architecture. This gives us flexibility, but typically
we still need some hand rolled assembly to write the bootloader
entrypoint.

We will focus on directly writing assembly as this is the less
complex method and we are building only for a single architecture.

## Linking

- https://lld.llvm.org/ELF/linker_script.html
- https://sourceware.org/binutils/docs/ld/Scripts.html
- https://sourceware.org/binutils/docs/ld/Simple-Example.html
