# 02 - Running code

Let's start by studying the existing bootloader!

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
toolchain. clang is a native cross compiler, unlike gcc. [^3]

The first task is to build a basic program to park the CPU. This
simple program will be an infinite loop and serves as a platform
to build our compiler settings for the next set of exercises.

Exercise [02-02-asm-stub](./exercises/02-02-asm-stub) contains
a base [Makefile](./exercises/02-02-asm-stub) that you can use
to get running quickly.

We will use clang to compile for our specific architecture.
By specifying a few arguments we can configure clang to output
a binary for the Raspberry Pi 4:

- [`-nostdlib`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-nostdlibinc)
- [`-mcpu=cortex-a72+nosimd`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-mcpu)
- [`-ffreestanding`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-ffreestanding)
- [`-nostdinc`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-nostdinc)
- [`-target aarch64-elf`](https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-target)

```Makefile
CLANGFLAGS = -Wall -nostdlib -mcpu=cortex-a72+nosimd -ffreestanding -nostdinc
TARGET = -target aarch64-elf
```

Once compiled, we can need to make sure all our references are
rebased correctly at link time. Unlike user mode programs, we
must have our first instruction at a specific address in memory.
The Raspberry Pi 4 ARM core expects its code to be at 0x80000
when running in 64 bit mode.

The LLVM toolchain provides a script language to tell the linker
where to place binary components in memory.
- https://lld.llvm.org 
- https://sourceware.org/binutils/docs/ld/Scripts.html
- https://sourceware.org/binutils/docs/ld/Simple-Example.html

A simple example is below. This script sets the base address
to 0x80000 and keeps any section starting with `.text`.

```
SECTIONS
{
    . = 0x80000;
    .text : { KEEP(*(.text)) *(.text .text.*) }
}
```

Once we've compiled our binary with these options, we will have an ELF
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
and these aren't valid instructions.

We can use [objcopy](https://releases.llvm.org/9.0.0/docs/CommandGuide/llvm-objcopy.html)
to extract just the binary part of the ELF file, removing the header and we can produce something
the Raspberry Pi can boot by placing the instructions directly at 0x80000.

```
llvm-objcopy -O binary kernel8.elf kernel8.img
```

Finally we have a bootable image! We can copy this to the `bootfs` partition
of our SD Card and boot into our bootloader!

With some luck we should see the bootloader boot the shellcode and the Raspberry
Pi will hang.


[^3] [The LLVM overview](https://llvm.org)

### ARM Shellcode

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
    and     x1, x1, #3
    cbz     x1, main_core
    // We're not on the main core, so hang in an infinite loop
park_loop:  wfe
    b       park_loop 
main_core:  // We're on the main core!
    // The rest of our shellcode can go here
```

## Serial communications

So far we have a machine that does nothing useful, with no
way to talk to the outside world. Wouldn't it be great to have some
output?

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
describes the `uart_2ndstage` option. When set to `1` the second stage bootloader will output
to the mini-UART.
The first stage bootloader does not parse this file, instead it must be patched.
[The bootcode.bin documentation](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#bootcode-bin-uart-enable)
describes how to enable UART for the first stage bootloader.

By observing existing output, we can confirm that our serial adapter is configured correctly.
To configure our serial adapter we need to connect the pins to the mini-UART output,
The following pins are used by the mini-UART:
- [Ground - pin 6](https://pinout.xyz/pinout/ground)
- [GPIO 14 - pin 8](https://pinout.xyz/pinout/pin8_gpio14/)
- [GPIO 15 - pin 10](https://pinout.xyz/pinout/pin10_gpio15/)

> Remember! You must connect the TX (transmit) pin on the RPi to the RX (receive) pin of your serial adapter!



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
[^2] Bugs in bootloaders stored in ROM have high impact. See:
     - [fail0verflow - Nintendo Switch Tegra X1 exploit](https://fail0verflow.com/blog/2018/shofel2/)
     - [Wired - checkm8 interview with axi0mX](https://arstechnica.com/information-technology/2019/09/developer-of-checkm8-explains-why-idevice-jailbreak-exploit-is-a-game-changer/) 
     - [GitHub - wind3x iPod exploit](https://github.com/freemyipod/wInd3x)
