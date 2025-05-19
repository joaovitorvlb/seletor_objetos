#ifndef __TCS34725_H__
#define __TCS34725_H__

#include <Arduino.h>
#include "conversor_cores.h"

#define TCS34725_ADDRESS  0x29  // Endereço I2C do sensor
#define TCS34725_COMMAND  0x80  // Bit de comando para registradores

// Registradores do TCS34725
#define TCS34725_ENABLE   0x00
#define TCS34725_ATIME    0x01
#define TCS34725_CONTROL  0x0F
#define TCS34725_CDATAL   0x14
#define TCS34725_RDATAL   0x16
#define TCS34725_GDATAL   0x18
#define TCS34725_BDATAL   0x1A

typedef struct 
{
    uint8_t r_norm;
    uint8_t g_norm;
    uint8_t b_norm;
} TCS34725_data_t;

typedef struct 
{
    uint8_t modo;
    uint8_t ganho;
    uint8_t tempo;
} TCS34725_cfg_t;

typedef struct 
{
    uint16_t r;
    uint16_t g;
    uint16_t b;
    uint16_t c;
} TCS34725RawData;

void tcs34725_init(uint8_t gain, uint8_t integration_time, uint8_t mode);
TCS34725_data_t tcs34725_read_data(void);
TCS34725RawData tcs34725_raw_data(void);
uint8_t normalizar(uint16_t leitura, uint16_t preto, uint16_t branco);
RGB tcs34725_normalizar(TCS34725RawData data, TCS34725RawData preto, TCS34725RawData branco);

#endif