#include "i2c.h"
#include <Wire.h>
#include <Arduino.h>

void i2c_init(void) 
{
	Wire.begin(SDA_PIN, SCL_PIN);  // Inicia I2C com pinos definidos
}

uint16_t i2c_read_word(uint8_t address, uint8_t command, uint8_t reg) 
{
    uint16_t data = 0;
    Wire.beginTransmission(address);
    Wire.write(command | reg);
    Wire.endTransmission();
    Wire.requestFrom(address, (uint8_t)2);
    if (Wire.available() == 2) {
        data = Wire.read() << 8; // Read high byte
        data |= Wire.read();     // Read low byte
    }
    return data;
}

void i2c_write_word(uint8_t address, uint8_t reg, uint16_t data) 
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((data >> 8) & 0xFF); // Write high byte
    Wire.write(data & 0xFF);        // Write low byte
    Wire.endTransmission();
}

void i2c_write_byte(uint8_t address, uint8_t command, uint8_t reg, uint8_t data) 
{
    Wire.beginTransmission(address);
    Wire.write(command | reg);
    Wire.write(data);
    Wire.endTransmission();
}

uint8_t i2c_read_byte(uint8_t address, uint8_t reg) 
{
    uint8_t data = 0;
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(address, (uint8_t)1);
    if (Wire.available() == 1) {
        data = Wire.read();
    }
    return data;
}



