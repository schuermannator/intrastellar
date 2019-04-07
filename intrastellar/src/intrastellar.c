/*
===============================================================================
 Name        : intrastellar.c
 Author      : Zach Schuermann
 Version     :
 Copyright   : Zach Schuermann, 2019
 Description : Intrastellar Retro Game
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

// GPIO
#define FIO0DIR (*(volatile unsigned int *) 0x2009c000)
#define FIO0PIN (*(volatile unsigned int *) 0x2009c014)
#define FIO3DIR (*(volatile unsigned int *) 0x2009c060)
#define FIO3PIN (*(volatile unsigned int *) 0x2009c074)
#define PINSEL0 (*(volatile unsigned int *) 0x4002C000)
#define PINSEL1 (*(volatile unsigned int *) 0x4002C004)

// clock
#define CLKOUTCFG   (*(volatile unsigned int *) 0x400fc1c8)
#define PLL0CON     (*(volatile unsigned int *) 0x400FC080)
#define PLL0CFG     (*(volatile unsigned int *) 0x400FC084)
#define PLL0FEED    (*(volatile unsigned int *) 0x400FC08C)
#define PLL0STAT    (*(volatile unsigned int *) 0x400FC088)
#define CCLKCFG     (*(volatile unsigned int *) 0x400FC104)
#define PCLKSEL0 	(*(volatile unsigned int *) 0x400FC1A8)

// SPI
#define S0SPCCR		(*(volatile unsigned int *) 0x4002000C)
#define S0SPCR 		(*(volatile unsigned int *) 0x40020000)
#define S0SPSR 		(*(volatile unsigned int *) 0x40020004)
#define S0SPDR		(*(volatile unsigned int *) 0x40020008)

/*
 * setup CPU clock for 96 MHz instead of default 4MHz
 * using PLL0 and IRC oscillator
 */
void clock_setup(void) {
	// select P1.27 for clock out and enable
	// PINSEL3 |= (1<<22);
	CLKOUTCFG = (1<<8);

	// bit 0 enable, bit 1 connect
	PLL0CON = 0;
	// PLL0 setup
	// feed sequence - disconnect
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// Generate 288MHz with 36x multiplier
	PLL0CFG = 0x23;
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	//enable
	PLL0CON = 0x1;
	PLL0FEED = 0xAA;
	PLL0FEED = 0x55;

	// Divide by 3 to get 96MHz
	CCLKCFG = 0x2;

	// wait for lock
    while(!((PLL0STAT & (1<<26)) >> 26)) {}

    // connect
    PLL0CON = 0x3;
    PLL0FEED = 0xAA;
    PLL0FEED = 0x55;
}

void gpio_setup() {
	// LED Setup
	FIO0DIR |= (1<<22);
	FIO3DIR |= (3<<25);

	// SPI SS
	FIO0DIR |= (1<<9);
	// port 8?

}

void spi_setup() {
	// 96 MHz / 8 => 12 MHz SPI clock
	PCLKSEL0 |= (3<<16);

	// 12 MHz / 48 MHz => 250kHz SCK
	S0SPCCR |= 0x30;

	// phase
	S0SPCR |= (1<<3);

	// active low clock
	S0SPCR |= (1<<4);

	// operate in master mode
	S0SPCR |= (1<<5);

	// LSB first
	S0SPCR |= (1<<6);

	PINSEL0 |= (3 << 30); // SCK
	PINSEL1 |= (3 << 2); // MISO
	PINSEL1 |= (3 << 4); // MOSI
}

void led(int num) {
	if(num == -1) {
		FIO3PIN |= (1<<25);
		FIO3PIN |= (1<<26);
		FIO0PIN |= (1<<22);
	}
	else if(num == 0) {
		FIO3PIN |= (1<<25);
		FIO3PIN |= (1<<26);
		FIO0PIN &= ~(1<<22); // turn on red
	}
	else if (num == 1) {
		FIO0PIN |= (1<<22); // turn off LED
		FIO3PIN |= (1<<26);
		FIO3PIN &= ~(1<<25); // turn on green
	}
	else {
		FIO0PIN |= (1<<22); // turn off LED
		FIO3PIN |= (1<<25);
		FIO3PIN &= ~(1<<26); // turn on blue
	}
}

/*
 * clock test - run for 5 seconds - default 4MHz counts to ~2MM,
 * 96 MHz counts to ~49MM
 */
void clock_test() {
	// Force the counter to be placed into memory
	volatile static int i = 0;

	// Enter an infinite loop, just incrementing a counter
	while(1) {
		i++ ;
	}
}

void wait_ticks(int count) {
    volatile int ticks = count;
    while (ticks > 0)
        ticks--;
}

int read_controller() {
	int controller = 0;
	int status;
	int read;

	// new packet assert SS
	FIO0PIN &= ~(1<<9);

	//// START HEADER ////
	S0SPDR = 0x01;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read0 = S0SPDR; //FF

	S0SPDR = 0x42;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read1 = S0SPDR; //FF

	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read2 = S0SPDR; //41

	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read3 = S0SPDR; //5A
	//// END HEADER ////

	//// DIGITAL ////
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read4 = S0SPDR; //0xFF -> 0xEF up pad
	controller |= (read4 ^ 0xff) << 22;

	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read5 = S0SPDR; //0xFF unpressed -> 7F square
	controller |= (read5 ^ 0xff) << 14;
	//// END DIGITAL ////

	//////UNUSED///////
	//// RIGHT JOY ////
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read6 = S0SPDR; //0x83 unpressed
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read7 = S0SPDR; //0x83 unpressed
	//// END RIGHT ////
	///////////////////

	//// LEFT JOY ////
	// X
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read8 = S0SPDR; //0x7C unpressed, 0x00 left, 0xff right
	controller |= read8 << 8;
	// Y
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	int read9 = S0SPDR; //0x6E unpressed 0x00 top, 0xff bottom
	controller |= read9;
	//// END LEFT ////

	// de-assert SS
	FIO0PIN |= (1<<9);

	return controller;
}

int main(void) {
	// setup (green then red after done)
	led(1);
	clock_setup();
	gpio_setup();
	spi_setup();
	led(-1);

	// first 16 bits: digital
	// next byte: left stick X
	// next byte: left stick Y
	while (1) {
		int controller = read_controller();
		int digital = controller >> 16;
		int joy_x = (controller & ~(0xff00)) >> 8;
		int joy_y = (controller & ~(0xff));
		if (digital)
			led(0);
		else
			led(-1);
	}

    return 0;
}
