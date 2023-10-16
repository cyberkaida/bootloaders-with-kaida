/*
 * In this exercise we will use direct memory access to control
 * the mini UART controller. The UART controller has a set of
 * "registers" that are mapped in at a defined memory region.
 * 
 * To implement this we will need to read the BCM2711 datasheet.
 * The datasheet will list the Address Map, this shows where
 * various peripherals, like the UART controller, maps its
 * memory.
 *
 * https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf
 * 
 * "1.2. Address map" lists the address map provided by the BCM2711.
 *
 * The UART controller is described here:
 * Chapter 2. Auxiliaries: UART1, SPI1 & SPI2
 * You might be tempted to use the address listed here, 
 * > The Auxiliary register base address is 0x7e215000.
 * It is confusing, but this is actially a "Legacy master address"
 * described in:
 * 1.2.4. Legacy master addresses
 *
 * We actually need to subtract 0x7E000000 from the address
 * and add the low mode peripheral base address,
 * 0xFE000000. If you do not do this, you will write to the wrong
 * location, and nothing will work (don't worry, it took me hours
 * to figure this out!)
 */

// First we will define the constants we will use
#define LEGACY_MASTER_BASE 0x7E000000 // From section 1.2.4
#define LOW_PERIPHERAL_BASE 0xFE000000 // From section 1.2.2

// The address listed in section 2.1 is 0x7E215000
// We must subtract the legacy master base address and
// add the low peripheral base address.
#define AUX_BASE (LOW_PERIPHERAL_BASE + 0x215000)

/*
 * The following section describes each of the memory addresses
 * we will need to configure and use the UART controller.
 * These are defined as offsets from the AUX_BASE in section 2.1
 * of the datasheet.
 *
 * We could also create a big struct and use that to access the
 * registers. We will just use the offsets for now.
 *
 * If you are interested in what each of these do and the exact
 * format, each is described in the datasheet, linked from the
 * table in section 2.1. I have also written a description here.
 */

// Now we will define the offsets for the registers we will use
// Note these are not truly registers, they are memory locations
// but they operate like direct access to the registers on the
// UART controller.

// The AUX_ENABLES register is used to enable the UART controller
#define AUX_ENABLES (AUX_BASE + 0x4)

// The AUX_MU_IO_REG register is used to send and receive data
// from the UART controller. It is a volatile location, this means
// it can change without our code doing anything. This is because it
// is controlled from the UART controller side.
// This memory location can be read from to receive data, and written to
// to send data.
#define AUX_MU_IO_REG (AUX_BASE + 0x40)

// The AUX_MU_IER_REG register is used to enable interrupts
// from the UART controller. We will disable interrupts for now.
// We can implement interrupt handlers in a later exercise.
#define AUX_MU_IER_REG (AUX_BASE + 0x44)

// The AUX_MU_IIR_REG register is used to read and set the interrupt
// status. We will use this register to disable interrupts.
#define AUX_MU_IIR_REG (AUX_BASE + 0x48)

// The AUX_MU_LCR_REG register is used to set the data line format.
// We will use this register to set the data line to 8 bits.
#define AUX_MU_LCR_REG (AUX_BASE + 0x4C)

// The AUX_MU_MCR_REG register is used control modem signals.
#define AUX_MU_MCR_REG (AUX_BASE + 0x50)

// The AUX_MU_LSR_REG register is used to read the status of the
// buffer in the UART controller. If we used interrupts, the UART
// controller could send an interrupt to alert us when data is ready.
// Since we are not using interrupts, we will poll this register to
// check the UARTs queues.
#define AUX_MU_LSR_REG (AUX_BASE + 0x54)
// The LSR register has two fields we are interested in
// bit 0: 0 if the receive FIFO is empty, 1 if it is not empty
// bit 6: 0 if the transmit FIFO is full, 1 if it is not full
#define LSR_MASK_TX_READY 0b100000 // You might see this as 0x20 in some
				   // documentation, I find this is easier
				   // to understand.
#define LSR_MASK_RX_READY 0b000001

// NOTE: We skip some registers we don't need, next is 0x60

// The AUX_MU_CNTL_REG register is used to control the UART controller.
// We will use this register to enable the transmitter and receiver after
// we have configured the UART controller.
#define AUX_MU_CNTL_REG (AUX_BASE + 0x60)

// The AUX_MU_BAUD_REG register is used to set the baud rate.
// We will use this register to set the baud rate to 115200.
// This matches the baud used by the first and second stage
// bootloaders. This way we don't need to change settings on
// our serial adapter.
#define AUX_MU_BAUD_REG (AUX_BASE + 0x68)

/*
 * Finally! We have defined all the register values we will need.
 * Now we have a few more constants to define.
 */

// We need to make sure we communicate at the correct clock speed
// The UART controller uses the system clock, which is 250MHz.
#define SYSTEM_CLOCK_FREQ 500000000 // We can set this in the config.txt
// A formula is given in section 2.2.1 to calculate the baud rate.
#define AUX_MU_BAUD(baud) ((SYSTEM_CLOCK_FREQ / (8 * (baud))) - 1)

/*
 * Now we can start implementing our functions.
 * We will need a few helpers to read and write to these
 * registers.
 */

void set_register(unsigned long register_location, unsigned int value) {
	// We need to cast the register to a pointer to a volatile
	// unsigned long pointer. The volatile keyword tells the compiler
	// to not trust that the value hasn't changed. This disables
	// certain optimisations, like if we were to write to the same
	// register twice in a row. We want both writes to go through and not
	// be omtimised out.
	*(volatile unsigned int *)register_location = value;
}

unsigned long get_register(unsigned long register_location) {
	// We need to cast the register to a pointer to a volatile
	// unsigned long pointer. The volatile keyword tells the compiler
	// to not trust that the value hasn't changed. This disables
	// certain optimisations, like if we were to write to the same
	// register twice in a row. We want both writes to go through and not
	// be omtimised out.
	return *(volatile unsigned int *)register_location;
}

/* We also need to interact with the GPIO controller
 * to reset the pins used for the mini UART. If we
 * do not do this we might be in a weird state and
 * UART may not work.
 *
 * Each PIN on the GPIO can be in one of 5 modes
 * on the Raspberry Pi 4.
 * The datasheet lists these in section
 * 5.3. Alternative Function Assignments
 *
 * The pins we are interested in are listed here
 * https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#more
 * We want to use GPIO pins 14 and 15
 *
 * To use these for UART we need to set
 * GPIO14 and GPIO15 to mode ALT5.
 *
 * Similar to configuring the mini UART,
 * we use DMA to write to the GPIO controller's
 * registers.
 * Section 5.2. Register View
 * gives us the addresses of the registers we need.
 */

// Note this is defined as 0x7e200000 in the datasheet
// but this is a legacy master address, we subtract 0x7e000000
// and add the low peripheral base address, as described in
// Section 1.2.4 Legacy Master Addresses
#define GPIO_BASE (LOW_PERIPHERAL_BASE + 0x200000)
#define GPFSEL1 (GPIO_BASE + 0x04)
#define GPIO_ALT5_VALUE 0b010
// Note that not all GPIO pins have 5 modes, so some fields are 2 bit
// and some are 3 bit
#define GPIO_PIN_14_SHIFT 12 // bits 12 - 14 are for GPIO14
#define GPIO_PIN_15_SHIFT 15 // bits 15 - 17 are for GPIO15
#define GPIO_PIN_14_MASK (0b111 << GPIO_PIN_14_SHIFT)
#define GPIO_PIN_15_MASK (0b111 << GPIO_PIN_15_SHIFT)

#define GPIO_PUP_PDN_CNTRL_REG0 (GPIO_BASE + 0xE4)
#define GPIO_RESISTOR_SELECT_NONE 0b00
#define GPIO_PUP_PDN_CNTRL_GPIO14_SHIFT 28
#define GPIO_PUP_PDN_CNTRL_GPIO15_SHIFT 30
#define GPIO_PUP_PDN_CNTRL_GPIO14_MASK (0b11 << GPIO_PUP_PDN_CNTRL_GPIO14_SHIFT)
#define GPIO_PUP_PDN_CNTRL_GPIO15_MASK (0b11 << GPIO_PUP_PDN_CNTRL_GPIO15_SHIFT)

// We could do a smart thing here where we calculate the register
// bank for a given pin, then bit shift to find the correct bit mask
// and then take as input and output 3 bit integers. But that is complex
// and for now, we only care about two pins

__attribute__((noinline))
void pull_uart_pins_low() {
	// First we will set the bits we want to mask out to 1
	unsigned int pup_pdn_mask = GPIO_PUP_PDN_CNTRL_GPIO14_MASK |
				     GPIO_PUP_PDN_CNTRL_GPIO15_MASK;
	// Now we will flip all the bits, so only the bits we _don't_ want are
	// set to 1
	pup_pdn_mask = ~pup_pdn_mask;

	// Finally we can AND this with the value we got from the DMA read
	// to clear the bits we want
	unsigned int pup_pdn_cntrl_reg0 = get_register(GPIO_PUP_PDN_CNTRL_REG0);
	pup_pdn_cntrl_reg0 = pup_pdn_cntrl_reg0 & pup_pdn_mask;

	// We can now set the values. We want GPIO_RESISTOR_SELECT_NONE
	// in both fields
	pup_pdn_cntrl_reg0 |= (GPIO_RESISTOR_SELECT_NONE << GPIO_PUP_PDN_CNTRL_GPIO14_SHIFT) |
			      (GPIO_RESISTOR_SELECT_NONE << GPIO_PUP_PDN_CNTRL_GPIO15_SHIFT);

	// Now we can write the new value back to the register
	set_register(GPIO_PUP_PDN_CNTRL_REG0, pup_pdn_cntrl_reg0);
}

__attribute__((noinline))
void initialise_gpio() {
	// First we need to put the resistors for the two
	// GPIO pins into the None state
	// We want to clear the bits for GPIO14 and GPIO15
	pull_uart_pins_low();
	
	// Now we need to set the bits for GPIO14 and GPIO15 to GPIO_ALT5_VALUE
	// We will do this the same as above
	// First we need to read the current value of GPFSEL1
	unsigned int current_value = get_register(GPFSEL1);

	unsigned int gpfsel_mask = GPIO_PIN_14_MASK | GPIO_PIN_15_MASK;
	gpfsel_mask = ~gpfsel_mask;
	current_value &= gpfsel_mask;

	// Now the bits we want to change are masked out, we can set
	// the new values
	current_value |= (GPIO_ALT5_VALUE << GPIO_PIN_14_SHIFT) |
			 (GPIO_ALT5_VALUE << GPIO_PIN_15_SHIFT);

	// Now we can write the new value back to GPFSEL1
	set_register(GPFSEL1, current_value);
}

void uart_init();
void gpio_useAsAlt5(unsigned int pin_number);
__attribute__((noinline))
void initialise_mini_uart() {
	// Time to set up the mini UART controller!
	// We will do this by performing DMA writes to
	// the registers we defined above.
	
	// First we need to enable the mini UART controller
	// We need to do this so the controller will accept
	// out inputs
	set_register(AUX_ENABLES, 0b1);
	// Now we disable the interrupts, we will be polling
	set_register(AUX_MU_IER_REG, 0);
	// We're not yet ready to handle any input or output
	// so we will disable the transmitter and receiver
	set_register(AUX_MU_CNTL_REG, 0);

	// Now that we're safe from unexpected inputs, we
	// can set the data format. We will use 8 bits,
	// sp we set the 0th bit to 1.
	set_register(AUX_MU_LCR_REG, 0b1); 

	// The UART RTS line is high so we need to set
	// the 1st bit to 0.
	set_register(AUX_MU_MCR_REG, 0);

	// Disable interrupts for transmit and receive
	set_register(AUX_MU_IER_REG, 0);

	// Clear anything that is currently in the queue
	set_register(AUX_MU_IIR_REG, 0b110);

	// Set the baud rate to 115200
	// Note we use our macro to calculate the value
	// based on our clock rate
	set_register(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));

	// We need to adjust the GPIO pins to ensure we are
	// in a correct state before enabling the UART controller
	//initialise_gpio();
	gpio_useAsAlt5(14);
    	gpio_useAsAlt5(15);


	// Finally! We can enable transmission and receiving!
	set_register(AUX_MU_CNTL_REG, 0b11);
}

__attribute__((always_inline))
unsigned int mini_uart_ready_for_write() {
	return (get_register(AUX_MU_LSR_REG) & LSR_MASK_TX_READY) > 0;
}

void mini_uart_write_byte(unsigned char byte) {
	// We need to wait until the UART controller is ready
	// to accept our byte
	while (!mini_uart_ready_for_write()) {
		// Spin until the queue is emptied
	}
	// Now we can write our byte to the IO register
	// and the UART controller will add it to its queue
	set_register(AUX_MU_IO_REG, byte);
}

void mini_uart_write_string(char* string) {
	// We will write each character in the string
	// until we reach the null terminator
	// Note that we don't use strlen (we don't have it!)
	while (*string != '\0') {
		mini_uart_write_byte(*string);
		string++;
	}
}

/*
 * Now we can implement the entry point of our bootloader!
 * We can't have any arguments or return value unless we update
 * our shellcode in entry.s to pass arguments or receive
 * a return value.
 */
void bootloader_entry(void) {
	/*
	 * Make sure that you handle .rodata.* in your linker script!
	 * If you are having trouble, disassemble the kernel8.elf file
	 * and make sure this is in the right place!
	 */
	char message[] = "Hello chat! This is a working bootloader!\n\r";
	// If you don't want to handle the .rodata section, you can instead do
	// something like this. Try it and disassemble the kernel8.elf file.
	// You will see that the string is constructed with a series of
	// MOV instructions instead!
	// char message[] = {'H', 'e', 'l', 'l', 'o', ' ', 'c', 'l', 'a', 's', 's', '!', '\n', '\r', '\0'};
	
	// We need to initialise the mini UART controller. I hope you created a stack in entry.s
	// so we can call functions!
	initialise_mini_uart();

	// Now we can write our message to the UART controller
	while (1) {
		mini_uart_write_string(message);
	}

	// Remember we can't return from here. There's nothing to return to other than
	// our shellcode in entry.s.
}
