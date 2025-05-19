#ifndef __BOTAO_COM_BORDA_H__
#define __BOTAO_COM_BORDA_H__
#include <Arduino.h>

class BotaoComBorda 
{
	private:
		uint8_t pino;
		bool ultimoEstado;

	public:
		BotaoComBorda(uint8_t p);

		bool caiu();          // Detecta borda de descida (HIGH → LOW)
		bool subiu();         // Detecta borda de subida  (LOW → HIGH)
		bool estadoAtual();   // Retorna o estado atual do pino
};

  #endif