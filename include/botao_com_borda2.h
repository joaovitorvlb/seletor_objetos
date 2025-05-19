#ifndef __BOTAO_COM_BORDA2_H__
#define __BOTAO_COM_BORDA2_H__
#include <Arduino.h>

class BotaoComBorda2 
{
private:
    volatile uint8_t *port; // Ponteiro para o endereço do port
    uint8_t bit;            // Número do bit a ser testado
    bool ultimoEstado;      // Último estado do bit

public:
    BotaoComBorda2(volatile uint8_t *port, uint8_t bit); // Construtor
    bool caiu();         // Detecta borda de descida
    bool subiu();        // Detecta borda de subida
    bool estadoAtual();  // Retorna o estado atual do bit
};

  #endif