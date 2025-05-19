#include "conversor_cores.h"


uint16_t referenciaHSV[4][3];

HSV rgb_to_hsv(RGB in) 
{
    HSV out;
    // converte as componente para faixa de 0 - 100
    float r = in.r / 255.0f;
    float g = in.g / 255.0f;
    float b = in.b / 255.0f;

    // Encontra o valor máximo e mínimo entre r, g e b
    float max = r;
    if (g > max) max = g;
    if (b > max) max = b;

    // Encontra o valor mínimo entre r, g e b
    float min = r;
    if (g < min) min = g;
    if (b < min) min = b;

    // Calcula a diferença entre o valor máximo e mínimo
    float delta = max - min;

    // Define o valor (v) como o valor máximo
    out.v = max * 100.0f; // Valor (0–100)

    if (max == 0)  // Se max for 0, a saturação e matiz são indefinidas
    {
        out.s = 0.0f;
        out.h = 0.0f;
    } 
    else    // caso contrário, calcula a saturação e matiz
    {
        out.s = (delta / max) * 100.0f; // caucula s fazendo a proporção entre a diferença e o valor máximo

        if (delta == 0)   // Se não houver diferença, a matiz é indefinida
        {
            out.h = 0.0f;
        } 
        else if (max == r && g >= b)   // Se o vermelho é o máximo e o verde é maior ou igual ao azul
        {
            out.h = 60 * ((g - b) / delta);  // Calcula h cal
        } 
        else if (max == r && g < b)   // Se o vermelho é o máximo e o verde é menor que o azul
        {
            out.h = 60 * ((g - b) / delta);
        } 
        else if (max == r && g < b)
        {
            out.h = 0;
        } 
        else if (max == r) 
        {
            out.h = 60 * fmod(((g - b) / delta), 6);
        } 
        else if (max == g) 
        {
            out.h = 60 * (((b - r) / delta) + 2);
        } 
        else 
        {
            out.h = 60 * (((r - g) / delta) + 4);
        }

        if (out.h < 0)
        {
            out.h += 360;
        }
    }

    return out;
}

RGB hsv_to_rgb(HSV in) 
{
    RGB out;
    float s = in.s / 100.0f;
    float v = in.v / 100.0f;

    float c = v * s;
    float x = c * (1 - fabs(fmod(in.h / 60.0f, 2) - 1));
    float m = v - c;

    float r_prime, g_prime, b_prime;

    if (in.h < 60) 
    {
        r_prime = c; 
        g_prime = x; 
        b_prime = 0;
    } 
    else if (in.h < 120) 
    {
        r_prime = x; 
        g_prime = c; 
        b_prime = 0;
    } 
    else if (in.h < 180) 
    {
        r_prime = 0; 
        g_prime = c; 
        b_prime = x;
    } 
    else if (in.h < 240) 
    {
        r_prime = 0; 
        g_prime = x; 
        b_prime = c;
    } 
    else if (in.h < 300) 
    {
        r_prime = x; 
        g_prime = 0; 
        b_prime = c;
    } 
    else 
    {
        r_prime = c; 
        g_prime = 0; 
        b_prime = x;
    }

    out.r = (unsigned char)((r_prime + m) * 255);
    out.g = (unsigned char)((g_prime + m) * 255);
    out.b = (unsigned char)((b_prime + m) * 255);

    return out;
}

float hueDistance(uint16_t h1, uint16_t h2) {
    int dh = abs((int)h1 - (int)h2);
    return min(dh, 360 - dh);
  }
  
  float hsvDistance(uint16_t h1, uint16_t s1, uint16_t v1,
                    uint16_t h2, uint16_t s2, uint16_t v2) {
    float dh = hueDistance(h1, h2) / 180.0;
    float ds = (float)(s1 - s2) / 100.0;
    float dv = (float)(v1 - v2) / 100.0;
    return sqrt(dh * dh + ds * ds + dv * dv);
  }
  
  int classificar(uint16_t h, uint16_t s, uint16_t v) {
    int maisProximo = 0;
    float menorDistancia = 9999;
  
    for (int i = 0; i < 4; i++) {
      float dist = hsvDistance(h, s, v,
                               referenciaHSV[i][0],
                               referenciaHSV[i][1],
                               referenciaHSV[i][2]);
      if (dist < menorDistancia) {
        menorDistancia = dist;
        maisProximo = i;
      }
    }
    return maisProximo;
  }