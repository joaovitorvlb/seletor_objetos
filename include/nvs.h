#ifndef __NVS_H__
#define __NVS_H__

#include <Arduino.h>
#include "tcs34725.h"


void salvar_calibracao_cor(String chave, TCS34725RawData cor);
TCS34725RawData ler_calibracao_cor(String chave);

void salvar_calibracao_sensor(String chave, TCS34725_cfg_t sensor);
TCS34725_cfg_t ler_calibracao_sensor(String chave);

void salvar_wifi_credenciais(String ssid, String password); 
void ler_wifi_credenciais(String &ssid, String &password); 
void salvar_servidor_config(String url, String rota1, String rota2);
void ler_servidor_config(char *url, char *rota1, char *rota2);

#endif