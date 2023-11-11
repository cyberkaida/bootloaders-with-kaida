# Serial setup

There are a three config changes we need to make to
observe the boot process on the Raspberry Pi 4:

- Enable UART from the first stage bootloader
- Enable UART from the second stage bootloader
- Enable serial output from Linux
- Find the voltage of the UART
- Connect the serial adapter

## First stage bootloader

Download the updated first stage from the
[Rasperry Pi eeprom GitHub](https://github.com/raspberrypi/rpi-eeprom/blob/master/firmware-2711/latest/recovery.bin)[^3]

The configuration for the UART output is inside this binary,
we can patch it using a variety of tools like a hex editor or `sed`.
This is documented in the Raspberry Pi documentation[^1][^2][^3].

```sh
sed -i -e "s/BOOT_UART=0/BOOT_UART=1/" recovery.bin
```

Then copy the `recovery.bin` to the `bootfs` partition of your SDCard.

[^1]: [Patching the bootloader](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#bootcode-bin-uart-enable)
[^2]: [Flashing the Raspberry Pi 4](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#raspberry-pi-bootloader-configuration). Note that we are manually flashing instead of using the script.
[^3]: [The recovery.bin](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#recovery-bin)

## Second stage bootloader

The second stage bootloader is complex enough to read a configuration file.
We can add the following to the bottom of the `config.txt`[^4] file on the `bootfs` partition
of the SDCard.

```
BOOT_UART=1
uart_2ndstage=1
```

[^4]: [BOOT\_UART](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#BOOT_UART)
[^5]: [uart\_2ndstage](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#uart_2ndstage)

## Linux

To configure linux to output to serial we need to change the `cmdline.txt` file
on the `bootfs` partition of the SDCard.

## Finding the pins

> Look for the pin with the square!
