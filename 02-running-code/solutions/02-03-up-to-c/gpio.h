enum {
	// These addresses are defined in section
	// 1.2 of the BCM2711 ARM Peripherals manual
	LEGACY_MASTER_BASE =  0x7E000000,
	LOW_PERIPHERAL_BASE = 0xFE000000,

	// These addresses are defined in section
	// 5.2 of the BCM2711 ARM Peripherals manual
	GPIO_BASE = LOW_PERIPHERAL_BASE + 0x200000,
	GPIO_GPFSEL0 = GPIO_BASE + 0x00,
	GPIO_GPSET0 = GPIO_BASE + 0x1C,
	GPIO_GPCLR0 = GPIO_BASE + 0x28,
	GPIO_PUP_PDN_CNTRL_REG0 = GPIO_BASE + 0xE4,

	AUX_BASE = LOW_PERIPHERAL_BASE + 0x215000,
	AUX_ENABLES = AUX_BASE + 0x04,
	AUX_MU_IO_REG = AUX_BASE + 0x40,
	AUX_MU_IER_REG = AUX_BASE + 0x44,
	AUX_MU_IIR_REG = AUX_BASE + 0x48,
	AUX_MU_LCR_REG = AUX_BASE + 0x4C,
	AUX_MU_MCR_REG = AUX_BASE + 0x50,
	AUX_MU_LSR_REG = AUX_BASE + 0x54,
	AUX_MU_CNTL_REG = AUX_BASE + 0x60,
	AUX_MU_BAUD_REG = AUX_BASE + 0x68,
	// CLOCK_SPEED = 250000000,
	CLOCK_SPEED = 500000000,
};

enum {
	GPIO_FUNCTION_INPUT = 0b000,
	GPIO_FUNCTION_OUTPUT = 0b001,
	GPIO_FUNCTION_ALT0 = 0b100,
	GPIO_FUNCTION_ALT1 = 0b101,
	GPIO_FUNCTION_ALT2 = 0b110,
	GPIO_FUNCTION_ALT3 = 0b111,
	GPIO_FUNCTION_ALT4 = 0b011,
	GPIO_FUNCTION_ALT5 = 0b010,
};

enum {
	GPIO_PULL_NONE = 0b00,
	GPIO_PULL_UP = 0b01,
	GPIO_PULL_DOWN = 0b10,
	GPIO_PULL_RESERVED = 0b11,
};

// This calculation is from 2.2.1 of the manual
#define AUX_MU_BAUD(baud) ((CLOCK_SPEED / (8 * (baud))) - 1)
#define CHAR_BITS 8
typedef unsigned int uint32_t;
typedef unsigned long uintptr_t;
typedef unsigned char uint8_t;


uint32_t gpio_calculate_mask(uint32_t field_size);
uintptr_t gpio_get_register_offset(uint32_t pin, uintptr_t base, uint32_t field_size);
uint32_t gpio_get_register_shift(uint32_t pin, uint32_t field_size);
void gpio_set_function(uint32_t pin, uint32_t function);
void gpio_set_pull(uint32_t pin, uint32_t pull);

/*
 * This function takes a register address,
 * and sets it to a value.
 */
void register_set(uintptr_t reg, uint32_t value);
/*
 * This function takes a register address,
 * and returns its value.
 */
uint32_t register_get(uintptr_t reg);

