// https://datasheets.raspberrypi.com/bcm2711/bcm2711-peripherals.pdf

#define LEGACY_PERIPHERAL_BASE 0x7E000000
#define LOW_PERIPHERAL_BASE 0xFE000000
#define GPFSEL1         (LOW_PERIPHERAL_BASE+0x200000)

//#define AUX_UART_CLOCK 500000000
#define AUX_MU_BAUD(baud) ((AUX_UART_CLOCK/(baud*8))-1)

// Convert from legacy address to low address
// 1.2.4. Legacy master addresses
// The 7e addresses are from the document, we need to use low address instead
#define AUX_REG_BASE (LOW_PERIPHERAL_BASE + 0x215000)

// Get the mini uart
enum {
    AUX_BASE        = LOW_PERIPHERAL_BASE + 0x215000,
    AUX_ENABLES     = AUX_BASE + 4,
    AUX_MU_IO_REG   = AUX_BASE + 64,
    AUX_MU_IER_REG  = AUX_BASE + 68,
    AUX_MU_IIR_REG  = AUX_BASE + 72,
    AUX_MU_LCR_REG  = AUX_BASE + 76,
    AUX_MU_MCR_REG  = AUX_BASE + 80,
    AUX_MU_LSR_REG  = AUX_BASE + 84,
    AUX_MU_CNTL_REG = AUX_BASE + 96,
    AUX_MU_BAUD_REG = AUX_BASE + 104,
    AUX_UART_CLOCK  = 500000000,
    UART_MAX_QUEUE  = 16 * 1024
};
#define LSR_MASK_TX_READY 0b100000
#define LSR_MASK_RX_READY 0b000001
// AUX_MU_IO_REG is defined in 2.2.2. Mini UART register details


void set_register(unsigned long reg, unsigned int value) {
    *(volatile unsigned int*)reg = value;
}

unsigned int get_register(unsigned long reg) {
    return *(volatile unsigned int*)reg;
}

void wait_some_cycles(int minimum_cycles_to_wait) {
    for (int i = 0; i < minimum_cycles_to_wait; i++) {
	asm volatile("nop");
    }
}


unsigned int gpio_call(unsigned int pin_number, unsigned int value, unsigned int base, unsigned int field_size, unsigned int field_max) {
    unsigned int field_mask = (1 << field_size) - 1;
  
    if (pin_number > field_max) return 0;
    if (value > field_mask) return 0; 

    unsigned int num_fields = 32 / field_size;
    unsigned int reg = base + ((pin_number / num_fields) * 4);
    unsigned int shift = (pin_number % num_fields) * field_size;

    unsigned int curval = get_register(reg);
    curval &= ~(field_mask << shift);
    curval |= value << shift;
    set_register(reg, curval);

    return 1;
}

enum {
    PERIPHERAL_BASE = 0xFE000000,
    GPFSEL0         = PERIPHERAL_BASE + 0x200000,
    GPSET0          = PERIPHERAL_BASE + 0x20001C,
    GPCLR0          = PERIPHERAL_BASE + 0x200028,
    GPPUPPDN0       = PERIPHERAL_BASE + 0x2000E4
};

enum {
    GPIO_MAX_PIN       = 53,
    GPIO_FUNCTION_ALT5 = 2,
};

enum {
    Pull_None = 0,
};

unsigned int gpio_set     (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPSET0, 1, GPIO_MAX_PIN); }
unsigned int gpio_clear   (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPCLR0, 1, GPIO_MAX_PIN); }
unsigned int gpio_pull    (unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPPUPPDN0, 2, GPIO_MAX_PIN); }
unsigned int gpio_function(unsigned int pin_number, unsigned int value) { return gpio_call(pin_number, value, GPFSEL0, 3, GPIO_MAX_PIN); }

void gpio_useAsAlt5(unsigned int pin_number) {
    gpio_pull(pin_number, Pull_None);
    gpio_function(pin_number, GPIO_FUNCTION_ALT5);
}

void uart_init() {
    set_register(AUX_ENABLES, 1); //enable UART1
    set_register(AUX_MU_IER_REG, 0);
    set_register(AUX_MU_CNTL_REG, 0);
    set_register(AUX_MU_LCR_REG, 3); //8 bits
    set_register(AUX_MU_MCR_REG, 0);
    set_register(AUX_MU_IER_REG, 0);
    set_register(AUX_MU_IIR_REG, 0xC6); //disable interrupts
    set_register(AUX_MU_BAUD_REG, AUX_MU_BAUD(115200));
    gpio_useAsAlt5(14);
    gpio_useAsAlt5(15);
    set_register(AUX_MU_CNTL_REG, 3); //enable RX/TX
}

unsigned int uart_isWriteByteReady() {
	return get_register(AUX_MU_LSR_REG) & 0x20;
}

void uart_writeByteBlockingActual(unsigned char ch) {
    while (!uart_isWriteByteReady()); 
    set_register(AUX_MU_IO_REG, (unsigned int)ch);
}


// We cannot have any arguments unless we update
// the calling convention setup in `entry.s`
void main() {
    // Make sure you handle .rodata.* in your linker script!
    char str[] = "Hello chat!\n";

    uart_init();

    while (1) {
	char* buffer = str;
	while (*buffer != '\0') {
            if (*buffer == '\n') uart_writeByteBlockingActual('\r');
            uart_writeByteBlockingActual(*buffer++);
	}
    }

    while (1); // never return

    return;
}
