/*
 * eeprom.h
 *
 *  Created on: Apr 28, 2019
 *      Author: Zach
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "intrastellar.h"

void i2c_start();
void i2c_write(int data);
int i2c_read();
void i2c_stop();
void eep_setup();
void eep_write(uint8_t data);
int eep_read();

#endif /* EEPROM_H_ */
