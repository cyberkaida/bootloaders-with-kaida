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

This is a proprietary instruction set ELF file.

> Idea! Let's boot from this to a new third stage bootloader... this is a proprietary instruction set...
> Idea! Let's blink the LED: https://www.raspberrypi.com/documentation/computers/configuration.html#led-warning-flash-codes
> Idea! Let's display a logo: https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#boot-diagnostics-on-the-raspberry-pi-4

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
