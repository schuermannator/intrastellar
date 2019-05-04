/*
 * ps2.c
 *
 *  Created on: Apr 28, 2019
 *      Author: Zach
 */

#include "ps2.h"

void ps2_spi_setup() {
	// 96 MHz / 8 => 12 MHz SPI clock
	//PCLKSEL0 |= (3<<16);  CANNOT DO THIS

	// 12 MHz / 48 => 250kHz SCK
	//S0SPCCR |= 0x30;
	S0SPCCR = 0x50;

	// phase
	S0SPCR |= (1<<3);

	// active low clock
	S0SPCR |= (1<<4);

	// operate in master mode
	S0SPCR |= (1<<5);

	// LSB first
	S0SPCR |= (1<<6);
	wait_us(1);
}

int read_controller() {
	int controller = 0;
	volatile int status;
	int read;

	ps2_spi_setup();

	// new packet assert SS
	FIO0PIN &= ~(1<<9);

	//// START HEADER ////
	S0SPDR = 0x01;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read0 = S0SPDR; //FF
	read = S0SPDR;

	S0SPDR = 0x42;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read1 = S0SPDR; //FF
	read = S0SPDR;

	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read2 = S0SPDR; //41
	read = S0SPDR;

	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read3 = S0SPDR; //5A
	read = S0SPDR;
	//// END HEADER ////

	//// DIGITAL ////
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read4 = S0SPDR; //0xFF -> 0xEF up pad
	read = S0SPDR;
	controller |= (read ^ 0xff) << 24;

	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read5 = S0SPDR; //0xFF unpressed -> 7F square
	read = S0SPDR;
	controller |= (read ^ 0xff) << 16;
	//// END DIGITAL ////

	//////UNUSED///////
	//// RIGHT JOY ////
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read6 = S0SPDR; //0x83 unpressed
	read = S0SPDR;
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read7 = S0SPDR; //0x83 unpressed
	read = S0SPDR;
	//// END RIGHT ////
	///////////////////

	//// LEFT JOY ////
	// X
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read8 = S0SPDR; //0x7C unpressed, 0x00 left, 0xff right
	read = S0SPDR;
	controller |= read << 8;
	// Y
	S0SPDR = 0x00;
	while((S0SPSR & (1<<7)) == 0) {}
	status = S0SPSR;
	//int read9 = S0SPDR; //0x6E unpressed 0x00 top, 0xff bottom
	read = S0SPDR;
	controller |= read;
	//// END LEFT ////

	// de-assert SS
	FIO0PIN |= (1<<9);

	return controller;
}

