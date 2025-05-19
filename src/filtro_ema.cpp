#include <Arduino.h>
#include "filtro_ema.h"

FiltroEMA::FiltroEMA(float alphaInicial, float valorInicial) 
{
    alpha = alphaInicial;
    valorSuavizado = valorInicial;
}
  
uint8_t FiltroEMA::atualizar(uint8_t novaLeitura) 
{
    valorSuavizado = alpha * novaLeitura + (1.0 - alpha) * valorSuavizado;
    return (uint8_t)valorSuavizado;
}

float FiltroEMA::getValor() 
{
    return valorSuavizado;
}

void FiltroEMA::setAlpha(float novoAlpha) 
{
    if (novoAlpha >= 0.0 && novoAlpha <= 1.0) 
    {
        alpha = novoAlpha;
    }
}

float FiltroEMA::getAlpha() 
{
    return alpha;
}
