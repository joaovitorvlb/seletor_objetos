#include "nvs.h"
#include <Preferences.h>
#include <tcs34725.h>
#include <freertos/semphr.h>

// Declare the mutex handle
extern SemaphoreHandle_t xNvsMutex;


Preferences preferences;
Preferences preferences_wifi;


void salvar_calibracao_cor(String chave, TCS34725RawData cor) 
{
    if (xSemaphoreTake(xNvsMutex, portMAX_DELAY)) // Bloqueia até o mutex estar disponível
    {
        preferences.begin("config", false);
        preferences.putUShort((chave + "_r").c_str(), cor.r);
        preferences.putUShort((chave + "_g").c_str(), cor.g);
        preferences.putUShort((chave + "_b").c_str(), cor.b);
        preferences.end();
        xSemaphoreGive(xNvsMutex); // Libera o mutex
    }
}

TCS34725RawData ler_calibracao_cor(String chave) 
{

    TCS34725RawData cor;

    if (xSemaphoreTake(xNvsMutex, portMAX_DELAY)) // Bloqueia até o mutex estar disponível
    {
        preferences.begin("config", false); // Abra em modo de escrita para criar a tag, se necessário
        

        // Verifica e inicializa as tags, se necessário
        cor.r = preferences.getUShort((chave + "_r").c_str(), 0); // Retorna 0 se a tag não existir
        if (cor.r == 0) 
        {
            preferences.putUShort((chave + "_r").c_str(), 0); // Cria a tag com valor padrão
        }

        cor.g = preferences.getUShort((chave + "_g").c_str(), 0);
        if (cor.g == 0) 
        {
            preferences.putUShort((chave + "_g").c_str(), 0);
        }

        cor.b = preferences.getUShort((chave + "_b").c_str(), 0);
        if (cor.b == 0) 
        {
            preferences.putUShort((chave + "_b").c_str(), 0);
        }

        preferences.end(); // Fecha o namespace
        
        xSemaphoreGive(xNvsMutex); // Libera o mutex
    }
    return cor;
}

void salvar_calibracao_sensor(String chave, TCS34725_cfg_t sensor)
{
    if (xSemaphoreTake(xNvsMutex, portMAX_DELAY)) // Bloqueia até o mutex estar disponível
    {
        preferences.begin("config", false);
        preferences.putUChar((chave + "_modo").c_str(), sensor.modo);
        preferences.putUChar((chave + "_ganho").c_str(), sensor.ganho);
        preferences.putUChar((chave + "_tempo").c_str(), sensor.tempo);
        preferences.end();
        xSemaphoreGive(xNvsMutex); // Libera o mutex
    }
}

TCS34725_cfg_t ler_calibracao_sensor(String chave)
{
    TCS34725_cfg_t sensor;
    if (xSemaphoreTake(xNvsMutex, portMAX_DELAY)) // Bloqueia até o mutex estar disponível
    {
        preferences.begin("config", false);
    

        sensor.modo = preferences.getUChar((chave + "_modo").c_str(), 0); // Retorna 0 se a tag não existir
        if (sensor.modo == 0) 
        {
            preferences.putUChar((chave + "_modo").c_str(), 0x03); // Cria a tag com valor padrão
        }

        sensor.ganho = preferences.getUChar((chave + "_ganho").c_str(), 0);
        if (sensor.ganho == 0) 
        {
            preferences.putUChar((chave + "_ganho").c_str(), 0x01);
        }

        sensor.tempo = preferences.getUChar((chave + "_tempo").c_str(), 0);
        if (sensor.tempo == 0) 
        {
            preferences.putUChar((chave + "_tempo").c_str(), 0xF6);
        }

        preferences.end();
        xSemaphoreGive(xNvsMutex); // Libera o mutex
    }
    return sensor;
}

void salvar_wifi_credenciais(String ssid, String password) 
{
    if (xSemaphoreTake(xNvsMutex, portMAX_DELAY)) // Bloqueia até o mutex estar disponível
    {
        preferences_wifi.begin("config", false);
        preferences_wifi.putString("ssid", ssid);
        preferences_wifi.putString("password", password);
        preferences_wifi.end();
        xSemaphoreGive(xNvsMutex); // Libera o mutex
    }
}

void ler_wifi_credenciais(String &ssid, String &password) 
{
    if (xSemaphoreTake(xNvsMutex, portMAX_DELAY)) // Bloqueia até o mutex estar disponível
    {
        if (!preferences_wifi.begin("config", false)) 
        {
            Serial.println("Falha ao iniciar o namespace 'wifi'");
            return;
        }

        ssid = preferences_wifi.getString("ssid", "");
        if (ssid.isEmpty()) 
        {
            Serial.println("Chave 'ssid' não encontrada. Criando com valor padrão.");
            preferences_wifi.putString("ssid", "");
        }

        password = preferences_wifi.getString("password", "");
        if (password.isEmpty()) 
        {
            Serial.println("Chave 'password' não encontrada. Criando com valor padrão.");
            preferences_wifi.putString("password", "");
        }

        preferences_wifi.end();
        xSemaphoreGive(xNvsMutex); // Libera o mutex
    }
}

void salvar_servidor_config(String url, String rota1, String rota2, String rota3)
{
    if (xSemaphoreTake(xNvsMutex, portMAX_DELAY)) // Bloqueia até o mutex estar disponível
    {
        preferences.begin("config", false);
        preferences.putString("servidor_url", url);
        preferences.putString("servidor_rota1", rota1);
        preferences.putString("servidor_rota2", rota2);
        preferences.putString("servidor_rota3", rota3);
        preferences.end();
        xSemaphoreGive(xNvsMutex); // Libera o mutex
    }
}

void ler_servidor_config(char *url, char *rota1, char *rota2, char *rota3)
{
    if (xSemaphoreTake(xNvsMutex, portMAX_DELAY)) // Bloqueia até o mutex estar disponível
    {
        if (!preferences.begin("config", false)) 
        {
            Serial.println("Falha ao iniciar o namespace 'config'");
            return;
        }

        String tempUrl = preferences.getString("servidor_url", "");
        if (tempUrl.isEmpty()) 
        {
            Serial.println("Chave 'servidor_url' não encontrada. Criando com valor padrão.");
            preferences.putString("servidor_url", "exemplo.com");
            tempUrl = "exemplo.com";
        }
        strcpy(url, tempUrl.c_str());

        String tempRota1 = preferences.getString("servidor_rota1", "");
        if (tempRota1.isEmpty()) 
        {
            Serial.println("Chave 'servidor_rota1' não encontrada. Criando com valor padrão.");
            preferences.putString("servidor_rota1", "/rota1");
            tempRota1 = "/rota1";
        }
        strcpy(rota1, tempRota1.c_str());

        String tempRota2 = preferences.getString("servidor_rota2", "");
        if (tempRota2.isEmpty()) 
        {
            Serial.println("Chave 'servidor_rota2' não encontrada. Criando com valor padrão.");
            preferences.putString("servidor_rota2", "/rota2");
            tempRota2 = "/rota2";
        }
        strcpy(rota2, tempRota2.c_str());

        String tempRota3 = preferences.getString("servidor_rota3", "");
        if (tempRota3.isEmpty()) 
        {
            Serial.println("Chave 'servidor_rota3' não encontrada. Criando com valor padrão.");
            preferences.putString("servidor_rota3", "/rota3");
            tempRota3 = "/rota3";
        }
        strcpy(rota3, tempRota3.c_str());

        preferences.end();
        xSemaphoreGive(xNvsMutex); // Libera o mutex
    }
}
