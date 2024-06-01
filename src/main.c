// Basic test for I2C protocol implementation
// Written by Leon Krieg <info@madcow.dev> 

// https://www.mouser.com/datasheet/2/348/bh1750fvi-e-186247.pdf
// https://www-user.tu-chemnitz.de/~heha/hsn/chm/ATmegaX8.chm/22.htm
// https://www.nongnu.org/avr-libc/user-manual/group__util__twi.html

#include <stdlib.h>
#include <assert.h>
#include <avr/io.h>
#include <util/delay.h>

#define  UNUSED(sym)     (void)(sym)

#define  TW_START        0x08
#define  TW_REP_START    0x10
#define  TW_MT_SLA_ACK   0x18
#define  TW_MT_SLA_NACK  0x20
#define  TW_MT_DATA_ACK  0x28
#define  TW_MR_SLA_ACK   0x40
#define  TW_MR_SLA_NACK  0x48
#define  TW_MR_DATA_ACK  0x50

static void I2C_Init(void);
static void I2C_Start(char addr, char mode);
static void I2C_Stop(void);
static char I2C_Read_ACK(void);
static char I2C_Read_NACK(void);
static void I2C_Write(char data);
static void I2C_Wait_ACK(void);

static void COM_Blink(unsigned int n);
static void COM_Error(void);

int main(void)
{
	unsigned short lx;
	unsigned char hi, lo;

	// Uses PB1 for status LED
	DDRB |= (1 << DDB1);

	// https://www.mouser.com/datasheet/2/348/bh1750fvi-e-186247.pdf:
	// We recommend to use H-Resolution Mode. Measurement time (integration
	// time) of H-Resolution Mode is so long that some kind of noise
	// (including in 50Hz / 60Hz noise) is rejected. And H-Resolution Mode
	// is 1 lx resolution so that it is suitable for darkness (less than 10
	// lx) H-resolution mode2 is also suitable to detect for darkness.

	// 00010000 = 1.0 lx resolution, 120ms
	// 00010001 = 0.5 lx resolution, 120ms
	// 00100011 = 4.0 lx resolution, 16ms

	I2C_Init();

	// BH1750: Enable continous high resolution mode
	// ---------------------------------------------

	// 1. Send "Continuous H-resolution mode" instruction
	// ST | 0100011 | 0 | <ACK> | 00010000 | <ACK> | SP

	I2C_Start(0x23, 0);
	I2C_Write(0x10);
	I2C_Wait_ACK();
	I2C_Stop();

	// 2. Wait 180ms to complete 1st measurement

	_delay_ms(180);

	// 3. Read measurement result
	// ST | 0100011 | 1 | <ACK> | <HI[15:8]> | ACK | <LO[7:0]> | NOACK | SP

	I2C_Start(0x23, 1);
	hi = I2C_Read_ACK();
	lo = I2C_Read_NACK();
	I2C_Stop();

	// 4. Combine bytes and calculate
	// TODO: Not sure if this is correct.
	// NOTE: Yeah should be right... See: (131 * 256 + 144) / 1.2
	// https://www.mouser.com/datasheet/2/348/bh1750fvi-e-186247.pdf:
	// How to calculate when the data High Byte is "10000011" and
	// Low Byte is "10010000" (215 + 29 + 28 + 27 + 24) / 1.2 = 28067 lx
	
	lx = (((unsigned short) hi << 8) | (unsigned char) lo) / 1.2;

	COM_Blink(lx); // Output lux with LED
}

static void I2C_Init(void)
{
	// Set SCL bit rate to 400kHz

	TWSR = 0x00; // TWI status register
	TWBR = 0x0C; // TWI bit rate register
}

static void I2C_Start(char addr, char mode)
{
	unsigned int status;

	assert(mode == 0 || mode == 1);

	// Send start condition

	TWCR = (1 << TWEN)    // Enable TWI
	     | (1 << TWINT)   // Clear interrupt flag
	     | (1 << TWSTA);  // Send start condition

	// Wait until start condition sent

	while ((TWCR & (1 << TWINT)) == 0);
	if ((TWSR & 0xf8) != TW_START)
		COM_Error();

	COM_Blink(1);

	// Send slave address

	addr = addr << 1; // Lowest bit for mode
	TWDR = addr + mode; // Mode: 0=R, 1=W
	TWCR = (1 << TWINT) | (1 << TWEN);

	// Wait until slave address sent

	while ((TWCR & (1 << TWINT)) == 0);
	status = TWSR & 0xF8;

	if ((mode == 0 && status != TW_MT_SLA_ACK) ||
	    (mode == 1 && status != TW_MR_SLA_ACK))
		COM_Error();

	COM_Blink(1);

	UNUSED(addr);
	UNUSED(mode);
}

static void I2C_Stop(void)
{
	// Send stop condition

	TWCR = (1 << TWEN)    // Enable TWI
	     | (1 << TWINT)   // Clear interrupt flag
	     | (1 << TWSTO);  // Send stop condition

	// Wait until stop condition sent

	while (TWCR & (1 << TWSTO));
}

static char I2C_Read_ACK(void)
{
	// Read data and acknowledge

	TWCR = (1 << TWEN)   // Enable TWI
	     | (1 << TWINT)  // Clear interrupt flag
	     | (1 << TWEA);  // Send acknowledgment

	// Wait until data read

	while ((TWCR & (1 << TWINT)) == 0);

	return TWDR;
}

static char I2C_Read_NACK(void)
{
	// Read data, expect last byte

	TWCR = (1 << TWEN)    // Enable TWI
	     | (1 << TWINT);  // Clear interrupt flag

	// Wait until data read

	while ((TWCR & (1 << TWINT)) == 0);

	return TWDR;
}

static void I2C_Write(char data)
{
	TWDR = data;
	TWCR = (1 << TWEN)    // Enable TWI
	     | (1 << TWINT);  // Clear interrupt flag

	while ((TWCR & (1 << TWINT)) == 0);

	if ((TWSR & 0xF8) != TW_MT_DATA_ACK)
		COM_Error();
}

static void I2C_Wait_ACK(void)
{
	unsigned int status;

	while ((TWCR & (1 << TWINT)) == 0);
	status = TWSR & 0xF8;

	if (status != TW_MT_DATA_ACK &&
	    status != TW_MR_DATA_ACK)
		COM_Error();
}

static void COM_Blink(unsigned int n)
{
	_delay_ms(200);
	while (n > 0) {
		PORTB |= (1 << PORTB1);
		_delay_ms(100);
		PORTB &= ~(1 << PORTB1);
		_delay_ms(100);
		n--;
	}
}

static void COM_Error(void)
{
	// Keep status LED lit

	PORTB |= (1 << PORTB1);

	exit(1);
}
