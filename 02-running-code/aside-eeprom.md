# The Raspberry Pi 4 EEPROM

The Raspberry Pi 4 differs from previous models during boot.
The first stage bootloader loads the second stage from an
EEPROM chip on the board, this allows the RPi4 to run without
an SDCard and boot from the network, among other things.

The first stage bootloader can read a basic FAT32 file system from the
SDCard and flash the EEPROM to perform a recovery if there
is an issue with the EEPROM data.

The EEPROM is also used to store a few components used by
the first and second stage bootloaders, like some configuration
for the bootloader.

The EEPROM data can be downloaded from the
[Raspberry Pi GitHub](https://github.com/raspberrypi/rpi-eeprom/)

Unfortunately the strucure of the EEPROM data is largely
undocumented. With some effort the format was reverse
engineered and a simple [imHex pattern](raspberrypi4-eeprom.imhex)
was built.

The format is as follows. Note all values are *big endian*.

```
4 byte magic
4 byte big endian `size`
`size` bytes of data
```

These structures are placed back to back in the EEPROM
and are not necessarily aligned.

The known magic values are:
| Bytes      | Description      |
| ---------- | ---------------- |
| `55aaf00f` | Unknown          |
| `55aafeef` | Padding          | 
| `55aaf11f` | Regular file     |
| `55aaf33f` | Copmpressed file |
| `ffffffff` | End of config    |

## `55aaf00f`

This section consists of:
- 4 byte `magic`
- 4 byte `size`
- `size` bytes of unknown data

The EEPROM starts with a section of this type.

## `55aafeef` - Padding

Padding sections consist of:
- 4 byte `magic` - `55aafeef`
- 4 byte size
- `size` bytes of `0xff` bytes

## `55aaf11f` - Regular File

A regular file type is used for small files and the
bootloader config. This lets the user and the configuration
tooling find and replace bytes to configure the bootloader.

- 4 byte `magic` - `55aaf11f`
- 4 byte `size`
- `size` bytes of data

The data block itself consists of another structure
- 12 bytes of `file name`, padded with `0x0` if less than 12 bytes
- `size` - 12 bytes of file data, uncompressed

## `55aaf33f` - Compressed file

A compressed file is the same as a regular file,
but the file data is compressed using a (potentially)
custom compression. Likely based on lzjb.

This algorithm seems to have been selected for ease of
implementation rather than compression ratio. The entropy
of compressed files was only ~5 rather than the expected ~7.

The algorithm used to compress and decompress these files
is hosted at https://git.venev.name/hristo/rpi-eeprom-compress.git

This was found on a [GitHub issue](https://github.com/raspberrypi/rpi-eeprom/issues/153)

# Known EEPROM sections
The following sections have been observed in
`pieeprom-2023-1-11.bin`.

- 60kb of unknown data
- `bootmain`
  - Second stage bootloader
  - ELF file
  - VideoCore Instruction Set (proprietary)
- `msys00.bin` through to `msys08.bin`
- `mcb.bin`
- `vl805hub.bin`
- `vl805mcu.bin`
- `logo.bin`
   - Raspberry Pi logo
   - [TGA image file](https://en.wikipedia.org/wiki/Truevision_TGA)
- `font.bin`
- `qrcode.bin`
   - QR code displayed to get help
   - [TGA image file](https://en.wikipedia.org/wiki/Truevision_TGA)
- `pubkey.bin`
- `bootconf.sig`
- `bootconf.txt`
