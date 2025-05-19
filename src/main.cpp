#include <Arduino.h>
#include <Wire.h>
#include "task_menu.h"
#include "task_sensor.h"
#include "task_wifi.h"
#include "task_http_client.h"
#include "task_motor.h"
#include "tcs34725.h"
#include "i2c.h"
#include "bsp.h"
#include "conversor_cores.h"
#include <nvs_flash.h>

SemaphoreHandle_t xI2CMutex;
SemaphoreHandle_t xNvsMutex;

QueueHandle_t xSensorDataQueue;
QueueHandle_t xFilaSensor;
QueueHandle_t xFilaHttpClient;
QueueHandle_t xFilaHttpClientResposta;

TaskHandle_t xTaskSensorHandle;
TaskHandle_t xTaskHttpClientHandle;

void setup() 
{
	Serial.begin(9600);

	//nvs_flash_erase(); // Apaga todo o NVS
    //nvs_flash_init();

	pinMode(LED_ON_OUTPUT, OUTPUT);
	digitalWrite(LED_ON_OUTPUT, LOW);

	xI2CMutex = xSemaphoreCreateMutex();
	xNvsMutex = xSemaphoreCreateMutex();

	xFilaSensor = xQueueCreate(10, sizeof(TCS34725_data_t));

	xFilaHttpClient = xQueueCreate(10, sizeof(Pacote_data_t));

	xFilaHttpClientResposta = xQueueCreate(10, sizeof(Pacote_resp_t));

	Serial.begin(9600);

	i2c_init();

	xTaskCreate(TaskMenu, "TaskMenu", 4096, NULL, 1, NULL);
	xTaskCreate(TaskSensor, "Sensor", 2048, NULL, 1, &xTaskSensorHandle);
	xTaskCreate(TaskMotor, "Motor", 2048, NULL, 1, NULL);
	xTaskCreate(TaskWifi, "WiFi", 4096, NULL, 1, NULL);
	xTaskCreate(TaskHttpClient, "TaskSendToAPI", 4096, NULL, 1, &xTaskHttpClientHandle);
}


void loop() 
{
  // Empty loop as tasks are handled by FreeRTOS
}



