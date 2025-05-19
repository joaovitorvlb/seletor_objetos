#include <Arduino.h>
#include "botao_com_borda.h"  

BotaoComBorda::BotaoComBorda(uint8_t p) 
{
    pino = p;
    pinMode(pino, INPUT_PULLUP);
    ultimoEstado = digitalRead(pino);
}

bool BotaoComBorda::caiu() 
{
    bool estado = digitalRead(pino);
    bool borda = (ultimoEstado == HIGH && estado == LOW);
    ultimoEstado = estado;
    return borda;
}

bool BotaoComBorda::subiu() 
{
    bool estado = digitalRead(pino);
    bool borda = (ultimoEstado == LOW && estado == HIGH);
    ultimoEstado = estado;
    return borda;
}

bool BotaoComBorda::estadoAtual() 
{
    return digitalRead(pino);
}

