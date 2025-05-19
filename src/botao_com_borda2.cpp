#include <Arduino.h>
#include "botao_com_borda2.h"  

BotaoComBorda2::BotaoComBorda2(volatile uint8_t *port, uint8_t bit) 
{
    this->port = port;  // Armazena o endereço do port
    this->bit = bit;    // Armazena o número do bit
    ultimoEstado = (*port & (1 << bit)) != 0; // Lê o estado inicial do bit
}

bool BotaoComBorda2::caiu() 
{
    bool estado = (*port & (1 << bit)) == 0; // Verifica se o bit está LOW
    bool borda = (ultimoEstado == HIGH && estado == LOW);
    ultimoEstado = estado;
    return borda;
}

bool BotaoComBorda2::subiu() 
{
    bool estado = (*port & (1 << bit)) != 0; // Verifica se o bit está HIGH
    bool borda = (ultimoEstado == LOW && estado == HIGH);
    ultimoEstado = estado;
    return borda;
}

bool BotaoComBorda2::estadoAtual() 
{
    return (*port & (1 << bit)) != 0; // Retorna o estado atual do bit
}