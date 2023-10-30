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

To make our lives easier we will set:
- [sha256](https://www.raspberrypi.com/documentation/computers/config_txt.html#sha256)
- [uart\_2ndstage](https://www.raspberrypi.com/documentation/computers/config_txt.html#uart_2ndstage)
- [kernel\_address](https://www.raspberrypi.com/documentation/computers/config_txt.html#kernel_address)

To test these changes work, let's first set the `kernel` parameter to point to the existing Raspberry Pi kernel, connect our
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

Unfortunely our instructions are below the header in the binary...
but we can use [objcopy](https://releases.llvm.org/9.0.0/docs/CommandGuide/llvm-objcopy.html)
to extract just the binary part of the ELF file and produce something
the Raspberry Pi can boot.

```
llvm-objcopy -O binary kernel8.elf kernel8.img
```

Finally we have a bootable image! We can copy this to the `bootfs` partition
of our SD Card and boot into our bootloader!

With some luck we should see the bootloader boot the shellcode and the Raspberry
Pi will hang.


[^3] [The LLVM overview](https://llvm.org)

> TODO: Add instructions for setting up the llvm toolchain for ARM
> TODO: Docker container for cross compilation
> TODO: GitHub Environment?
> TODO: Example of debugging with QEMU as an aside

> TODO: Add shellcode for sleeping the CPU

## Serial communications

> TODO: Explain UARTs
> TODO: Explain UART controllers on RPi4

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
