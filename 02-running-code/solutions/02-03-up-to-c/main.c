#include "uart.h"
#include "gpio.h"

void bootloader_main(void)
{
    //uart_init();
    initialise_uart();
    while(1) {
	    uart_send_string("Hello Chat!\n");
    }
}
