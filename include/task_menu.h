#ifndef __TASK_MENU_H__
#define __TASK_MENU_H__

#define MENU_STATE_CORES 0
#define MENU_STATE_CFG_GANHO 1
#define MENU_STATE_CFG_TEMPO 2
#define MENU_STATE_NETWORK 3
#define MENU_STATE_OPERACAO 4
#define MENU_STATE_CAL_BRANCO 5
#define MENU_STATE_CAL_PRETO 6
#define MENU_STATE_TEST_MOTOR 7


void TaskMenu(void *pvParameters);

#endif