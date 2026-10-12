#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "task_menu.h"
#include "botao_com_borda.h"
#include "botao_com_borda2.h"
#include "tela.h"
#include "i2c.h"
#include "tcs34725.h"
#include "conversor_cores.h"
#include <ArduinoJson.h>
#include <freertos/task.h>
#include "bsp.h"
#include <WiFi.h>
#include <nvs.h>


extern SemaphoreHandle_t xI2CMutex;

extern QueueHandle_t xFilaSensor;
extern QueueHandle_t xFilaHttpClientResposta;

extern TaskHandle_t xTaskSensorHandle;
extern TaskHandle_t xTaskMotorHandle;

// Instância do display LCD (endereço padrão 0x27, 16 colunas, 2 linhas)
LiquidCrystal_I2C lcd(0x22, 20, 4);

// Handle do timer
TimerHandle_t xOneShotTimer;

// Função de callback do timer
void vTimerCallback(TimerHandle_t xTimer);
uint8_t ler_gpio(void);

void TaskMenu(void *pvParameters) 
{
	xOneShotTimer = xTimerCreate(
        "OneShotTimer",           // Nome do timer (opcional)
        pdMS_TO_TICKS(2000),      // Período do timer (5 segundos)
        pdFALSE,                  // pdFALSE para contagem única
        (void *)0,                // ID do timer (opcional)
        vTimerCallback            // Função de callback
    );

	enum ComandoSensor {
		CMD_LER_PRETO,
		CMD_LER_BRANCO,
		CMD_LER_COR,
		CMD_RELOAD_SENSOR
	};

	typedef enum {
		CMD_NOOP = 1,
		CMD_STEP1 = 2,
		CMD_STEP2 = 3,
		CMD_STEP3 = 4,
		CMD_FIND_ZERO = 5
	} MotorCommand;

	uint8_t gpio;

	uint8_t operacao = 0;

	uint8_t estado_menu = 0;
	uint8_t led_state_flag = LOW;

	char formatted_string[20];

	TCS34725_data_t sensor_data;

	Pacote_resp_t pacote_resposta;

	BotaoComBorda2 botao_avancar(&gpio, 3);
	BotaoComBorda2 botao_voltar(&gpio, 2);
	BotaoComBorda2 botao_selecionar(&gpio, 4);
	BotaoComBorda2 botao_aumentar(&gpio, 0);
	BotaoComBorda2 botao_diminuir(&gpio, 1);

	BotaoComBorda2 botao_f1(&gpio, 5);
	BotaoComBorda2 botao_f2(&gpio, 6);
	BotaoComBorda2 botao_f3(&gpio, 7);

	// Valores de referência HSV (uint16_t): H [0–360], S [0–100], V [0–100]
	referenciaHSV[0][0] = 100; referenciaHSV[0][1] = 80; referenciaHSV[0][2] = 50;  // Verde
	referenciaHSV[1][0] =  30; referenciaHSV[1][1] = 90; referenciaHSV[1][2] = 70;  // Alaranjado
	referenciaHSV[2][0] =   5; referenciaHSV[2][1] = 85; referenciaHSV[2][2] = 85;  // Vermelho Claro
	referenciaHSV[3][0] =   0; referenciaHSV[3][1] = 95; referenciaHSV[3][2] = 90;  // Vermelho Maduro

	if (xSemaphoreTake(xI2CMutex, portMAX_DELAY)) 
	{
		lcd.init();
		lcd.backlight();
		lcd.print("Inicializando...");
		xSemaphoreGive(xI2CMutex);
	}

	vTaskDelay(1000 / portTICK_PERIOD_MS);

	TCS34725_cfg_t sensor = ler_calibracao_sensor("sensor");

	while (true) 
	{
		if (xSemaphoreTake(xI2CMutex, portMAX_DELAY)) 
        {
			gpio = ler_gpio();
			
			switch (estado_menu)
			{
				case MENU_STATE_CORES:
				{
					if(botao_selecionar.caiu())
					{
						xTaskNotify(xTaskSensorHandle, CMD_LER_COR, eSetValueWithOverwrite);
					}

					lcd.setCursor(0, 0);
					lcd.print(menu_items[0]);

					// Recebe mensagem da task sensor
					if (xQueueReceive(xFilaSensor, &sensor_data, 0) == pdPASS) 
					{
						lcd.setCursor(0, 1);
						snprintf(formatted_string, sizeof(formatted_string), "R:%3d G:%3d B:%3d", sensor_data.r_norm, sensor_data.g_norm, sensor_data.b_norm);
						lcd.print(formatted_string);
					}	
					
					if (xQueueReceive(xFilaHttpClientResposta, &pacote_resposta, 0) == pdPASS) 
					{
						Serial.println(pacote_resposta.mensagem);
						// Extrai o conteúdo JSON de uma string e um float
						JsonDocument doc;
						DeserializationError error = deserializeJson(doc, pacote_resposta.mensagem);

						if (!error) 
						{
							const char* stringValue = doc["padrao"];
							float floatValue = doc["distancia"];
							int estagio_id = doc["estagio_id"];

							lcd.setCursor(0, 2);
							snprintf(formatted_string, sizeof(formatted_string), "%s       ", stringValue);
							lcd.print(formatted_string);

							lcd.setCursor(0, 3);
							snprintf(formatted_string, sizeof(formatted_string), "%.2f estagio %d   ", floatValue, estagio_id);
							lcd.print(formatted_string);

							MotorCommand cmd_m;

							if(estagio_id == 1)
							{
								cmd_m = CMD_STEP1;
							} 
							else if(estagio_id == 2)
							{
								cmd_m = CMD_STEP2;
							} 
							else if(estagio_id == 3)
							{
								cmd_m = CMD_STEP3;
							} 

							xTaskNotify(xTaskMotorHandle, cmd_m, eSetValueWithOverwrite);
						} 
						else 
						{
							lcd.setCursor(0, 1);
							lcd.print("Erro JSON");
						}
					}

					if(botao_avancar.caiu())
					{
						estado_menu = MENU_STATE_CFG_GANHO;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					break;
				}

				case MENU_STATE_CFG_GANHO:
				{
					if(botao_aumentar.caiu())
					{
						if (sensor.ganho < 3) 
						{
							sensor.ganho++;
						}
					}
					else if(botao_diminuir.caiu())
					{
						if(sensor.ganho > 0)
						{
							sensor.ganho--;
						}
					}

					lcd.setCursor(0, 0);
					lcd.print(menu_items[1]);

					lcd.setCursor(0, 1);
					snprintf(formatted_string, sizeof(formatted_string), "Ganho: %d", sensor.ganho);
					lcd.print(formatted_string);

					if(botao_avancar.caiu())
					{
						estado_menu  = MENU_STATE_CFG_TEMPO;
						lcd.setCursor(0, 1);
						lcd.print("                    "); // Apaga o conteúdo da segunda linha
					}
					else if(botao_voltar.caiu())
					{
						estado_menu = MENU_STATE_CORES;
						lcd.setCursor(0, 1);
						lcd.print("                    "); // Apaga o conteúdo da segunda linha
					}

					if(botao_selecionar.caiu())
					{
						// Envia uma notificação para a TaskSensor
						salvar_calibracao_sensor("sensor", sensor);
						xTaskNotify(xTaskSensorHandle, CMD_RELOAD_SENSOR, eSetValueWithOverwrite);

					}
					break;
				}

				case MENU_STATE_CFG_TEMPO:
				{
					if(botao_aumentar.caiu())
					{
						if (sensor.tempo < 255) 
						{
							sensor.tempo++;
						}
					}
					else if(botao_diminuir.caiu())
					{
						if(sensor.tempo > 0)
						{
							sensor.tempo--;
						}
					}

					lcd.setCursor(0, 0);
					lcd.print(menu_items[2]);
			
					lcd.setCursor(0, 1);
					lcd.print(sensor.tempo);

					if(botao_avancar.caiu())
					{
						estado_menu = MENU_STATE_NETWORK;
						lcd.setCursor(0, 1);
						lcd.print("                    "); // Apaga o conteúdo da segunda linha
						lcd.setCursor(0, 2);
						lcd.print("                    "); // Apaga o conteúdo da segunda linha
						lcd.setCursor(0, 3);
						lcd.print("                    "); // Apaga o conteúdo da segunda linha
					}
					else if(botao_voltar.caiu())
					{
						estado_menu = MENU_STATE_CFG_GANHO;
						lcd.setCursor(0, 1);
						lcd.print("                   "); // Apaga o conteúdo da segunda linha
					}

					if(botao_selecionar.caiu())
					{
						// Envia uma notificação para a TaskSensor
						salvar_calibracao_sensor("sensor", sensor);
						xTaskNotify(xTaskSensorHandle, CMD_RELOAD_SENSOR, eSetValueWithOverwrite);

					}

					break;
				}

				case MENU_STATE_NETWORK:
				{
					lcd.setCursor(0, 0);
					lcd.print(menu_items[3]);

					IPAddress ip;

					// Verifica o estado do WiFi
					if (WiFi.getMode() == WIFI_STA) 
					{
						lcd.setCursor(10, 0);
						lcd.print("STA   ");
						ip = WiFi.localIP();
					} 
					else if (WiFi.getMode() == WIFI_AP) 
					{
						lcd.setCursor(10, 0);
						lcd.print("AP    ");
						ip = WiFi.softAPIP();
					} 
					
					lcd.setCursor(0, 1);
					lcd.print(ip.toString().c_str());

					if(botao_avancar.caiu())
					{
						estado_menu = MENU_STATE_OPERACAO;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					else if(botao_voltar.caiu())
					{
						estado_menu = MENU_STATE_CFG_TEMPO;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					break;
				}

				case MENU_STATE_OPERACAO:
				{
					if(botao_f1.caiu())
					{
						operacao = !operacao;
					}

					lcd.setCursor(0, 0);
					lcd.print(menu_items[4]);

					lcd.setCursor(0, 1);
					lcd.print(operacao_options[operacao]);

					if(botao_avancar.caiu())
					{
						estado_menu = MENU_STATE_CAL_BRANCO;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					else if(botao_voltar.caiu())
					{
						estado_menu = MENU_STATE_NETWORK;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					break;
				}

				case MENU_STATE_CAL_BRANCO:
				{
					if(botao_selecionar.caiu())
					{
						// Envia uma notificação para a TaskSensor
						xTaskNotify(xTaskSensorHandle, CMD_LER_BRANCO, eSetValueWithOverwrite);
					}

					lcd.setCursor(0, 0);
					lcd.print(menu_items[5]);

					if(botao_avancar.caiu())
					{
						estado_menu = MENU_STATE_CAL_PRETO;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					else if(botao_voltar.caiu())
					{
						estado_menu = MENU_STATE_OPERACAO;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					break;
				}

				case MENU_STATE_CAL_PRETO:
				{
					if(botao_selecionar.caiu())
					{
						// Envia uma notificação para a TaskSensor
						xTaskNotify(xTaskSensorHandle, CMD_LER_PRETO, eSetValueWithOverwrite);
						xTimerStart(xOneShotTimer, 0);
					}

					lcd.setCursor(0, 0);
					lcd.print(menu_items[6]);

					if(botao_avancar.caiu())
					{
						estado_menu = MENU_STATE_TEST_MOTOR;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}

					if(botao_voltar.caiu())
					{
						estado_menu = MENU_STATE_CAL_BRANCO;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					break;
				}

				case MENU_STATE_TEST_MOTOR:
				{
					if(botao_f1.caiu())
					{
						// Envia uma notificação para a TaskSensor
						xTaskNotify(xTaskMotorHandle, CMD_STEP1, eSetValueWithOverwrite);
					}

					if(botao_f2.caiu())
					{
						// Envia uma notificação para a TaskSensor
						xTaskNotify(xTaskMotorHandle, CMD_STEP2, eSetValueWithOverwrite);
					}

					if(botao_f3.caiu())
					{
						// Envia uma notificação para a TaskSensor
						xTaskNotify(xTaskMotorHandle, CMD_STEP3, eSetValueWithOverwrite);
					}

					if(botao_selecionar.caiu())
					{
						// Envia uma notificação para a TaskSensor
						xTaskNotify(xTaskMotorHandle, CMD_FIND_ZERO, eSetValueWithOverwrite);
					}

					lcd.setCursor(0, 0);
					lcd.print(menu_items[7]);

					if(botao_voltar.caiu())
					{
						estado_menu = MENU_STATE_CAL_PRETO;
						lcd.setCursor(0, 1);
						lcd.print("                "); // Apaga o conteúdo da segunda linha
					}
					break;
				}

			}
				
            xSemaphoreGive(xI2CMutex);
        }
		vTaskDelay(50 / portTICK_PERIOD_MS);
	}
}

void vTimerCallback(TimerHandle_t xTimer)
{
	Serial.println("Timer callback triggered!");
}

uint8_t ler_gpio(void)
{
	uint8_t data = 0;

	Wire.beginTransmission(0x21);
	if (Wire.endTransmission() == 0) 
	{
		Wire.requestFrom(0x21, 1);
		if (Wire.available()) 
		{
			data = Wire.read();
		}
	}
	return data;
}