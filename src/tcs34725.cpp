#include "tcs34725.h"
#include <Arduino.h>
#include <Wire.h>
#include "i2c.h"
#include "filtro_ema.h"

FiltroEMA filtro_r;
FiltroEMA filtro_g;
FiltroEMA filtro_b;


void tcs34725_init(uint8_t gain, uint8_t integration_time, uint8_t mode)
{
    i2c_write_byte(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_CONTROL, gain);
    i2c_write_byte(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_ATIME, integration_time);
    i2c_write_byte(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_ENABLE, mode);
} 

TCS34725_data_t tcs34725_read_data(void)
{
    TCS34725_data_t data;
    uint16_t c, r, g, b;
    uint8_t r_bruto, g_bruto, b_bruto;

    filtro_r.setAlpha(0.9);
    filtro_g.setAlpha(0.9);
    filtro_b.setAlpha(0.9);

    c = i2c_read_word(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_CDATAL);
    r = i2c_read_word(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_RDATAL);
    g = i2c_read_word(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_GDATAL);
    b = i2c_read_word(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_BDATAL);

    // Normalize the values
    r_bruto = (uint8_t)((r * 255) / c);
    g_bruto = (uint8_t)((g * 255) / c);
    b_bruto = (uint8_t)((b * 255) / c);

    // Aplica o filtro EMA
    data.r_norm = filtro_r.atualizar(r_bruto);
    data.g_norm = filtro_g.atualizar(g_bruto);
    data.b_norm = filtro_b.atualizar(b_bruto);
    
    return data;
}

TCS34725RawData tcs34725_raw_data(void)
{
    TCS34725RawData data;
    data.r = i2c_read_word(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_RDATAL);
    data.g = i2c_read_word(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_GDATAL);
    data.b = i2c_read_word(TCS34725_ADDRESS, TCS34725_COMMAND, TCS34725_BDATAL);

    return data;
}
RGB tcs34725_normalizar(TCS34725RawData data, TCS34725RawData preto, TCS34725RawData branco)
{
    RGB cor;

    cor.r = normalizar(data.r, preto.r, branco.r);
    cor.g = normalizar(data.g, preto.g, branco.g);
    cor.b = normalizar(data.b, preto.b, branco.b);

    return cor;
}

uint8_t normalizar(uint16_t leitura, uint16_t preto, uint16_t branco) 
{
    if (branco == preto)
    {
        return 0;
    }
    
    int32_t valor = ((int32_t)(leitura - preto) * 255) / (branco - preto);
    valor = constrain(valor, 0, 255);
    return (uint8_t)valor;
  }