#include "task_motor.h"
#include <freertos/mpu_wrappers.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <AccelStepper.h>

// Definição dos pinos do driver A4988
#define STEP_PIN 32  // Pino para o sinal de passo
#define DIR_PIN 33   // Pino para o sinal de direção

// Criação do objeto AccelStepper
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Definição do pino para o ponto zero
#define ZERO_GPIO_PIN 27 // Substitua 34 pelo número do pino correto

// Definição dos estados da máquina de estados
typedef enum {
    MOTOR_IDLE,         // Motor parado
    MOTOR_MOVING_TO,   // Motor movendo para frente
    MOTOR_FIND_ZERO     // Motor procurando o ponto zero
} MotorState;

// Definição dos comandos
typedef enum {
    CMD_NOOP = 1,
    CMD_STEP1 = 2,
    CMD_STEP2 = 3,
    CMD_STEP3 = 4,
    CMD_FIND_ZERO = 5
} MotorCommand;

void TaskMotor(void *pvParameters)
{
    stepper.setMaxSpeed(1200);       // Velocidade máxima em passos por segundo
    stepper.setAcceleration(6000);  // Aceleração em passos por segundo^2
    stepper.setSpeed(600);          // Velocidade inicial

    MotorState currentState = MOTOR_IDLE; // Estado inicial do motor
    int targetPosition = 0;               // Posição alvo do motor

    MotorCommand motorCommand = CMD_NOOP; // Instance of the MotorCommand enum

    while(1)
    {
        switch (currentState)
        {
            case MOTOR_IDLE:
            {
                // Motor está parado, aguarda comando
                break;
            }

            case MOTOR_MOVING_TO:
            {
                // Move o motor para frente até a posição alvo
                stepper.moveTo(targetPosition);
                // se a posição atual for igual à posição alvo, muda para o estado ocioso
                if (stepper.distanceToGo() == 0)  //  stepper.distanceToGo retorna a distancia da posição atual para a posição do alvo
                {
                    currentState = MOTOR_IDLE; // Retorna ao estado ocioso
                }
 
                break;
            }

            case MOTOR_FIND_ZERO:
            {
                // Procura o ponto zero do ângulo pelo pulso de uma GPIO
                if (digitalRead(ZERO_GPIO_PIN) == LOW) // Verifica se o pulso foi detectado
                {
                    stepper.setCurrentPosition(0); // Define a posição atual como zero
                    currentState = MOTOR_IDLE;    // Retorna ao estado ocioso
                }
                else
                {
                    stepper.moveTo(5000);
                }
                break;
            }

            default:
            {
                //stepper.setCurrentPosition(0); // Define a posição atual como zero
                //currentState = MOTOR_IDLE;    // Retorna ao estado ocioso
                break;
            }
        }

        stepper.run();

        uint32_t notificationValue;
        if (xTaskNotifyWait(0, 0, &notificationValue, 0))
        {
            motorCommand = static_cast<MotorCommand>(notificationValue);
            switch (notificationValue) 
            {
                case CMD_STEP1:
                {
                    targetPosition = -850;
                    currentState = MOTOR_MOVING_TO; // Inicia o movimento para frente
                    break;
                }

                case CMD_STEP2:
                {
                    targetPosition = -1250;
                    currentState = MOTOR_MOVING_TO; // Para o motor
                    break;
                }

                case CMD_STEP3:
                {
                    targetPosition = -1650;
                    currentState = MOTOR_MOVING_TO; // Para o motor
                    break;
                }

                case CMD_FIND_ZERO:
                {
                    currentState = MOTOR_FIND_ZERO; // Para o motor
                    break;
                }

                default:
                {
                    //currentState = MOTOR_IDLE; // Para o motor
                    break;
                }
            }
        }
        
        vTaskDelay(1 / portTICK_PERIOD_MS); // Aguarda 1 segundo
    }
}