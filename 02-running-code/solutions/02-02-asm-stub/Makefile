# It's sometimes good practice to set these with an environment variable
# so we can change toolchains for cross compilation

# Base of the LLVM installation
LLVM_HOME ?= $(shell llvm-config --prefix)
# Check that LLVM_HOME is set
ifeq ($(LLVM_HOME),)
$(error LLVM_HOME is not set. Check that llvm-config is on your PATH or set the LLVM_HOME environment variable)
endif

# We need a few tools to build an image the second stage bootloader can boot


# We need an assembler to turn our human readable assembly into machine code
# Here we get two, the llvm assembler for machine code and the llvm assembler for
# llvm intermediate representation.
LLVM_MC := $(LLVM_HOME)/bin/llvm-mc

# This is the LLVM intermediate representation assembler. It assembles from LLVM IR
# to archetecture specific assembly, the machine code assembler (llvm-mc) can take
# this and assemble it into machine code. If we wanted to bootstrap multiple architectures
# we could try using this.
LLVM_LLC := $(LLVM_HOME)/bin/llc

# The linker takes the machine code and lays it out into a structured file, like an ELF
# Mach-O or PE file. Because LLVM is a cross compilation toolchain, it includes linkers
# for ELF (ld.lld), Mach-O (ld64.lld) and PE (lld-link) and even WebAssembly (wasm-ld).
# You can find these by running the generic linker engine binary (lld) with the --help
# flag.
# The second stage bootloader expects us to provide a UNIX ELF file, so we will use that
# linker.
LLVM_LD := $(LLVM_HOME)/bin/ld.lld

# LLVM version of object copy
LLVM_OBJCOPY := $(LLVM_HOME)/bin/llvm-objcopy



all: kernel8.img

clean: 
	rm -f *.o *.elf *.img || true


entry.s: entry.llvm
	# Use llc to compile the LLVM IR into assembly
	$(LLVM_LLC) -march=arm64 -filetype=asm entry.llvm -o entry.s

entry.o: entry.s
	# Use llvm-mc to assemble the assembly file into an object file
	$(LLVM_MC) --assemble --triple arm64 entry.s -filetype=obj -o entry.o

qemu: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio -gdb tcp::1234 -S
	# To attach from lldb:
	# lldb
	# gdb-remote localhost:1234
	# continue

kernel8.img: entry.o
	# Use ld to link the object file into an ELF
	$(LLVM_LD) --nostdlib entry.o -o kernel8.elf -T link.ld
	# We don't want an ELF header, we want a raw binary for the bootloader to
	# jump to
	$(LLVM_OBJCOPY) -O binary kernel8.elf kernel8.img

disassemble: kernel8.elf
	$(LLVM_HOME)/bin/llvm-objdump --disassemble kernel8.elf
