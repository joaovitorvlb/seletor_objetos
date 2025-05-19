#include <Arduino.h>
#include <Preferences.h>
#include <freertos/queue.h>
#include "task_sensor.h"
#include "tcs34725.h"
#include "bsp.h"
#include "conversor_cores.h"
#include <freertos/task.h>
#include <nvs.h>


extern SemaphoreHandle_t xI2CMutex;
extern QueueHandle_t xFilaSensor;
extern QueueHandle_t xFilaHttpClient;

void inicializa_sensor(TCS34725RawData *data_preto, TCS34725RawData *data_branco, TCS34725_cfg_t *sensor);
void inicializa_sensor2(TCS34725RawData *data_preto, TCS34725RawData *data_branco, TCS34725_cfg_t *sensor);


void TaskSensor(void *pvParameters)
{
    enum ComandoSensor 
    {
        CMD_LER_PRETO,
        CMD_LER_BRANCO,
        CMD_LER_COR,
		CMD_RELOAD_SENSOR
    };

    Pacote_data_t pacote;
    HSV hsv;
    RGB rgb;

    uint8_t operacao = OPERACAO_SELECIONANDO;
    uint32_t cmd; // Declare the variable 'cmd' to store the notification value

    TCS34725_cfg_t sensor;
    TCS34725RawData data_preto;
    TCS34725RawData data_branco;

    inicializa_sensor(&data_preto, &data_branco, &sensor);

    Serial.print("Preto - R: ");
    Serial.print(data_preto.r);
    Serial.print(", G: ");
    Serial.print(data_preto.g);
    Serial.print(", B: ");
    Serial.println(data_preto.b);

    Serial.print("Branco - R: ");
    Serial.print(data_branco.r);
    Serial.print(", G: ");
    Serial.print(data_branco.g);
    Serial.print(", B: ");
    Serial.println(data_branco.b);

    Serial.print("Sensor - Ganho: ");
    Serial.print(sensor.ganho);
    Serial.print(", Tempo: ");
    Serial.print(sensor.tempo);
    Serial.print(", Modo: ");
    Serial.println(sensor.modo);

    while (1)
    {

        if (xSemaphoreTake(xI2CMutex, portMAX_DELAY)) 
        {
            if (xTaskNotifyWait(0, 0, &cmd, 0)) // Use 0 as the wait time to avoid blocking
            {
                switch (cmd) 
                {
                    case CMD_LER_PRETO:
                    {
                        digitalWrite(LED_ON_OUTPUT, HIGH);
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        TCS34725RawData data_preto = tcs34725_raw_data();
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        digitalWrite(LED_ON_OUTPUT, LOW);
                        salvar_calibracao_cor("preto", data_preto);
                        break;
                    }

                    case CMD_LER_BRANCO:
                    {
                        digitalWrite(LED_ON_OUTPUT, HIGH);
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        TCS34725RawData data_preto = tcs34725_raw_data();
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        digitalWrite(LED_ON_OUTPUT, LOW);
                        salvar_calibracao_cor("branco", data_preto);                     
                        break;
                    }

                    case CMD_LER_COR:
                    {
                        digitalWrite(LED_ON_OUTPUT, HIGH);
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        TCS34725_data_t sensor_data = tcs34725_read_data();
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                        digitalWrite(LED_ON_OUTPUT, LOW);

                        xQueueSend(xFilaSensor, &sensor_data, portMAX_DELAY);

                        pacote.r = sensor_data.r_norm;
                        pacote.g = sensor_data.g_norm;
                        pacote.b = sensor_data.b_norm;

                        pacote.operacao = operacao;

                        pacote.classificacao = 1;
                        
                        xQueueSend(xFilaHttpClient, &pacote, portMAX_DELAY);

                        break;
                    }

                    case CMD_RELOAD_SENSOR:
                    {
                        vTaskDelay(200 / portTICK_PERIOD_MS);
                        inicializa_sensor2(&data_preto, &data_branco, &sensor);
                        break;
                    }
                }
            }
            xSemaphoreGive(xI2CMutex);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void inicializa_sensor(TCS34725RawData *data_preto, TCS34725RawData *data_branco, TCS34725_cfg_t *sensor)
{
    *data_preto = ler_calibracao_cor("preto");
    *data_branco = ler_calibracao_cor("branco");
    *sensor = ler_calibracao_sensor("sensor");

    // Configuração do sensor TCS34725
    if (xSemaphoreTake(xI2CMutex, portMAX_DELAY)) 
    {
        tcs34725_init(sensor->ganho, sensor->tempo, sensor->modo); // Configura o ganho e o tempo de integração
        xSemaphoreGive(xI2CMutex);
    }
}


void inicializa_sensor2(TCS34725RawData *data_preto, TCS34725RawData *data_branco, TCS34725_cfg_t *sensor)
{
    *data_preto = ler_calibracao_cor("preto");
    *data_branco = ler_calibracao_cor("branco");
    *sensor = ler_calibracao_sensor("sensor");
    tcs34725_init(sensor->ganho, sensor->tempo, sensor->modo); // Configura o ganho e o tempo de integraçã
}


