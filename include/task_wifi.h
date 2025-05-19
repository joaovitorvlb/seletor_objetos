#ifndef __TASK_WIFI_H__
#define __TASK_WIFI_H__

#define WIFI_ESTADO_SETUP 0
#define WIFI_ESTADO_CONECTANDO 1
#define WIFI_ESTADO_CONECTADO 2
#define WIFI_ESTADO_AP_MODE 3

void TaskWifi(void *pvParameters);

#endif