/*
 * intrastellar.h
 *
 *  Created on: Apr 28, 2019
 *      Author: Zach
 */

#ifndef INTRASTELLAR_H_
#define INTRASTELLAR_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

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

// I2C
#define I2C1CONSET 	(*(volatile unsigned int *) 0x4005C000)
#define I2C1CONCLR 	(*(volatile unsigned int *) 0x4005C018)
#define I2C1DAT 	(*(volatile unsigned int *) 0x4005C008)
#define I2C1STAT 	(*(volatile unsigned int *) 0x4005C004)
#define I2C1SCLH 	(*(volatile unsigned int *) 0x4005C010)
#define I2C1SCLL 	(*(volatile unsigned int *) 0x4005C014)

// Systick
#define STCTRL		(*(volatile unsigned int *) 0xE000E010)
#define STRELOAD	(*(volatile unsigned int *) 0xE000E014)

#define T0TC 		(*(volatile unsigned int *) 0x40004008)
#define T0TCR 		(*(volatile unsigned int *) 0x40004004)

void wait_ticks(int);
void wait_ms(int);
void wait_us(int);

void clock_test();

int poll();
int dpoll();
int get_controller();
void controller_test();


#endif /* INTRASTELLAR_H_ */
