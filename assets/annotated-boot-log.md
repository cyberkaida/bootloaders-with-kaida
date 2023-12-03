# Annotated boot log

The below is a log captured from the UART pins on a Raspberry Pi 4
during boot.

First we need to [patch the bootloader](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#updating-the-eeprom-configuration)
to [enable UART output](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#BOOT_UART).
We also need to configure the [second stage bootloader](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#updating-the-eeprom-configuration)
and Linux to log to UART.

---

> First we connect to the [JTAGulator](http://www.grandideastudio.com/jtagulator/)
> or serial connection.
> In this example we use the screen utility.

> To find the appropriate tty device, you can disconnect the JTAGulator, run an `ls /dev` and then diff
> after plugging it in.

```
kaida@spinthink:~$ sudo screen /dev/ttyUSB0                                                                                                                  
 
[sudo] password for kaida: 
[screen is terminating] 
kaida@spinthink:~$ 
kaida@spinthink:~$ sudo screen /dev/ttyUSB0 115200 
[screen is terminating] 
kaida@spinthink:~$ 
kaida@spinthink:~$ 
```

> Here we are connected to the JTAGulator and we'll configure it to talk
> to the Raspberry Pi. To do this we need the [pinout](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#gpio-pinout)
> of [UART](https://www.raspberrypi.com/documentation/computers/configuration.html#primary-uart).
> We can use a multimeter to detect the voltage required for UART by testing the transmit
> pin's voltage relative to the GND ground pin. It's also available in the documentation.
> Here GPIO pin 14 is transmit (TX) and is connected to pin 0 on the JTAGulator.
> GPIO pin 15 is receive (RX) and is connected to pin 1 on the JTAGulator.

```
kaida@spinthink:~$ reset 
 
 
                                    UU  LLL 
 JJJ  TTTTTTT AAAAA  GGGGGGGGGGG   UUUU LLL   AAAAA TTTTTTTT OOOOOOO  RRRRRRRRR 
 JJJJ TTTTTTT AAAAAA GGGGGGG       UUUU LLL  AAAAAA TTTTTTTT OOOOOOO  RRRRRRRR 
 JJJJ  TTTT  AAAAAAA GGG      UUU  UUUU LLL  AAA AAA   TTT  OOOO OOO  RRR RRR 
 JJJJ  TTTT  AAA AAA GGG  GGG UUUU UUUU LLL AAA  AAA   TTT  OOO  OOO  RRRRRRR 
 JJJJ  TTTT  AAA  AA GGGGGGGGG UUUUUUUU LLLLLLLL AAAA  TTT OOOOOOOOO  RRR RRR 
  JJJ  TTTT AAA   AA GGGGGGGGG UUUUUUUU LLLLLLLLL AAA  TTT OOOOOOOOO  RRR RRR 
  JJJ  TT                  GGG             AAA                         RR RRR 
 JJJ                        GG             AA                              RRR 
JJJ                          G             A                                 RR 
 
 
           Welcome to JTAGulator. Press 'H' for available commands. 
         Warning: Use of this tool may affect target system behavior! 
 
>  
? 
 
> ? 
? 
 
> h 
Target Interfaces: 
J   JTAG 
U   UART 
G   GPIO 
S   SWD 
 
General Commands: 
V   Set target I/O voltage 
I   Display version information 
H   Display available commands 
 
> V 
Current target I/O voltage: Undefined 
Enter new target I/O voltage (1.4 - 3.3, 0 for off): 1.5 
New target I/O voltage set: 1.5 
Warning: Ensure VADJ is NOT connected to target! 
 
> U 
 
UART> P 
UART pin naming is from the target's perspective. 
Enter X to disable either pin, if desired. 
Enter TXD pin [0]:  
Enter RXD pin [0]: 1 
Enter baud rate [0]: 115200 
Enable local echo? [y/N]:  
Entering UART passthrough! Press Ctrl-X to exit... 
```

> Starting the first stage boot loader. The first stage boot loader must be very small because
> the system RAM is not yet turned on. The first stage bootloader enables system RAM, then loads
> the bigger second stage bootloader into RAM and jumps to it.
> The second stage bootloader is much larger because it supports things like booting over a network
> (needs DHCP, TCP, etc), booting from complex file systems like `ext4` or `NTFS` etc, and outputting
> to the video output (support for HDMI). All of these features make the second stage bootloader too
> large to fit into the CPU cache, which is the only memory available at this time.
> The first stage bootloader is usually written at least partially in raw assembly, and se we want to
> get to the second stage, which can be much larger and written in a higher level language (C) as fast
> as possible.

> The [boot process is documented on the wiki](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#raspberry-pi-4-boot-flow).

```
RPi: BOOTLOADER release VERSION:8ba17717 DATE: 2023/01/11 TIME: 17:40:52 
BOOTMODE: 0x06 partition 0 build-ts BUILD_TIMESTAMP=1673458852 serial 70e1d6ea boardrev c03112 stc 550119 
PM_RSTS: 0x00001000 
part 00000000 reset_info 00000000 
uSD voltage 3.3V 
```
> The first stage bootloader is responsible for turning on the RAM chips
> We do this here, then we will load the second stage bootloader into RAM
```
Initialising SDRAM 'Micron' 16Gb x2 total-size: 32 Gbit 3200 
DDR 3200 1 0 32 152 
 
XHCI-STOP 
xHC ver: 256 HCS: 05000420 fc000031 00e70004 HCC: 002841eb 
USBSTS 11 
xHC ver: 256 HCS: 05000420 fc000031 00e70004 HCC: 002841eb 
xHC ports 5 slots 32 intrs 4 
Reset USB port-power 1000 ms 
xhci_set_port_power 1 0 
xhci_set_port_power 2 0 
xhci_set_port_power 3 0 
xhci_set_port_power 4 0 
xhci_set_port_power 5 0 
xhci_set_port_power 1 1 
xhci_set_port_power 2 1 
xhci_set_port_power 3 1 
xhci_set_port_power 4 1 
xhci_set_port_power 5 1 
XHCI-STOP 
xHC ver: 256 HCS: 05000420 fc000031 00e70004 HCC: 002841eb 
USBSTS 18 
XHCI-STOP 
xHC ver: 256 HCS: 05000420 fc000031 00e70004 HCC: 002841eb 
USBSTS 19 
xHC ver: 256 HCS: 05000420 fc000031 00e70004 HCC: 002841eb 
xHC ports 5 slots 32 intrs 4 
```

> The boot mode is a single byte bitmask which defines how
> [we should boot the system](https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#BOOT_ORDER)

```
Boot mode: SD (01) order f4 
```
> The boot mode tells us to load from the SD card
> We bring up the USB hub on the board and enumerate Devices
> to find the SDCard
```
USB2[1] 400202e1 connected 
USB2 root HUB port 1 init 
DEV [01:00] 2.16 000000:01 class 9 VID 2109 PID 3431 
HUB init [01:00] 2.16 000000:01 
SD HOST: 200000000 CTL0: 0x00800000 BUS: 400000 Hz actual: 390625 HZ div: 512 (256) status: 0x1fff0000 delay: 276 
SD HOST: 200000000 CTL0: 0x00800f00 BUS: 400000 Hz actual: 390625 HZ div: 512 (256) status: 0x1fff0000 delay: 276 
OCR c0ff8000 [14] 
CID: 0003534453433634478080018acb014b 
CSD: 400e00325b590001dbd37f800a404000 
SD: bus-width: 4 spec: 2 SCR: 0x02458043 0x00000000 
SD HOST: 200000000 CTL0: 0x00800f04 BUS: 50000000 Hz actual: 50000000 HZ div: 4 (2) status: 0x1fff0000 delay: 2 
```
> Here we have found the SD Card, and we begin parsing the [Master Boot Record](https://en.wikipedia.org/wiki/Master_boot_record)
> to find the second stage bootloader on a [FAT partition](https://en.wikipedia.org/wiki/File_Allocation_Table).
> In other boot modes we might bring up a serial device to reseive the second stage
> bootloader via a serial connection. This is typically used during "bring up" while the manufacturer is building
> the device. An engineer developing a bootloader needs to rapidly iterate, so having the bootloader wait to load the
> next stage from a serial connection can speed up development.
> TODO: Clarify and reword this.
```
MBR: 0x00002000,  524288 type: 0x0c 
MBR: 0x00082000,124203008 type: 0x83 
MBR: 0x00000000,       0 type: 0x00 
MBR: 0x00000000,       0 type: 0x00 
Trying partition: 0 
type: 32 lba: 8192 oem: 'mkfs.fat' volume: ' bootfs     ' 
rsc 32 fat-sectors 1020 c-count 130554 c-size 4 
root dir cluster 2 sectors 0 entries 0 
FAT32 clusters 130554 
Trying partition: 0 
type: 32 lba: 8192 oem: 'mkfs.fat' volume: ' bootfs     ' 
rsc 32 fat-sectors 1020 c-count 130554 c-size 4 
root dir cluster 2 sectors 0 entries 0 
FAT32 clusters 130554 
```
> At this point we have parsed the FAT filesystem and found the three files the first stage bootloader is interested
> in!
> 1. `config.txt` - This is a [configuration file](https://www.raspberrypi.com/documentation/computers/config_txt.html#what-is-config-txt), we pass to the second stage bootloader.
> 2. `start4.elf` - This is the second stage bootloader, loaded onto the VideoCore GPU to bring up the CPU. This is responsible for loading the other partitions and starting Linux
> 3. `fixup4.dat` - This defines the CPU/GPU memory boundary in the shared memory. It's used to patch the `start4.elf`.
```
Read config.txt bytes     2140 hnd 0x102 
Read start4.elf bytes  2251392 hnd 0x167f 
Read fixup4.dat bytes     5399 hnd 0x108 
0x00c03112 0x00000000 0x00001fff 
MEM GPU: 76 ARM: 948 TOTAL: 1024 
Firmware: 82f3750a65fadae9a38077e3c2e217ad158c8d54 Mar 17 2023 10:50:39 
```
> Now we map `start4.elf` into memory and we jump to it to start the second stage bootloader.
```
Starting start4.elf @ 0xfec00200 partition 0 
XHCI-STOP 
xHC ver: 256 HCS: 05000420 fc000031 00e70004 HCC: 002841eb 
USBSTS 18 
+ 
```
> Welcome to stage 2!
> the second stage bootloader is responsible for powering up the RAM and a few devices, mounting the boot
> partition and finding the Linux kernel, finding the appropriate DeviceTree and starting Linux on the ARM core.
> - [Wikipedia](https://en.wikipedia.org/wiki/Devicetree)
> - [Kernel.org](https://www.kernel.org/doc/html/latest/devicetree/usage-model.html)
> - [Standard](https://www.devicetree.org)
> - [Raspberry Pi 4 Device Tree](https://github.com/raspberrypi/firmware/blob/master/boot/bcm2711-rpi-4-b.dtb)
```
MESS:00:00:05.163105:0: arasan: arasan_emmc_open 
MESS:00:00:05.164765:0: arasan: arasan_emmc_set_clock C0: 0x00800000 C1: 0x000e0047 emmc: 200000000 actual: 390625 div: 0x00000100 target: 400000 min: 400000 max: 400000 delay: 5 
MESS:00:00:05.284998:0: arasan: arasan_emmc_set_clock C0: 0x00800000 C1: 0x000e0047 emmc: 200000000 actual: 390625 div: 0x00000100 target: 400000 min: 400000 max: 400000 delay: 5 
MESS:00:00:05.297870:0: arasan: arasan_emmc_set_clock C0: 0x00800f00 C1: 0x000e0047 emmc: 200000000 actual: 390625 div: 0x00000100 target: 400000 min: 390000 max: 400000 delay: 5 
MESS:00:00:05.344704:0: arasan: arasan_emmc_set_clock C0: 0x00800f06 C1: 0x000e0207 emmc: 200000000 actual: 50000000 div: 0x00000002 target: 50000000 min: 0 max: 50000000 delay: 1 
```
> Load the filesystem and retrieve the `config.txt` - This is the [configuration for the second stage bootloader](https://github.com/raspberrypi/firmware.git).
```
MESS:00:00:05.363049:0: brfs: File read: /mfs/sd/config.txt 
MESS:00:00:05.366500:0: brfs: File read: 2140 bytes 
```
> Here we bring up the HDMI so we can log to the screen.
```
MESS:00:00:05.391717:0: HDMI0:EDID error reading EDID block 0 attempt 0 
MESS:00:00:05.396226:0: HDMI0:EDID giving up on reading EDID block 0 
MESS:00:00:05.413079:0: HDMI1:EDID error reading EDID block 0 attempt 0 
MESS:00:00:05.417583:0: HDMI1:EDID giving up on reading EDID block 0 
MESS:00:00:05.425864:0: brfs: File read: /mfs/sd/config.txt 
MESS:00:00:06.189655:0: gpioman: gpioman_get_pin_num: pin DISPLAY_DSI_PORT not defined 
MESS:00:00:06.196966:0: *** Restart logging 
MESS:00:00:06.198370:0: brfs: File read: 2140 bytes 
MESS:00:00:06.208382:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 0 
MESS:00:00:06.213404:0: hdmi: HDMI0:EDID giving up on reading EDID block 0 
MESS:00:00:06.224028:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 0 
MESS:00:00:06.229057:0: hdmi: HDMI0:EDID giving up on reading EDID block 0 
MESS:00:00:06.234655:0: hdmi: HDMI:hdmi_get_state is deprecated, use hdmi_get_display_state instead 
MESS:00:00:06.248445:0: hdmi: HDMI1:EDID error reading EDID block 0 attempt 0 
MESS:00:00:06.253474:0: hdmi: HDMI1:EDID giving up on reading EDID block 0 
MESS:00:00:06.264095:0: hdmi: HDMI1:EDID error reading EDID block 0 attempt 0 
MESS:00:00:06.269116:0: hdmi: HDMI1:EDID giving up on reading EDID block 0 
MESS:00:00:06.274714:0: hdmi: HDMI:hdmi_get_state is deprecated, use hdmi_get_display_state instead 
MESS:00:00:06.283478:0: HDMI0: hdmi_pixel_encoding: 300000000 
MESS:00:00:06.288951:0: HDMI1: hdmi_pixel_encoding: 300000000 
```
> We're going to find the appropriate device tree for this device. Note the first stage bootloader
> found the board revision. The second stage picks the apppriate device tree for the current hardware
> and will read this file from the SD card `boot` partition, and then pass this to Linux so Linux
> knows what device drives and options to load and set.
> Multiple device trees are loaded, and then overlaid. So additional options can be set or options
> can be changed on top of the base device tree. Things like additional hardware, or boot options
> or settings for the kernel or drives can be set.
```
MESS:00:00:06.294745:0: kernel= 
MESS:00:00:06.302192:0: dtb_file 'bcm2711-rpi-4-b.dtb' 
MESS:00:00:06.311708:0: brfs: File read: /mfs/sd/bcm2711-rpi-4-b.dtb 
MESS:00:00:06.314955:0: Loaded 'bcm2711-rpi-4-b.dtb' to 0x100 size 0xcd71 
MESS:00:00:06.334664:0: brfs: File read: 52593 bytes 
MESS:00:00:06.349226:0: brfs: File read: /mfs/sd/overlays/overlay_map.dtb 
MESS:00:00:06.428344:0: brfs: File read: 2347 bytes 
MESS:00:00:06.433943:0: brfs: File read: /mfs/sd/config.txt 
MESS:00:00:06.436877:0: dtparam: audio=on 
MESS:00:00:06.444711:0: brfs: File read: 2140 bytes 
MESS:00:00:06.463320:0: brfs: File read: /mfs/sd/overlays/vc4-kms-v3d-pi4.dtbo 
MESS:00:00:06.528486:0: Loaded overlay 'vc4-kms-v3d' 
MESS:00:00:06.685065:0: brfs: File read: 3913 bytes 
```
> We load `cmdline.txt` from the `boot` partition. This [contains the Linux kernel command line arguments](https://www.raspberrypi.com/documentation/computers/configuration.html#the-kernel-command-line).
> The name of the file containing the Kernel command line is configurable with the [boot option `cmdline`](https://www.raspberrypi.com/documentation/computers/config_txt.html#cmdline)
> Note that the device tree and bootloader will influence the final kernel command line Linux receives. (see below)
```
MESS:00:00:06.690176:0: brfs: File read: /mfs/sd/cmdline.txt 
MESS:00:00:06.692745:0: Read command line from file 'cmdline.txt': 
MESS:00:00:06.698621:0: 'console=serial0,115200 console=tty1 root=PARTUUID=abb15ddf-02 rootfstype=ext4 fsck.repair=yes rootwait' 
MESS:00:00:06.819816:0: brfs: File read: 102 bytes 
```
> Load the Linux kernel from disk and into system memory.
> The kernel is loaded into the physical address `0x80000` - see [Kernel.org](https://www.kernel.org/doc/html/latest/arch/arm64/booting.html#booting-aarch64-linux)
```
MESS:00:00:07.545247:0: brfs: File read: /mfs/sd/kernel8.img 
MESS:00:00:07.547802:0: Loaded 'kernel8.img' to 0x80000 size 0x7d6bd0 
```
> The kernel is loaded in physical memory to the standard location on aarch64, `0x80000`
```
MESS:00:00:08.727304:0: Kernel relocated to 0x200000 
MESS:00:00:08.729160:0: Device tree loaded to 0x2eff2c00 (size 0xd31e) 
MESS:00:00:08.737058:0: uart: Set PL011 baud rate to 103448.300000 Hz 
MESS:00:00:08.744483:0: uart: Baud rate change done... 
```
> Now we jump to the [Linux kernel entry point (`kernel_init` in init/main.c)](https://github.com/torvalds/linux/blob/master/init/main.c#L140)
> and Linux starts!
> Our second stage bootloader is complete and output continues from Linux.
```
MESS:00:00:08.746506:0:[    0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd083] 
```
> Print the `uname` string. Here we see the builder of this kernel (dom@buildbot). We see it's an Ubuntu
> kernel with [symmetric multiprocessing enabled (SMP)](https://en.wikipedia.org/wiki/Symmetric_multiprocessing)
> and [preemption](https://en.wikipedia.org/wiki/Preemption_(computing)) enabled.
```
[    0.000000] Linux version 6.1.21-v8+ (dom@buildbot) (aarch64-linux-gnu-gcc-8 (Ubuntu/Linaro 8.4.0-3ubuntu1) 8.4.0, GNU ld (GNU Binutils for Ubuntu) 2.34) #1642 SMP PREEMPT Mon Apr  3 17:24:16 BST 2023 
[    0.000000] random: crng init done 
[    0.000000] Machine model: Raspberry Pi 4 Model B Rev 1.2 
[    0.000000] efi: UEFI not found. 
```
> Start reserving memory for [Direct Memory Access (DMA) ranges](https://en.wikipedia.org/wiki/Direct_memory_access)
> These are [physical](https://www.kernel.org/doc/html/latest/mm/physical_memory.html) [pages](https://en.wikipedia.org/wiki/Page_%28computer_memory%29)
> used to communicate with hardware.
```
[    0.000000] Reserved memory: created CMA memory pool at 0x000000000ec00000, size 512 MiB 
[    0.000000] OF: reserved mem: initialized node linux,cma, compatible id shared-dma-pool 
[    0.000000] Zone ranges: 
[    0.000000]   DMA      [mem 0x0000000000000000-0x000000003fffffff] 
[    0.000000]   DMA32    [mem 0x0000000040000000-0x00000000fbffffff] 
[    0.000000]   Normal   empty 
[    0.000000] Movable zone start for each node 
[    0.000000] Early memory node ranges 
[    0.000000]   node   0: [mem 0x0000000000000000-0x000000003b3fffff] 
[    0.000000]   node   0: [mem 0x0000000040000000-0x00000000fbffffff] 
[    0.000000] Initmem setup node 0 [mem 0x0000000000000000-0x00000000fbffffff] 
[    0.000000] On node 0, zone DMA32: 19456 pages in unavailable ranges 
[    0.000000] On node 0, zone DMA32: 16384 pages in unavailable ranges 
[    0.000000] percpu: Embedded 29 pages/cpu s78504 r8192 d32088 u118784 
[    0.000000] Detected PIPT I-cache on CPU0 
```
> Detect CPU features and enable/disable kernel features to match
> In this case we use this to detect if the CPU is vulnerable to the [Spectre exploit](https://en.wikipedia.org/wiki/Spectre_(security_vulnerability)
> We also have options for additional ARM features and security features like [KPTI](https://en.wikipedia.org/wiki/Kernel_page-table_isolation).
```
[    0.000000] CPU features: detected: Spectre-v2 
[    0.000000] CPU features: detected: Spectre-v3a 
[    0.000000] CPU features: detected: Spectre-v4 
[    0.000000] CPU features: detected: Spectre-BHB 
[    0.000000] CPU features: kernel page table isolation forced ON by KASLR 
[    0.000000] CPU features: detected: Kernel page table isolation (KPTI) 
[    0.000000] CPU features: detected: ARM erratum 1742098 
[    0.000000] CPU features: detected: ARM errata 1165522, 1319367, or 1530923 
[    0.000000] alternatives: applying boot alternatives 
[    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 996912 
```
> The kernel command line is [printed here](https://github.com/torvalds/linux/blob/f0b0d403eabbe135d8dbb40ad5e41018947d336c/init/main.c#L901).
> Note that it has changed a lot since we extracted it in the second
> stage bootloader. This is because changes are made by the bootloader and by Linux based on the device tree
> we loaded.
> The `  ` before `console=` is the delimiter, but this is not always the case.
```
[    0.000000] Kernel command line: coherent_pool=1M 8250.nr_uarts=1 snd_bcm2835.enable_headphones=0 snd_bcm2835.enable_headphones=1 snd_bcm2835.enable_hdmi=1 snd_bcm2835.enable_hdmi=0  smsc95xx.macaddr=DC:A6:32:95:86:8D vc_mem.mem_base=0x3ec00000 vc_mem.mem_size=0x40000000  console=ttyS0,115200 console=tty1 root=PARTUUID=abb15ddf-02 rootfstype=ext4 fsck.repair=yes rootwait 
```
> Start allocating memory for the directory entries (`dentry` or `dentries`) and [index nodes (`inode`)](https://en.wikipedia.org/wiki/Inode) caches.
> These will be required when we spin up the virtual file system to load the userland later.
```
[    0.000000] Dentry cache hash table entries: 524288 (order: 10, 4194304 bytes, linear) 
[    0.000000] Inode-cache hash table entries: 262144 (order: 9, 2097152 bytes, linear) 
```
> Allocate the [Translation Lookaside Buffer (TLB)](https://en.wikipedia.org/wiki/Translation_lookaside_buffer)
> this is a [cache used by Linux](https://www.kernel.org/doc/html/latest/arch/x86/tlb.html?highlight=tlb)
> to refer to the mapping between Physical and Virtual memory and speed up
> [Memory Management Unit (MMU)](https://en.wikipedia.org/wiki/Memory_management_unit) operations
```
[    0.000000] mem auto-init: stack:off, heap alloc:off, heap free:off 
[    0.000000] software IO TLB: area num 4. 
[    0.000000] software IO TLB: mapped [mem 0x0000000037400000-0x000000003b400000] (64MB) 
[    0.000000] Memory: 3356948K/4050944K available (11776K kernel code, 2106K rwdata, 3688K rodata, 4160K init, 1077K bss, 169708K reserved, 524288K cma-reserved) 
```
> We allacte the pages for the [SLUB allocator](https://www.kernel.org/doc/html/latest/mm/slub.html?highlight=slub)
> which is a [SLAB allocator](https://en.wikipedia.org/wiki/Slab_allocation)
```
[    0.000000] SLUB: HWalign=64, Order=0-3, MinObjects=0, CPUs=4, Nodes=1 
```
> Allocate the memory for [ftrace (function tracing)](https://en.wikipedia.org/wiki/Ftrace)
```
[    0.000000] ftrace: allocating 39744 entries in 156 pages 
[    0.000000] ftrace: allocated 156 pages with 4 groups 
[    0.000000] trace event string verifier disabled 
[    0.000000] rcu: Preemptible hierarchical RCU implementation. 
[    0.000000] rcu:     RCU event tracing is enabled. 
[    0.000000] rcu:     RCU restricting CPUs from NR_CPUS=256 to nr_cpu_ids=4. 
[    0.000000]  Trampoline variant of Tasks RCU enabled. 
[    0.000000]  Rude variant of Tasks RCU enabled. 
[    0.000000]  Tracing variant of Tasks RCU enabled. 
[    0.000000] rcu: RCU calculated value of scheduler-enlistment delay is 25 jiffies. 
[    0.000000] rcu: Adjusting geometry for rcu_fanout_leaf=16, nr_cpu_ids=4 
```
> Allocate the [interrupt requests (IRQs) ](https://en.wikipedia.org/wiki/Interrupt_request). [IRQ handling subsystem](https://www.kernel.org/doc/html/v4.12/core-api/genericirq.html)
> [IRQ setup](https://github.com/torvalds/linux/blob/f0b0d403eabbe135d8dbb40ad5e41018947d336c/init/main.c#L970)
> is specific to each architecture Linux supports, and each handles things differently.
> The arm64 setup is located under [arch/arm64/kernel/irq.c](https://github.com/raspberrypi/linux/blob/e079555a4c68356e58249cfc041b28f6eb455bd5/arch/arm64/kernel/irq.c)
> See the [Linux documentation for the IRQ subsystem](https://github.com/torvalds/linux/tree/master/Documentation/core-api/irq).
```
[    0.000000] NR_IRQS: 64, nr_irqs: 64, preallocated irqs: 0 
[    0.000000] Root IRQ handler: gic_handle_irq 
[    0.000000] GIC: Using split EOI/Deactivate mode 
[    0.000000] rcu: srcu_init: Setting srcu_struct sizes based on contention. 
[    0.000000] arch_timer: cp15 timer(s) running at 54.00MHz (phys). 
[    0.000000] clocksource: arch_sys_counter: mask: 0xffffffffffffff max_cycles: 0xc743ce346, max_idle_ns: 440795203123 ns 
[    0.000001] sched_clock: 56 bits at 54MHz, resolution 18ns, wraps every 4398046511102ns 
```
> At this point we set up the output device (an 80x25 character pseudo [teletype](https://en.wikipedia.org/wiki/Teleprinter))
> Since we have serial enabled, this will also output to the serial device (UART), this is attached to the JTAGulator
```
[    0.000300] Console: colour dummy device 80x25 
[    0.000910] printk: console [tty1] enabled 
[    0.000980] Calibrating delay loop (skipped), value calculated using timer frequency.. 108.00 BogoMIPS (lpj=216000) 
[    0.001021] pid_max: default: 32768 minimum: 301 
[    0.001160] L0] cblist_init_generic: Setting shift to 2 and lim to 1. 
[    0.005705] rcu: Hierarchical SRCU implementation. 
[    0.005730] rcu:     Max phase no-delay instances is 1000. 
[    0.007035] EFI services will not be available. 
```
> Enter into [Symmetric Multiprocessing mode (SMP)](https://en.wikipedia.org/wiki/Symmetric_multiprocessing)
> Here we bring each additional CPU online. Until now we have been running only on core0. The other cores
> have been parked in a "busy loop" running the same instruction over and over.
> The [busy loop implementation is in the first stage boot loader](https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S#L156).
```
[    0.007551] smp: Bringing up secondary CPUs ... 
[    0.008668] Detected PIPT I-cache on CPU1 
[    0.008814] CPU1: Booted secondary processor 0x0000000001 [0x410fd083] 
[    0.009936] Detected PIPT I-cache on CPU2 
[    0.010054] CPU2: Booted secondary processor 0x0000000002 [0x410fd083] 
[    0.011143] Detected PIPT I-cache on CPU3 
[    0.011267] CPU3: Booted secondary processor 0x0000000003 [0x410fd083] 
```
> We have one physical processor with 4 CPUs.
> Note that after this point things can happen in parallel. Until now everything was single thread, single core.
```
[    0.011413] smp: Brought up 1 node, 4 CPUs 
[    0.011503] SMP: Total of 4 processors activated. 
```
> Now let's evaluate our [exception levels, we started at `EL2` and we can drop down to `EL1`](https://en.wikipedia.org/wiki/Protection_ring)
```
[    0.011525] CPU features: detected: 32-bit EL0 Support 
[    0.011543] CPU features: detected: 32-bit EL1 Support 
[    0.011565] CPU features: detected: CRC32 instructions 
[    0.011710] CPU: All CPU(s) started at EL2 
[    0.011743] alternatives: applying system-wide alternatives 
[    0.013533] devtmpfs: initialized 
[    0.025337] Enabled cp15_barrier support 
[    0.025397] Enabled setend support 
[    0.025616] clocksource: jiffies: mask: 0xffffffff max_cycles: 0xffffffff, max_idle_ns: 7645041785100000 ns 
[    0.025667] futex hash table entries: 1024 (order: 4, 65536 bytes, linear) 
[    0.038093] pinctrl core: initialized pinctrl subsystem 
[    0.038937] DMI not present or invalid. 
```
> Bring the network subsystem online
```
[    0.039610] NET: Registered PF_NETLINK/PF_ROUTE protocol family 
[    0.043143] DMA: preallocated 1024 KiB GFP_KERNEL pool for atomic allocations 
[    0.043459] DMA: preallocated 1024 KiB GFP_KERNEL|GFP_DMA pool for atomic allocations 
[    0.044429] DMA: preallocated 1024 KiB GFP_KERNEL|GFP_DMA32 pool for atomic allocations 
[    0.044534] audit: initializing netlink subsys (disabled) 
[    0.044840] audit: type=2000 audit(0.044:1): state=initialized audit_enabled=0 res=1 
```
> Thermal power management online, this will underclock the CPU during hot times.
```
[    0.045439] thermal_sys: Registered thermal governor 'step_wise' 
[    0.045529] cpuidle: using governor menu 
[    0.045932] hw-breakpoint: found 6 breakpoint and 4 watchpoint registers. 
[    0.046141] ASID allocator initialised with 32768 entries 
[    0.046315] Serial: AMBA PL011 UART driver 
```
> Bring the Broadcom 2835 mailbox online. This is used for routing interrupts (IRQs).
> See the bcm2711 datasheet and search for `Chapter 13. ARM Mailboxes`
> [Raspberry Pi Documentation](https://www.raspberrypi.com/documentation/computers/processors.html#bcm2711)
> [Linux driver](https://github.com/torvalds/linux/blob/master/drivers/mailbox/bcm2835-mailbox.c)
```
[    0.060010] bcm2835-mbox fe00b880.mailbox: mailbox enabled 
[    0.080786] raspberrypi-firmware soc:firmware: Attached to firmware from 2023-03-17T10:50:39, variant start 
[    0.084800] raspberrypi-firmware soc:firmware: Firmware hash is 82f3750a65fadae9a38077e3c2e217ad158c8d54 
[    0.100782] KASLR enabled 
[    0.137986] bcm2835-dma fe007000.dma: DMA legacy API manager, dmachans=0x1 
[    0.143746] SCSI subsystem initialized 
```
> Bring up the USB subsystem, find USB devices.
```
[    0.144022] usbcore: registered new interface driver usbfs 
[    0.144094] usbcore: registered new interface driver hub 
[    0.144172] usbcore: registered new device driver usb 
[    0.144571] usb_phy_generic phy: supply vcc not found, using dummy regulator 
[    0.144797] usb_phy_generic phy: dummy supplies not allowed for exclusive requests 
[    0.145162] pps_core: LinuxPPS API ver. 1 registered 
[    0.145187] pps_core: Software ver. 5.3.6 - Copyright 2005-2007 Rodolfo Giometti <giometti@linux.it> 
[    0.145229] PTP clock support registered 
[    0.146371] vgaarb: loaded 
[    0.147080] clocksource: Switched to clocksource arch_sys_counter 
```
> Bringing up the [Virtual File System (VFS)](https://en.wikipedia.org/wiki/Virtual_file_system).
> [Kernel.org](https://www.kernel.org/doc/html/latest/filesystems/vfs.html)
> This is the kernel abstraction over file systems. Provides a unified view of the file system tree.
```
[    0.147828] VFS: Disk quotas dquot_6.6.0 
[    0.147926] VFS: Dquot-cache hash table entries: 512 (order 0, 4096 bytes) 
[    0.148121] FS-Cache: Loaded 
[    0.148318] CacheFiles: Loaded 
```
> Network subsystem registers the `PF_INET` family to enable TCP and UDP connections over the
> [Internet Protocol](https://en.wikipedia.org/wiki/Internet_Protocol).
> We allocate a number of caches and queues for connections.
```
[    0.158963] NET: Registered PF_INET protocol family 
[    0.159434] IP idents hash table entries: 65536 (order: 7, 524288 bytes, linear) 
[    0.164134] tcp_listen_portaddr_hash hash table entries: 2048 (order: 3, 32768 bytes, linear) 
[    0.164214] Table-perturb hash table entries: 65536 (order: 6, 262144 bytes, linear) 
[    0.164349] TCP established hash table entries: 32768 (order: 6, 262144 bytes, linear) 
[    0.164568] TCP bind hash table entries: 32768 (order: 8, 1048576 bytes, linear) 
[    0.165471] TCP: Hash tables configured (established 32768 bind 32768) 
[    0.165692] UDP hash table entries: 2048 (order: 4, 65536 bytes, linear) 
[    0.165784] UDP-Lite hash table entries: 2048 (order: 4, 65536 bytes, linear) 
[    0.166088] NET: Registered PF_UNIX/PF_LOCAL protocol family 
[    0.166905] RPC: Registered named UNIX socket transport module. 
[    0.166939] RPC: Registered udp transport module. 
[    0.16696r=20 bucket_order=0 
[    1.322430] zbud: loaded 
```
> Now that we have both a network stack and the virtual file system started,
> we can start up the [Network File System (NFS)](https://en.wikipedia.org/wiki/Network_File_System) driver.
```
[    1.325360] NFS: Registering the id_resolver key type 
[    1.325417] Key type id_resolver registered 
[    1.325439] Key type id_legacy registered 
[    1.325560] nfs4filelayout_init: NFSv4 File Layout Driver Registering... 
[    1.325589] nfs4flexfilelayout_init: NFSv4 Flexfile Layout Driver Registering... 
[    1.327044] Key type asymmetric registered 
[    1.327117] Asymmetric key parser 'x509' registered 
[    1.327220] Block layer SCSI generic (bsg) driver version 0.4 loaded (major 247) 
[    1.327496] io scheduler mq-deadline registered 
[    1.327524] io scheduler kyber registered 
[    1.329335] irq_brcmstb_l2: registered L2 intc (/soc/interrupt-controller@7ef00100, parent irq: 27) 
[    1.337295] brcm-pcie fd500000.pcie: host bridge /scb/pcie@7d500000 ranges: 
[    1.337359] brcm-pcie fd500000.pcie:   No bus range found for /scb/pcie@7d500000, using [bus 00-ff] 
[    1.337464] brcm-pcie fd500000.pcie:      MEM 0x0600000000..0x063fffffff -> 0x00c0000000 
[    1.337564] brcm-pcie fd500000.pcie:   IB MEM 0x0000000000..0x00bfffffff -> 0x0400000000 
[    1.338613] brcm-pcie fd500000.pcie: PCI host bridge to bus 0000:00 
[    1.338646] pci_bus 0000:00: root bus resource [bus 00-ff] 
[    1.338675] pci_bus 0000:00: root bus resource [mem 0x600000000-0x63fffffff] (bus address [0xc0000000-0xffffffff]) 
[    1.338770] pci 0000:00:00.0: [14e4:2711] type 01 class 0x060400 
[    1.339036] pci 0000:00:00.0: PME# supported from D0 D3hot 
[    1.342975] pci 0000:00:00.0: bridge configuration invalid ([bus 00-00]), reconfiguring 
[    1.343269] pci_bus 0000:01: supply vpcie3v3 not found, using dummy regulator 
[    1.343470] pci_bus 0000:01: supply vpcie3v3aux not found, using dummy regulator 
[    1.343586] pci_bus 0000:01: supply vpcie12v not found, using dummy regulator 
[    1.381183] brcm-pcie fd500000.pcie: link up, 5.0 GT/s PCIe x1 (SSC) 
[    1.381363] pci 0000:01:00.0: [1106:3483] type 00 class 0x0c0330 
[    1.381501] pci 0000:01:00.0: reg 0x10: [mem 0x00000000-0x00000fff 64bit] 
[    1.381954] pci 0000:01:00.0: PME# supported from D0 D3hot 
[    1.382656] pci_bus 0000:01: busn_res: [bus 01-ff] end is updated to 01 
[    1.382712] pci 0000:00:00.0: BAR 8: assigned [mem 0x600000000-0x6000fffff] 
[    1.382747] pci 0000:01:00.0: BAR 0: assigned [mem 0x600000000-0x600000fff 64bit] 
[    1.382804] pci 0000:00:00.0: PCI bridge to [bus 01] 
[    1.382835] pci 0000:00:00.0:   bridge window [mem 0x600000000-0x6000fffff] 
[    1.391474] Serial: 8250/16550 driver, 1 ports, IRQ sharing enabled 
[    1.394624] iproc-rng200 fe104000.rng: hwrng registered 
[    1.395251] vc-mem: phys_addr:0x00000000 mem_base=0x3ec00000 mem_size:0x40000000(1024 MiB) 
[    1.396763] gpiomem-bcm2835 fe200000.gpiomem: Initialised: Registers at 0xfe200000 
[    1.409123] brd: module loaded 
[    1.417594] loop: module loaded 
[    1.418398] Loading iSCSI transport class v2.0-870. 
[    1.423749] bcmgenet fd580000.ethernet: GENET 5.0 EPHY: 0x0000 
[    1.483243] unimac-mdio unimac-mdio.-19: Broadcom UniMAC MDIO bus 
[    1.484388] usbcore: registered new interface driver r8152 
[    1.484481] usbcore: registered new interface driver lan78xx 
[    1.484571] usbcore: registered new interface driver smsc95xx 
[    1.486136] pci 0000:00:00.0: enabling device (0000 -> 0002) 
[    1.486189] xhci_hcd 0000:01:00.0: enabling device (0000 -> 0002) 
[    1.486311] xhci_hcd 0000:01:00.0: xHCI Host Controller 
[    1.486354] xhci_hcd 0000:01:00.0: new USB bus registered, assigned bus number 1 
[    1.487046] xhci_hcd 0000:01:00.0: hcc params 0x002841eb hci version 0x100 quirks 0x0000e40000000890 
[    1.487759] xhci_hcd 0000:01:00.0: xHCI Host Controller 
[    1.487793] xhci_hcd 0000:01:00.0: new USB bus registered, assigned bus number 2 
[    1.487829] xhci_hcd 0000:01:00.0: Host supports USB 3.0 SuperSpeed 
[    1.488182] usb usb1: New USB device found, idVendor=1d6b, idProduct=0002, bcdDevice= 6.01 
[    1.488218] usb usb1: New USB device strings: Mfr=3, Product=2, SerialNumber=1 
[    1.488246] usb usb1: Product: xHCI Host Controller 
[    1.488269] usb usb1: Manufacturer: Linux 6.1.21-v8+ xhci-hcd 
[    1.488291] usb usb1: SerialNumber: 0000:01:00.0 
[    1.489025] hub 1-0:1.0: USB hub found 
[    1.489135] hub 1-0:1.0: 1 port detected 
[    1.490132] usb usb2: New USB device found, idVendor=1d6b, idProduct=0003, bcdDevice= 6.01 
[    1face driver usb-storage 
[    1.494089] mousedev: PS/2 mouse device common for all mice 
```
> Getting ready to mount the SD card.
```
[    1.500084] sdhci: Secure Digital Host Controller Interface driver 
[    1.500119] sdhci: Copyright(c) Pierre Ossman 
[    1.500829] sdhci-pltfm: SDHCI platform and OF driver helper 
[    1.504337] ledtrig-cpu: registered to indicate activity on CPUs 
[    1.504711] hid: raw HID events driver (C) Jiri Kosina 
[    1.504947] usbcore: registered new interface driver usbhid 
[    1.504973] usbhid: USB HID core driver 
[    1.513311] NET: Registered PF_PACKET protocol family 
[    1.513468] Key type dns_resolver registered 
[    1.515152] registered taskstats version 1 
[    1.515257] Loading compiled-in X.509 certificates 
[    1.516139] Key type .fscrypt registered 
[    1.516167] Key type fscrypt-provisioning registered 
[    1.531497] uart-pl011 fe201000.serial: there is not valid maps for state default 
[    1.532250] uart-pl011 fe201000.serial: cts_event_workaround enabled 
[    1.532407] fe201000.serial: ttyAMA0 at MMIO 0xfe201000 (irq = 36, base_baud = 0) is a PL011 rev2 
[    1.540555] bcm2835-aux-uart fe215040.serial: there is not valid maps for state default 
[    1.541345] printk: console [ttyS0] disabled 
[    1.541471] fe215040.serial: ttyS0 at MMIO 0xfe215040 (irq = 37, base_baud = 62500000) is a 16550 
[    1.755103] usb 1-1: new high-speed USB device number 2 using xhci_hcd 
[    1.761808] printk: console [ttyS0] enabled 
[    1.975538] usb 1-1: New USB device found, idVendor=2109, idProduct=3431, bcdDevice= 4.21 
[    1.989258] bcm2835-wdt bcm2835-wdt: Broadcom BCM2835 watchdog timer 
[    1.994120] usb 1-1: New USB device strings: Mfr=0, Product=1, SerialNumber=0 
[    1.994137] usb 1-1: Product: USB2.0 Hub 
[    1.994618] bcm2835-power bcm2835-power: Broadcom BCM2835 power domains driver 
[    2.001405] hub 1-1:1.0: USB hub found 
[    2.005699] mmc-bcm2835 fe300000.mmcnr: mmc_debug:0 mmc_debug2:0 
[    2.009590] hub 1-1:1.0: 4 ports detected 
[    2.013653] mmc-bcm2835 fe300000.mmcnr: DMA channel allocated 
[    3.076323] of_cfs_init 
[    3.078946] of_cfs_init: OK 
```
> Here we find the SD card and enumerate file systems to look for our root partition
> that was specified in the kernel command line
```
[    3.111131] mmc0: SDHCI controller on fe340000.mmc [fe340000.mmc] using ADMA 
```
> The command line argument for the root partition is parsed here.
> We find the device and mount it as our root partition.
```
[    3.118736] Waiting for root device PARTUUID=abb15ddf-02... 
[    3.170035] mmc1: new high speed SDIO card at address 0001 
[    3.224750] mmc0: new ultra high speed DDR50 SDXC card at address aaaa 
[    3.232386] mmcblk0: mmc0:aaaa SC64G 59.5 GiB  
[    3.242566]  mmcblk0: p1 p2 
[    3.245953] mmcblk0: mmc0:aaaa SC64G 59.5 GiB (quirks 0x00004000) 
[    3.276695] EXT4-fs (mmcblk0p2): INFO: recovery required on readonly filesystem 
[    3.284202] EXT4-fs (mmcblk0p2): write access will be enabled during recovery 
[    3.455870] EXT4-fs (mmcblk0p2): recovery complete 
[    3.464611] EXT4-fs (mmcblk0p2): mounted filesystem with ordered data mode. Quota mode: none. 
```
> Mounted!
```
[    3.473398] VFS: Mounted root (ext4 filesystem) readonly on device 179:2. 
[    3.488560] devtmpfs: mounted 
```
> Now we can start freeing memory we reserved for the log and other things during boot
> We're about to start the first user land application, `/sbin/init`
```
[    3.499208] Freeing unused kernel memory: 4160K 
[    3.504041] Run /sbin/init as init process 
[    3.861402] systemd[1]: System time before build time, advancing clock. 
[    4.042486] NET: Registered PF_INET6 protocol family 
[    4.049412] Segment Routing with IPv6 
[    4.053220] In-situ OAM (IOAM) with IPv6 
```
> Welcome to user land! `systemd` is going to get read to pivotroot and start services
> so we can log in and use our computer!
```
[    4.129995] systemd[1]: systemd 247.3-7+deb11u2 running in system mode. (+PAM +AUDIT +SELINUX +IMA +APPARMOR +SMACK +SYSVINIT +UTMP +LIBCRYPTSETUP +GCRYPT +GNUTLS +ACL +XZ +LZ4 +ZSTD +SECCOMP +BLKID +ELFUTILS +KMOD +IDN2 -IDN +PCRE2 default-hierarchy=unified) 
[    4.154472] systemd[1]: Detected architecture arm64. 
[    4.168670] systemd[1]: Set hostname to <raspberrypi>. 
[    4.844386] systemd[1]: Queued start job for default target Multi-User System. 
[    4.872764] systemd[1]: Created slice system-getty.slice. 
[    4.880070] systemd[1]: Created slice system-modprobe.slice. 
[    4.887245] systemd[1]: Created slice system-serial\x2dgetty.slice. 
[    4.894946] systemd[1]: Created slice system-systemd\x2dfsck.slice. 
[    4.902550] systemd[1]: Created slice User and Session Slice. 
[    4.909240] systemd[1]: Started Dispatch Password Requests to Console Directory Watch. 
[    4.918073] systemd[1]: Started Forward Password Requests to Wall Directory Watch. 
[    4.927186] systemd[1]: Set up automount Arbitrary Executable File Formats File System Automount Point. 
[    4.937313] systemd[1]: Reached target Local Encrypted Volumes. 
[    4.943826] systemd[1]: Reached target Paths. 
[    4.948705] systemd[1]: Reached target Slices. 
[    4.953645] systemd[1]: Reached target Swap. 
[    4.963393] systemd[1]: Listening on Syslog Socket. 
[    4.969712] systemd[1]: Listening on fsck to fsckd communication Socket. 
[    4.977275] systemd[1]: Listening on initctl Compatibility Named Pipe. 
[    4.985448] systemd[1]: Listening on Journal Audit Socket. 
[    4.992168] systemd[1]: Listening on Journal Socket (/dev/log). 
[    4.999585] systemd[1]: Listening on Journal Socket. 
[    5.011506] systemd[1]: Listening on udev Control Socket. 
[    5.018354] systemd[1]: Listening on udev Kernel Socket. 
[    5.025038] systemd[1]: Condition check resulted in Huge Pages File System being skipped. 
[    5.055577] systemd[1]: Mounting POSIX Message Queue File System... 
[    5.067276] systemd[1]: Mounting RPC Pipe File System... 
[    5.078428] systemd[1]: Mounting Kernel Debug File System... 
[    5.089860] systemd[1]: Mounting Kernel Trace File System... 
[    5.096468] systemd[1]: Condition check resulted in Kernel Module supporting RPCSEC_GSS being skipped. 
[    5.112538] systemd[1]: Starting Restore / save the current clock... 
[    5.124545] systemd[1]: Starting Set the console keyboard layout... 
[    5.136949] systemd[1]: Starting Create list of static device nodes for the current kernel... 
[    5.152108] systemd[1]: Starting Load Kernel Module configfs... 
[    5.163723] systemd[1]: Starting Load Kernel Module drm... 
[    5.174974] systemd[1]: Starting Load Kernel Module fuse... 
[    5.187545] systemd[1]: Condition check resulted in Set Up Additional Binary Formats being skipped. 
[    5.201242] systemd[1]: Starting File System Check on Root Device... 
[    5.217609] systemd[1]: Starting Journal Service... 
[    5.235570] systemd[1]: Starting Load Kernel Modules... 
[    5.250483] systemd[1]: Starting Coldplug All udev Devices... 
[    5.288874] fuse: init (API version 7.37) 
[    5.291491] systemd[1]: Mounted POSIX Message Queue File System. 
[    5.301851] systemd[1]: Mounted RPC Pipe File System. 
[    5.308248] systemd[1]: Mounted Kernel Debug File System. 
[    5.315294] systemd[1]: Mounted Kernel Trace File System. 
[    5.323788] systemd[1]: Finished Restore / save the current clock. 
[    5.333678] systemd[1]: Finished Create list of static device nodes for the current kernel. 
[    5.356117] systemd[1]: modprobe@configfs.service: Succeeded. 
[    5.375308] systemd[1]: Finished Load Kernel Module configfs. 
[    5.386652] systemd[1]: modprobe@fuse.service: Succeeded. 
[    5.394730] systemd[1]: Finished Load Kernel Module fuse. 
[    5.403863] systemd[1]: modprobe@drm.service: Succeeded. 
[    5.411699] systemd[1]: Finished Load Kernel Module drm. 
[    5.420786] systemd[1]: Finished Load Kernel Modules. 
[    5.455724] systemd[1]: Mounting FUSE Control File System... 
[    5.466822] systemd[1]: Mounting Kernel Configuration File System... 
[    5.487985] systemd[1]: Started File System Check Daemon to report status. 
[    5.501049] systemd[1]: Starting Apply Kernel Variables... 
[    5.518734] systemd[1]: Mounted FUSE Control File System. 
[    5.536235] systemd[1]: Mounted Kernel Configuration File System. 
```
> The `systemd` init process has finished it's work. We get a login prompt and our GNU/Linux system
> is ready to use!
```
 
Debian GNU/Linux 11 raspberrypi ttyS0 
 
raspberrypi login:  
```
