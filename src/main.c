// Basic test for I2C protocol implementation
// Written by Leon Krieg <info@madcow.dev> 

#include <avr/io.h>
#include <util/delay.h>

#define UNUSED(sym) (void)(sym)

static void I2C_Init(void);
static void I2C_Start(char addr, char mode);
static void I2C_Write(char data);
static void I2C_Wait_ACK(void);
static char I2C_Read_ACK(void);
static char I2C_Read_NOACK(void);
static void I2C_Stop(void);

int main(void)
{
	unsigned short lx;
	unsigned char hi, lo;

	// Excerpts below are taken from the BH1750FVI datasheet:
	// https://www.mouser.com/datasheet/2/348/bh1750fvi-e-186247.pdf

	// TODO: Wait 1us reset term between VCC and DVI power supply? (Page 6)
	// TODO: What do they mean with "necessary on power supply sequence"?
	// Asynchronous reset: All registers are reset. It is necessary on
	// power supply sequence. Please refer "Timing chart for VCC and DVI
	// power supply sequence". It is power down mode during DVI = 'L'.

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
	I2C_Wait_ACK();
	I2C_Write(0x10);
	I2C_Wait_ACK();
	I2C_Stop();

	// 2. Wait 180ms to complete 1st measurement

	_delay_ms(180);

	// 3. Read measurement result
	// ST | 0100011 | 1 | <ACK> | <HI[15:8]> | ACK | <LO[7:0]> | NOACK | SP

	I2C_Start(0x23, 1);
	I2C_Wait_ACK();
	hi = I2C_Read_ACK();
	lo = I2C_Read_NOACK();
	I2C_Stop();

	// 4. Combine bytes and calculate
	// TODO: Not sure if this is correct.
	// How to calculate when the data High Byte is "10000011" and
	// Low Byte is "10010000" (215 + 29 + 28 + 27 + 24) / 1.2 = 28067 lx

	lx = (((unsigned short) hi << 8) | (unsigned char) lo) / 1.2;
	UNUSED(lx);
}

static void I2C_Init(void)
{
}

static void I2C_Start(char addr, char mode)
{
	UNUSED(addr);
	UNUSED(mode);
}

static void I2C_Wait_ACK(void)
{
	// Received: Short blink
	// Bail-out: Keep LED lit
}

static char I2C_Read_ACK(void)
{
	return 0;
}

static char I2C_Read_NOACK(void)
{
	return 0;
}

static void I2C_Write(char data)
{
	UNUSED(data);
}

static void I2C_Stop(void)
{
}
