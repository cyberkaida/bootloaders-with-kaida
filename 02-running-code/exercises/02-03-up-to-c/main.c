#include "uart.h"
#include "gpio.h"

/**
 * @brief This function is the entry point of the bootloader program.
 * It's called by the shellcode in `entry.s`
 */
void bootloader_main(void)
{
    initialise_uart();
    while(1) {
	    uart_send_string("Hello Chat!\n");
    }
}
