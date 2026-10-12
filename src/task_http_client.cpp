#include <WiFi.h>
#include "task_http_client.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "conversor_cores.h"
#include "tela.h"
#include "bsp.h"
#include <freertos/task.h>
#include <freertos/mpu_wrappers.h>
#include <nvs.h>

extern QueueHandle_t xFilaHttpClient;
extern QueueHandle_t xFilaHttpClientResposta;


void TaskHttpClient(void *pvParameters) 
{
    enum ComandoHttpClient 
    {
        CMD_API_RELOAD
    };

    JsonDocument jsonDoc;
    Pacote_data_t pacote;
    Pacote_resp_t pacote_resposta;

    char apiUrl[60];
    char host[40];
    char rota1[20];
    char rota2[20];
    char rota3[20];

    // Ler as configurações do servidor
    ler_servidor_config(host, rota1, rota2, rota3);

    uint8_t operacao = OPERACAO_SELECIONANDO;

    uint32_t cmd; // Declare the variable to store the notification value

    while (true) 
    {
        if (xQueueReceive(xFilaHttpClient, &pacote, 0) == pdPASS) 
        {
            // Verifica se o Wi-Fi está conectado no modo STA
            if (WiFi.status() == WL_CONNECTED) 
            {
                if (pacote.operacao == OPERACAO_SELECIONANDO) 
                {
                    // Montar a URL da API
                    snprintf(apiUrl, sizeof(apiUrl), "%s%s", host, rota1);
                    Serial.printf("URL da API: %s\n", apiUrl);
                }
                else if(pacote.operacao == OPERACAO_CLASSIFICANDO) 
                {
                    // Montar a URL da API
                    snprintf(apiUrl, sizeof(apiUrl), "%s/%s", host, rota2);
                    Serial.printf("URL da API: %s\n", apiUrl);
                }

                // Configuração do cliente HTTP
                HTTPClient http;
                http.begin(apiUrl);
                http.addHeader("Content-Type", "application/json");

                // Criar o JSON com os dados RGB, HSV e descrição
                jsonDoc["r"] = pacote.r;  // Valor do componente R
                jsonDoc["g"] = pacote.g;  // Valor do componente G
                jsonDoc["b"] = pacote.b;  // Valor do componente B 
                jsonDoc["descricao"] = descricao_options[pacote.classificacao]; // Descrição da cor

                // Serializar o JSON para uma string
                String jsonString;
                serializeJson(jsonDoc, jsonString);

                // Enviar a requisição POST
                int httpResponseCode = http.POST(jsonString);

                // Verifica a resposta da API
                if (httpResponseCode > 0) 
                {
                    Serial.printf("Resposta da API: %d\n", httpResponseCode);
                    
                    // Preencher pacote_resposta com a mensagem da resposta da API
                    String respostaApi = http.getString();
                    strncpy(pacote_resposta.mensagem, respostaApi.c_str(), sizeof(pacote_resposta.mensagem) - 1);
                    pacote_resposta.mensagem[sizeof(pacote_resposta.mensagem) - 1] = '\0'; // Garantir terminação nula

                    Serial.println(pacote_resposta.mensagem);

                   xQueueSend(xFilaHttpClientResposta, &pacote_resposta, portMAX_DELAY);
                } 
                else 
                {
                    // Preencher JSON de erro
                    JsonDocument errorDoc;
                    errorDoc["padrao"] = "Erro ao enviar dados para a API";
                    errorDoc["distancia"] = -1.0f;

                    String errorJson;
                    serializeJson(errorDoc, errorJson);

                    strncpy(pacote_resposta.mensagem, errorJson.c_str(), sizeof(pacote_resposta.mensagem) - 1);
                    pacote_resposta.mensagem[sizeof(pacote_resposta.mensagem) - 1] = '\0';
                    xQueueSend(xFilaHttpClientResposta, &pacote_resposta, portMAX_DELAY);
                    Serial.printf("Erro ao enviar dados: %s\n", http.errorToString(httpResponseCode).c_str());
                }
                http.end();
            }
            else 
            {
                Serial.println("Wi-Fi não conectado. Dados não enviados.");
            }
        } 

        if (xTaskNotifyWait(0, 0, &cmd, 0))
        {
            switch (cmd) 
            {
                case CMD_API_RELOAD:
                {
                    // Ler as configurações do servidor
                    ler_servidor_config(host, rota1, rota2, rota3);
                    break;
                }
            }
        }
            
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}