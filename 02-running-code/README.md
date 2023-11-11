# 02 - Running code

Let's start by studying the existing bootloader!
We can read through the [annotated boot log](../01-initial-research/annotated-boot-log.md)
for an overview of the boot process.

Some references that will be useful throughout this section:
- [DataSheet for the BCM2711](https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf)

## Serial communications

A common early stage output is via serial and a common protocol for
serial output is [UART](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter).
UART is a simple communication protocol and is widely implemented in
embedded hardware.

There are actually [many UARTs on the Raspberry Pi](https://elinux.org/RPi_Serial_Connection), connected to different
components. There is a simple UART designed for early boot and to act as
a console, the mini-UART. This is described in the BCM2711 datasheet, section 2.2.
This section sounds complex, but we can break this down into simple steps.

First let's observe existing output. We can do this by enabling the mini-UART for the
first and second stage bootloaders. The [documentation for the `config.txt` file](https://www.raspberrypi.com/documentation/computers/legacy_config_txt.html#uart_2ndstage)
(on the `bootfs` partition of the SDCard)
describes the `uart_2ndstage` option. When set to `1` the second stage bootloader will output
to the mini-UART.

The first stage bootloader is too simple to parse this file, instead it must be patched.
[The bootcode.bin documentation](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#bootcode-bin-uart-enable)
describes how to enable UART for the first stage bootloader.

By observing existing output, we can confirm that our serial adapter is configured correctly.
To configure our serial adapter we need to connect the pins to the mini-UART output,
The following pins are used by the mini-UART:
- [Ground - pin 6](https://pinout.xyz/pinout/ground)
- [GPIO 14 - pin 8](https://pinout.xyz/pinout/pin8_gpio14/)
- [GPIO 15 - pin 10](https://pinout.xyz/pinout/pin10_gpio15/)

> Remember! You must connect the TX (transmit) pin on the RPi to the RX (receive) pin of your serial adapter!

## The first stage bootloader

- [ARM Cortex-A72](https://en.wikipedia.org/wiki/ARM_Cortex-A72)
- [DataSheet for the BCM2711](https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf)
- [pico-pi bootrom source code](https://github.com/raspberrypi/pico-bootrom)

The first stage bootloader on the Raspberry Pi 4
is stored in Read Only Memory (ROM). This is common
for many device designs, known as a [BootROM](https://en.wikipedia.org/wiki/Boot_ROM).
A known good, working, secure and small bootloader is
flashed onto the device in the
factory to provide a [root of trust](https://en.wikipedia.org/wiki/Trust_anchor)
for security, and a minimal flashing capability for reliability and
recovery.

The Raspberry Pi 4 follows this common design.

From the [documentation](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#first-stage-bootloader)...

Our [Broadcom BCM2711 System on a Chip](https://github.com/raspberrypi/documentation/blob/develop/documentation/asciidoc/computers/processors/bcm2711.adoc)
powers up and initialises. It loads the first stage bootloader from
ROM.

The first stage bootloader is immutable[^1], and for this reason
is typically simple and tries to verify, load and run a second stage
as quickly as possible. Any bugs found after shipping devices are
effectively permanent, and as this component is the root of trust,
can lead to catastrophic jailbreaks and impact[^2].


# Second Stage Bootloader

- [Documentation](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#raspberry-pi-4-boot-eeprom)
- [Current Stable EEPROM](https://github.com/raspberrypi/rpi-eeprom/blob/master/firmware/stable/recovery.bin)
- [Example EEPROM code](https://github.com/raspberrypi/usbboot/)

This is a proprietary instruction set ELF file. We can configure it to boot a custom third stage bootloader instead of a Linux
kernel by changing the `config.txt` on the SD card. We will [set the `kernel` parameter](https://www.raspberrypi.com/documentation/computers/config_txt.html#kernel)
to point to a new file on our SD card that will contain the assembled shellcode for our bootloader.

Later, we will investigate the following config.txt settings:
- [sha256](https://www.raspberrypi.com/documentation/computers/config_txt.html#sha256)
- [uart\_2ndstage](https://www.raspberrypi.com/documentation/computers/config_txt.html#uart_2ndstage)
- [kernel\_address](https://www.raspberrypi.com/documentation/computers/config_txt.html#kernel_address)

To test these changes work, let's first set the `kernel` parameter to point to the existing Raspberry Pi kernel (`kernel8.img`), connect our
serial adapter and confirm the device boots and outputs logs correctly.

## Setting up the cross compilation environment and an ASM stub

For our exercises we will be using the LLVM clang cross compilation
toolchain. clang is a native cross compiler, unlike gcc.[^3] Cross
compilation in gcc can be a long and difficult process if you start
from scratch.

The first task is to build a basic program to park the CPU. This
simple program will be an infinite loop and serves as a platform
to build our compiler settings for the next set of exercises.

Exercise [02-02-asm-stub](./exercises/02-02-asm-stub) contains
a base [Makefile](./exercises/02-02-asm-stub) that you can use
to get running quickly. You can look at the task before reading
the rest of this section, but you should finish reading this
section before starting!

We will use clang to compile for our specific architecture.
By specifying a few arguments we can configure clang to output
a binary for the Raspberry Pi 4:

- [`-nostdlib`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-nostdlibinc) to remove any links to the C standard library. These depend on a kernel, and we don't have one yet!
- [`-mcpu=cortex-a72+nosimd`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-mcpu) to output code for our specific core. We could also define just aarch64 but being specific helps the optimiser.
- [`-ffreestanding`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-ffreestanding) to remove all links and assumptions about loaders, etc.
- [`-nostdinc`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-nostdinc) to error on any imports from the standard include paths, these depend on the C standard library, which we don't have!
- [`-target aarch64-elf`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-target) to output an ELF file for aarch64. This will make it easier to analyse if we have bugs. We will extract the code using `objcopy` for flashing later.

```Makefile
CLANGFLAGS = -Wall -nostdlib -mcpu=cortex-a72+nosimd -ffreestanding -nostdinc
TARGET = -target aarch64-elf
```

Once compiled, we can need to make sure all our references are
rebased correctly at link time. Unlike user mode programs, we
must have our first instruction at a specific address in memory.
The Raspberry Pi 4 ARM core expects its code to be at 0x80000
when running in 64 bit mode, this is defined not in the ARM
specification, but by the manufacturer.

Constants like this 0x80000 are common when building bootloaders,
like with software, hardware also has an API or a contract.

> Question! How can we find the expected load address?

This 0x80000 address can be found in a few ways:
- [Observing output from the previous boot stage over serial](../01-initial-research/annotated-boot-log.md) (search for `Loading 'kernel8.img' to 0x80000 size`)
- [Looking at an existing implementation like Linux](https://docs.kernel.org/5.19/arm64/booting.html) (search for 0x80000)
- Asking the hardware team!

The LLVM toolchain provides a script language to tell the linker
where to place binary components in memory.
A simple example is below. This script sets the base address
to 0x80000 and keeps any section starting with `.text`.

```
SECTIONS
{
    . = 0x80000;
    .text : { KEEP(*(.text)) *(.text .text.*) }
}
```

Some useful references for linker scripting:
- https://lld.llvm.org 
- https://sourceware.org/binutils/docs/ld/Scripts.html
- https://sourceware.org/binutils/docs/ld/Simple-Example.html

Once we have compiled our binary with these options, we will have an ELF
file. If we inspect this file we'll see it starts with an ELF header:

First we can output the first few bytes as hex using [`xxd`](https://linux.die.net/man/1/xxd)
```
xxd kernel8.elf | head -n 5
```
At the start of the file here, we see the [ELF magic](https://linux.die.net/man/5/elf) (`EI_MAG0`, etc)
```
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000010: 0200 b700 0100 0000 0000 0800 0000 0000  ................
00000020: 4000 0000 0000 0000 a000 0100 0000 0000  @...............
00000030: 0000 0000 4000 3800 0200 4000 0600 0400  ....@.8...@.....
00000040: 0100 0000 0500 0000 0000 0100 0000 0000  ................
```

Unfortunely our instructions are below the header in the binary... `\x7fELF` will be at 0x80000,
and these bytes aren't valid instructions.

We can use [objcopy](https://releases.llvm.org/9.0.0/docs/CommandGuide/llvm-objcopy.html)
to extract just the binary part of the ELF file, removing the header to produce something
the Raspberry Pi can boot by placing the instructions directly at 0x80000.

```sh
llvm-objcopy -O binary kernel8.elf kernel8.img
```

Finally we have a bootable image! We can copy `kernel8.img` to the `bootfs` partition
of our SD Card, insert it into the Raspberry Pi and boot into our bootloader!

With some luck we should see the bootloader boot the shellcode and the Raspberry
Pi will hang ✨👩‍💻🎉🎉

[^3]: [The LLVM overview](https://llvm.org)

## Bootstrapping to C

### Single threading

At this stage of the bootstrap, we don't have many of the
conveniences that we have in user land or even a kernel, so we need to do
a lot ourselves.

For example:
- We run all code on all cores
- We don't have an allocator, or page tables
- We don't have a calling convention, so no functions

This sounds very scary, but we can build this ourselves!

The first step is to make sure we are single threaded, we don't
want to deal with multithreading or multiprocessing complexity this early.

We can do this on ARM using a special register and the [`mrs` instruction](https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/coprocessor-instructions/mrs)
the [Multiprocessor Affinity Register](https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/MPIDR-EL1--Multiprocessor-Affinity-Register) (in the [ID functional group](https://developer.arm.com/documentation/ddi0595/2021-12/Registers-by-Functional-Group?lang=en#ID)) contains the current core ID. We can use this to write some
shellcode that executions only on a particular core.

The logic is:
- Check the core ID
- If the core is the first core, jump to main code
- For any other value, infinite loop (called a "parking" a core)

In this way we can disregard the complexity of multiprocessing until
we have more abstraction layers in place (usually the kernel will enable
multiprocessing).

In practice we can do something like the following:

```aarch64
_start:
    // Check processor ID is zero (executing on main core)
    // else hang.
    mrs     x1, mpidr_el1
    // Bitwise AND with 0b111.
    and     x1, x1, #3
    // If the result of the AND above was 0b000
    // jump to `main_core`
    cbz     x1, main_core
    // We're not on the main core, so hang in an infinite loop
park_loop:  
    wfe     // Put the core to sleep until an interrupt arrives
    b       park_loop  // If we wake, go back to sleep.
main_core:  // We're on the main core!
    // The rest of our shellcode can go here
```

So far we have a machine that does nothing useful, with no
way to talk to the outside world. Wouldn't it be great to have some
output? Next we will bootstrap up to C and implement serial output!

### Setup for C

Until now we have been programming in raw assembly. This is fine for simple programs,
but it would be great to have a high level language like C! Our UART communication will
require more complexity, and using C will be much easier.

We could also call into another systems language like Rust, but right now (2023) most bootloaders
at least start in C. If you are writing a bootloader from scratch, try Rust! The process
is very similar to C.

To get from assembly to C, we must implement a few things:
- A [calling convention](https://en.wikipedia.org/wiki/Calling_convention#ARM_(A64))
- A [stack for local variables](https://en.wikipedia.org/wiki/Call_stack)
- [Data segments](https://en.wikipedia.org/wiki/Data_segment)
    - The `.bss` and `.rodata` sections for data such as strings
    - Optionally, the `.data` section for global variables

We will need to make changes in our linker script for the new sections, and
in out shellcode for the calling convention.

For the standard C [calling convention](https://en.wikipedia.org/wiki/Calling_convention#ARM_(A64))
we must implement [the stack](https://en.wikipedia.org/wiki/Call_stack)
and place the bottom of the stack in the [`sp` register](https://developer.arm.com/documentation/dui0801/a/Overview-of-AArch64-state/Stack-Pointer-register).

On ARM (and most platforms) the stack grows _down_. This means we place the highest address in the `sp` register, and each call will _subtract_ from
this value to grow the stack.

We should pick a location in memory for our stack that will not contain code or data that our bootloader uses. We
don't want C to override any of our code or data with stack variables.

For our purposes we can place the base of our stack at `__start` (defined in the shellcode and placed by the linker at 0x80000).
When we call the `bootloader_main` function with
a [branch and link instruction (`bl`)](https://developer.arm.com/documentation/dui0802/b/A32-and-T32-Instructions/B--BL--BX--and-BLX)
we will store the current address in the link register and jump to the start of `bootloader_main`.
The `bootloader_main` function will be compiled by llvm with the C calling convention (unless we override it), and the first
few instructions will subtract from the stack pointer (`sp` register) enough space for local variables. By placing our stack
at `__start` we will have 0x80000 bytes available on the stack total. Using more than this will result in a stack overflow and
_bad things_ will happen.

Implementing the stack allows us to call functions and allocate stack variables in C, but global variables and read only data will
not be available.

We _could_ only use stack variables like so:

```c
void bootloader_main(void) {
    // This will create a stack variable and populate it with
    // individual mov instructions...
    char string[] = {'h', 'i', '\n', '\0'};

    while(1) {
        // We have no where to return to...
    }
}
```

but it would be much easier to do something more natural like:

```c
void bootloader_main(void) {
    // This will create a variable in the .rodata section
    // and set `string` with a `ldr` instruction. Much easier!
    char *string = "hi\n";

    while(1) {
        // We have no where to return to...
    }
}
```

For constants we need to implement two things, the [`.rodata` section](https://en.wikipedia.org/wiki/Data_segment#Data)
and the [`.bss` section](https://en.wikipedia.org/wiki/Data_segment#BSS).

First let's add the `.data` and `.rodata` sections in our linker script:

```
    /* Place .rodata in the binary. */
    /* This is where read only data (like our string) will be placed */
    .rodata : { *(.rodata .rodata.*) }
    /* Place a symbol called _data at the current location */
    PROVIDE(_data = .);
    /* Place .data in the binary */
    /* This is where global variables will be placed */
    .data : { *(.data .data.*) }
```

This part of the linker script will tell the linker to place the
`.rodata` and `.data` sections in the output binary directly at compile time.
This is fine as they are accounted for in the executable image
(the bytes are actually in the file).

For example, the string from our C would be located somewhere in the `.rodata` section
(as it is read only).

Unfortunately the `.bss` section is allocated by the loader. The
file on disk does not contain bytes for the `.bss` section. As
we are the loader, we will have to allocate this ourselves!

In our linker script we will define some constants in the binary
like so to indicate the start, end and size of the `.bss` section.

Here we define the `.bss` section as [NOLOAD](https://sourceware.org/binutils/docs/ld/Output-Section-Type.html).
We then set [the alignment](https://sourceware.org/binutils/docs/ld/Builtin-Functions.html#index-ALIGN_0028align_0029)
for the start and end addresses to 16 byte alignment and set a symbol (`__bss_start`) to the address of the beginning of
the section.

The linker will place all the uninitialised variables that would go into the `.bss` section here,
then set another symbol (`__bss_end`) to the end of the
section. Note as this is `NOLOAD` this space is not emitted into the binary, only the symbols we define are inserted.
Finally we emit a symbol for the size of the `.bss` section, so we know how long to loop for in our shellcode.

```
    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .; /* emit a symbol for the current address */
        *(.bss .bss.*)   /* Place the uninitialised variables */
        *(COMMON)
        __bss_end = .;   /* emit a symbol for the current address */
    }

/* Finally emit a symbol for the size of the .bss */
/* We're doing a divide by 8 here, because in the loader we will allocate 8 bytes at a time. */
__bss_size = (__bss_end - __bss_start)>>3;
```

Now that the binary has the correct symbols, we can allocate this space at runtime in the bootloader. By doing this we reduce the
EEPROM and cache space requirements of our bootloader, the entire bootloader binary must be loaded into cache to run.

We add the following code to our bootloader to allocate this space. There are a few ways to implement this,
an example is below:

```aarch64
    // Clean the BSS section
    ldr     x1, =__bss_start // Start address. From the linker script.
    ldr     w2, =__bss_size  // Size of the section. From the linker script.
bss_loop:
    cbz     w2, bootloader_loop_exit  // Quit loop if zero
    str     xzr, [x1], #8 // Load 8 NULL bytes from the zero register
    sub     w2, w2, #1 // subtract from our counter
    cbnz    w2, bss_loop // Loop if non-zero
bss_loop_exit:
```

Now we are ready to call into C! We have a stack and memory allocated for
constants (`.rodata`), global variables (`.data`), and the `.bss` section!

> TODO: Simple print from UART example

> TODO: Exercise: Read number and print ascii art for 0xCC logo

## Accessing EEPROM from bootloader

> TODO: Explain memory view, physical vs virtual memory
> TODO: Description of memory layout from datasheet

> TODO: Exercise: Blink pattern from EEPROM

> TODO: Load code from EEPROM and jump to it!

> TODO: simple hash, refuse to boot wrong hash.


## Implement simple boot menu

> TODO: Set Linux command line
> TODO: Boot Linux

# Video Core Firmware

https://github.com/hermanhermitage/videocoreiv

- [Documentation](https://www.raspberrypi.com/documentation/computers/configuration.html#start-elf-start_x-elf-start_db-elf-start_cd-elf-start4-elf-start4x-elf-start4db-elf-start4cd-elf)
- [Configuration](https://www.raspberrypi.com/documentation/computers/config_txt.html#boot-options)
- [Current stable for Raspberry Pi 4](https://github.com/raspberrypi/firmware/blob/master/boot/start4.elf)
- [Current stable for Raspberry Pi 400](https://github.com/raspberrypi/firmware/blob/master/boot/start4x.elf)

# Linux Kernel
- [Current Linux Kernel](https://github.com/raspberrypi/firmware/blob/master/boot/kernel8.img)

[^1]: There are many attacks against these "immutable" bootloaders
      including power glitching (a type of [side channel attack](https://en.wikipedia.org/wiki/Side-channel_attack))
      using tools like the [ChipWhisperer](https://wiki.newae.com/Main_Page)
[^2]: Bugs in bootloaders stored in ROM have high impact. See:
     - [fail0verflow - Nintendo Switch Tegra X1 exploit](https://fail0verflow.com/blog/2018/shofel2/)
     - [Wired - checkm8 interview with axi0mX](https://arstechnica.com/information-technology/2019/09/developer-of-checkm8-explains-why-idevice-jailbreak-exploit-is-a-game-changer/) 
     - [GitHub - wind3x iPod exploit](https://github.com/freemyipod/wInd3x)

# Extra references
- https://nostarch.com/art-arm-assembly
- https://nostarch.com/bare-metal-c
