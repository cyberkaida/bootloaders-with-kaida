# Welcome to Bootloaders!

To work on this class you will need a few things!

- A Linux or Mac machine
- Some compilation tools
- A Raspberry Pi 4 or Raspberry Pi 400
- A UART Serial Cable

## Machine Setup

- [llvm](https://llvm.org) - The LLVM compiler toolchain for building our bootloader
- [clang](https://clang.llvm.org) - The C compiler from LLVM
- [python3](https://www.python.org) - For small scripts
- [qemu](https://www.qemu.org) - An Emulator for running small VMs to test the bootloaer early on
- [make](https://www.gnu.org/software/make/) - For compiling our project

Optionally we can use an RE tool to examine our binaries. We can also use `llvm-objdump` to examine
if you would prefer to use the command line.

- [Ghidra](https://ghidra-sre.org) - Open source RE tool OK for our class
- [BinaryNinja](https://binary.ninja) - A paid RE tool, the free version should be OK for our class
- [Cutter](https://cutter.re) - A GUI version of Rizin
- [rizin](https://rizin.re) - A small reverse engineering tool, for some minor checks

### Mac

To set up our Mac, we will use the [brew package manager](https://brew.sh)

We will then install:

```sh
brew install llvm clang rizin python3 qemu make screen
```

### Ubuntu

```sh
apt install qemu-system-aarch64 qemu-kvm make clang llvm lld screen
```

### GitHub CodeSpace

The [GitHub CodeSpace](https://docs.github.com/en/codespaces)
is a hosted virtual machine with the tools required for this class pre-installed.

You can use this to build the exercises and then copy
the build outputs to a Raspberry Pi!

To use the CodeSpace you will need to set up the
[GitHub command line tool](https://github.com/cli/cli#installation)

```sh
# You will need to authenticate at least once
gh auth
gh auth refresh -h github.com -s codespace

# Once authenticated you can create the VM. Don't forget to
# stop it when you are done, and delete it when you don't need it.
gh codespace create --repo cyberkaida/bootloaders-with-kaida
gh codespace stop

# You can copy files and ssh to the codespace with
gh codespace cp
gh codespace ssh
```
