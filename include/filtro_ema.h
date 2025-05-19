#ifndef __FILTRO_EMA_H__
#define __FILTRO_EMA_H__
#include <Arduino.h>

class FiltroEMA 
{
  private:
    float valorSuavizado;
    float alpha;

  public:
    FiltroEMA(float alphaInicial = 0.1, float valorInicial = 0.0);
    
    uint8_t atualizar(uint8_t novaLeitura);
    float getValor();
    void setAlpha(float novoAlpha);
    float getAlpha();
};

#endif


