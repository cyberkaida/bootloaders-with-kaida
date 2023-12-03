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

## Raspberry Pi Linux

To configure linux on the raspberry pi to output to serial we need to change the `cmdline.txt` file
on the `bootfs` partition of the SDCard[^5].

> Your commandline.txt will have some options specific to your install, like the partition UUID.
> It is a good idea to back this up before modifying!

We need to add the following to the kernel command line:

```
console=serial0,115200
```

This will add an additional console on the serial lines. We can use this to monitor output and
log in to the Raspberry Pi over our UART connection. Note there will be two `console` entries
in the command line once you add the one above. The other will be the `pty`[^6].

[^5]: [Linux cmdline.txt serial options.](https://www.raspberrypi.com/documentation/computers/configuration.html#command-line-options)
[^6]: [Linux pseudo-terminals](https://linux.die.net/man/7/pty)

## Finding the pins

There is an online diagram at [pinout.xyz](https://pinout.xyz). You can also find
the pinout diagram on the Raspberry Pi documentation[^7]

> Look for the pin with the square and match it to the diagram

We need to connect the RX on our UART USB serial adapter to the TX on the Raspberry Pi,
and the TX on our adapter to the RX on the Raspberry Pi. The ground is the same on them both.

[^7]: [Pinout on the Raspberry Pi documentation](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#gpio-and-the-40-pin-header)

## Connecting the `screen` tool to the UART adapter

Now we are ready to connect! 🎉

Some tools we can use:
- [screen](https://linux.die.net/man/1/screen)
- [pwntools](https://docs.pwntools.com/en/latest/tubes/serial.html)

Both of these tools connect to a USB tty device (our USB UART adapter).

I _always_ forget what the device is called, and it can change depending
on your adapter, so I always do something like this:

```sh
ls -la /dev | tee /tmp/dev-before.txt
# Plug in your UART adapter
ls -la /dev | tee /tmp/dev-after.txt
diff /tmp/dev-before.txt /tmp/dev-after.txt
```

It will usually be called something like `/dev/ttyUSB0`
or `/dev/ttyACM0`, but the `diff` above will help find it!

### Using `screen`

There are two important things to know with `screen`.
First, how to start it. Second, how to exit!

To start screen we supply the device to connect to
and the baud rate like this:

```sh
sudo screen /dev/ttyUSB0 115200
```

We set the baud rate (line speed) when we configured the
Raspberry Pi above. 115200 is very common.

The device we found using the diff trick above.

To exit screen we can press `control + a` to enter the
command mode for screen (by default it passes keys to the
serial adapter), then `k` for "kill" to end the session.

# Looking through the output

**Now we are ready to turn the device back on!**

With everything connected, let's plug the power cable in
and see if our output comes out!

You should see something scroll past really fast on your terminal.

If you don't we can do some troubleshooting below.

# Troubleshooting

## I see funny ? and random characters

This could be a few things.
- The wires are the wrong way.
    - Try swapping the TX and RX wires. Sometimes it's like a USB and you need to flip it 😅
- The baudrate is wrong. Check that your baudrate is set to 115200
    - In a real scenario you can try some common baudrates, or use a [logic analyser](https://support.saleae.com/tutorials/learning-portal/learning-resources/learn-asynchronous-serial).

## I get a permission error using `screen`

By default on Ubuntu the user isn't in the `dialup` group.
You can use `sudo` when you run screen, or you can add yourself to
the `dialup` group:

```sh
sudo usermod -a -G dialup ${USER}
```

Then log out and back in (or reboot to be sure).

## I can't see any output from the device at all

If you are working on the early exercise, try skipping to the
[Raspberry Pi Linux](#raspberry-pi-linux) section and test
again. Linux is the easiest to configure and this can help
debug between the UART hardware and the device configuration.

If it still doesn't work, try with someone else's device!

## Check the pins with a multimeter

You can check if the voltage changes between ground and the device's TX
pin during boot to see if the device is transmitting.

## Try connecting and pressing the enter key a bunch

The device might be idle and waiting for input. Try pressing random buttons to
see if the device does something. It works more often than you think!
