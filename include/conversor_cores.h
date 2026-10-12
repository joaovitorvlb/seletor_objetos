#ifndef __CONVERSOR_CORES_H__
#define __CONVERSOR_CORES_H__

#include <Arduino.h>

typedef struct 
{
    uint8_t classificacao;
    uint8_t r; // Hue [0, 360)
    uint8_t g; // Saturation [0, 1]
    uint8_t b; // Value [0, 1]
    uint8_t operacao; // Red [0, 255]
} Pacote_data_t;

typedef struct 
{
    char mensagem[150];
    double distancia;
    int estagio; // Estado da maturação da fruta
} Pacote_resp_t;

typedef struct {
    uint16_t h; // Hue [0, 360)
    uint16_t s; // Saturation [0, 1]
    uint16_t v; // Value [0, 1]
} HSV;

typedef struct {
    uint8_t r; // Red [0, 255]
    uint8_t g; // Green [0, 255]
    uint8_t b; // Blue [0, 255]
} RGB;

extern uint16_t referenciaHSV[4][3];

HSV rgb_to_hsv(RGB in);
RGB hsv_to_rgb(HSV in);

float hueDistance(uint16_t h1, uint16_t h2); 
float hsvDistance(uint16_t h1, uint16_t s1, uint16_t v1,
                uint16_t h2, uint16_t s2, uint16_t v2); 

int classificar(uint16_t h, uint16_t s, uint16_t v);

#endif