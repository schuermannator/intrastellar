/*
 * eeprom.c
 *
 *  Created on: Apr 28, 2019
 *      Author: Zach
 */

#include "eeprom.h"

/*
 * Starts I2C communication
 * @param: void
 * @return: void
 */
void i2c_start() {
	I2C1CONSET = (1<<3); // ****
	I2C1CONSET = (1<<5); // SR style flip flop (this is readable)
	I2C1CONCLR = (1<<3); // cannot read from here -- no OR's etc
	while(((I2C1CONSET >> 3) & 1) == 0) {} /////////STUCK HERE
	I2C1CONCLR = (1<<5); // write to clear register
}

/*
 * write data to I2C bus
 * @param: int data, to be written to bus
 * @return: void
 */
void i2c_write(int data) {
	I2C1DAT = data;
	I2C1CONCLR = (1<<3);
	while(((I2C1CONSET >> 3) & 1) == 0) {}
}

/*
 * Read from I2C bus
 * @param: void
 * @return: int - bitstream read from the bus
 */
int i2c_read() {
	I2C1CONCLR = (1<<3);
	while(((I2C1CONSET >> 3) & 1) == 0) {}
	int data = I2C1DAT;
	return data;
}

/*
 * Stop I2C communication
 * @param: void
 * @return: void
 */
void i2c_stop() {
	I2C1CONSET = (1<<4);
	I2C1CONCLR = (1<<3);
	while((I2C1CONSET >> 4) & 1) {} // wait if == 1
}

void eep_setup() {
	// setup pins P0.19/P0.20 as I2C
	PINSEL1 |= (0xf << 6);

	// enable I2C
	I2C1CONSET = 0x40;

	// set duty count to 60 (30 each)
	I2C1SCLH = 120;
	I2C1SCLL = 120;

}

void eep_write(uint8_t data) {
	// I2C test
	i2c_start();
	i2c_write(0b10100000); // first 7 bits are address, last is R/!W -- in our case write
	// check ACK with I2C stat reg
	i2c_write(0); // address high ___-----
	i2c_write(0); // address low (full byte)
	i2c_write(data); // data
	i2c_stop();
}

int eep_read() {
	// I2C test
	i2c_start();
	i2c_write(0b10100000); // write
	// check ACK with I2C stat reg
	i2c_write(0); // address high ___-----
	i2c_write(0); // address low (full byte)
	i2c_start();
	i2c_write(0b10100001); // read
	int data = i2c_read();
	i2c_stop();
	return data;
}
