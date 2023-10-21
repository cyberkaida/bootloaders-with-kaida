#include "gpio.h"

uint32_t gpio_calculate_mask(uint32_t field_size) {
	uint32_t mask = 0;
	for (uint32_t i = 0; i < field_size; i++) {
		mask |= (1 << i);
	}
	return mask;
}

uintptr_t gpio_get_register_offset(uint32_t pin, uintptr_t base, uint32_t field_size) {
	// These are bit field, so we need to calculate the size of the register
	// in bytes, then convert to bits, then divide by the field size to get
	// the number of fields
	uint32_t number_of_fields = (sizeof(uint32_t) * CHAR_BITS) / field_size;
	// Calculate the offset from the base register to the register containing
	// the pin we asked for
	uintptr_t register_offset = base + ((pin / number_of_fields) * sizeof(uint32_t));

	return register_offset;
}

uint32_t gpio_get_register_shift(uint32_t pin, uint32_t field_size) {
	// These are bit field, so we need to calculate the size of the register
	// in bytes, then convert to bits, then divide by the field size to get
	// the number of fields
	uint32_t number_of_fields = (sizeof(uint32_t) * CHAR_BITS) / field_size;
	// Calculate the shift from the start of the register to the start of the
	// field we asked for
	uint32_t register_shift = (pin % number_of_fields) * field_size;

	return register_shift;
}

void gpio_set_function(uint32_t pin, uint32_t function) {
	// The field size is 3 bits
	// This is in the manual in section 5.2
	// GPFSEL0
	uint32_t field_size = 3;
	uintptr_t base = GPIO_GPFSEL0;
	uintptr_t register_offset = gpio_get_register_offset(pin, base, field_size);
	uint32_t register_shift = gpio_get_register_shift(pin, field_size);
	uint32_t mask = gpio_calculate_mask(field_size);

	uint32_t current_value = register_get(register_offset);
	// Clear the current value from the pin field
	current_value &= ~(mask << register_shift);
	// Set the new value in the pin field
	current_value |= function << register_shift;
	// Write the new value to the register
	register_set(register_offset, current_value);
}

void gpio_set_pull(uint32_t pin, uint32_t pull) {
	// The field size is 2 bits
	// This is in the manual in section 5.6
	// GPPUPPDN0
	uint32_t field_size = 2;
	uintptr_t base = GPIO_PUP_PDN_CNTRL_REG0;
	uintptr_t register_offset = gpio_get_register_offset(pin, base, field_size);
	uint32_t register_shift = gpio_get_register_shift(pin, field_size);
	uint32_t mask = gpio_calculate_mask(field_size);

	uint32_t current_value = register_get(register_offset);
	// Clear the current value from the pin field
	current_value &= ~(mask << register_shift);
	// Set the new value in the pin field
	current_value |= pull << register_shift;
	// Write the new value to the register
	register_set(register_offset, current_value);
}

void register_set(uintptr_t reg, uint32_t value) {
	*(volatile uint32_t *)reg = value;
}

uint32_t register_get(uintptr_t reg) {
	return *(volatile uint32_t *)reg;
}

