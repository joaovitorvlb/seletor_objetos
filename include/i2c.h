#ifndef __I2C_H__
#define __I2C_H__

#include <Arduino.h>

#define SDA_PIN 23  // Pino SDA do ESP32
#define SCL_PIN 22  // Pino SCL do ESP32

void i2c_init(void);
uint16_t i2c_read_word(uint8_t address, uint8_t command, uint8_t reg);
void i2c_write_word(uint8_t address, uint8_t reg, uint16_t data);
uint8_t i2c_read_byte(uint8_t address, uint8_t reg);
void i2c_write_byte(uint8_t address, uint8_t command, uint8_t reg, uint8_t data);


#endif