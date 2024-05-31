// Basic test for I2C protocol implementation
// Written by Leon Krieg <info@madcow.dev> 

#include <avr/io.h>
#include <util/delay.h>

#define UNUSED(sym)  (void)(sym)  // Disable -Wunused warnings
#define BH1750       0x23         // Address of BH1750 light sensor

// https://www.nxp.com/docs/en/user-guide/UM10204.pdf
// https://ez.analog.com/video/w/documents/15352/i2c-protocol-sequence

static void i2c_send(char address, char reg, char data);
static char i2c_read(char address, char reg);
static void set_status(int status);

// TODO: Main update loop
// TODO: Segment displays for feedback
// TODO: Serial interface (later)

int main(void)
{
	// Set PB1 as output
	DDRB |= (1 << DDB1);

	// BH1750 Light sensor:
	// - ADDR=GND:     0x23
	// - ADDR=3.3-5V:  0x5C

	UNUSED(i2c_send);
	UNUSED(i2c_read);

	set_status(0);
}

// TODO: Bail out if NOACK received?
// TODO: Check for possible errors.

static void i2c_send(char addr, char reg, char data)
{
	TWCR = 0xA4;             // Send start condition
	while (!(TWCR & 0x80));  // Wait for acknowledge

	TWDR = addr;             // Set slave address
	TWCR = 0x84;             // Transmit on I2C port
	while (!(TWCR & 0x80));  // Wait for acknowledge

	TWDR = reg;              // Set register address
	TWCR = 0x84;             // Transmit on I2C port
	while (!(TWCR & 0x80));  // Wait for acknowledge

	TWDR = data;             // Set data to be written
	TWCR = 0x84;             // Transmit on I2C port
	while (!(TWCR & 0x80));  // Wait for acknowledge
	TWCR = 0x94;             // Send stop condition
}

// TODO: Bail out if NOACK received?
// TODO: Check for possible errors.

static char i2c_read(char address, char reg)
{
	char data = 0;

	TWCR = 0xA4;             // Send start condition
	while (!(TWCR & 0x80));  // Wait for acknowledge

	TWDR = address;          // Set slave address
	TWCR = 0x84;             // Transmit on I2C port
	while (!(TWCR & 0x80));  // Wait for acknowledge

	TWDR = reg;              // Set register address
	TWCR = 0x84;             // Transmit on I2C port
	while (!(TWCR & 0x80));  // Wait for acknowledge

	TWCR = 0xA4;             // Send start condition
	while (!(TWCR & 0x80));  // Wait for acknowledge

	TWDR = address + 1;      // Set address /w read bit
	TWCR = 0xC4;             // Clear transmit interrupt
	while (!(TWCR & 0x80));  // Wait for acknowledge

	data = TWDR;             // Load data from register
	TWCR = 0x84;             // Send No-Acknowledge
	while (!(TWCR & 0x80));  // Wait for acknowledge
	TWCR = 0x94;             // Send stop condition

	return data;
}

static void set_status(int status)
{
	// Reset status LED state
	PORTB &= ~(1 << PORTB1);

	// Success?
	if (status == 0) {
		for (int i = 0; i < 5; i++) {
			PORTB |= (1 << PORTB1);
			_delay_ms(200);
			PORTB &= ~(1 << PORTB1);
			_delay_ms(200);
		}
		return;
	}

	// Keep LED lit on error
	PORTB |= (1 << PORTB1);
}
