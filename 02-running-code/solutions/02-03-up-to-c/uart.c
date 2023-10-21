#include "uart.h"
#include "gpio.h"
void initialise_uart(void) {
	// Time to set up the mini UART controller!
	// We will do this by performing DMA writes to
	// the registers we defined above.
	
	// First we need to enable the mini UART controller
	// We need to do this so the controller will accept
	// out inputs
	register_set(AUX_ENABLES, 0b1);
	// Disable interrupts, we will be polling
	register_set(AUX_MU_IER_REG, 0b0);
	// Disable the UART TX and RX
	register_set(AUX_MU_CNTL_REG, 0b0);
	// Now that we're safe from unexpected inputs, we
	// can set the data format. We will use 8 bits,
	// so we set the 0th bit to 1. NOTE: We use 0x3
	// here. This is because the documentation is wrong...
	register_set(AUX_MU_LCR_REG, 0b11);

	// The UART RTS line is high so we need to set
	// the 1st bit to 0.
	register_set(AUX_MU_MCR_REG, 0b0);

	// Disable interrupts again, we will be polling
	register_set(AUX_MU_IER_REG, 0b0);

	// Clear the current UART queue
	// There is a small FIFO queue in the UART controller
	// we need to clear it, because it is used by the previoud
	// bootloader stages.
	register_set(AUX_MU_IIR_REG, 0xC6);

	// Set the baud rate to 115200
	register_set(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));

	// Now we change the TX and RX pins to the right mode
	gpio_set_pull(14, GPIO_PULL_NONE);
	gpio_set_pull(15, GPIO_PULL_NONE);

	// And change the pins to function ALT5 (miniUART)
	gpio_set_function(14, GPIO_FUNCTION_ALT5);
	gpio_set_function(15, GPIO_FUNCTION_ALT5);

	// Enable the UART TX and RX
	register_set(AUX_MU_CNTL_REG, 0b11);
}

void uart_send_byte(uint8_t byte) {
	// Wait until the UART is ready to send
	while (!(register_get(AUX_MU_LSR_REG) & 0x20)) {
		// Do nothing
	}
	// Send the byte
	register_set(AUX_MU_IO_REG, byte);
}

void uart_send_string(char* str) {
	char* current = str;
	while (*current != '\0') {
		if (*current == '\n') {
			uart_send_byte('\r');
		}
		uart_send_byte(*current);
		current++;
	}
}

