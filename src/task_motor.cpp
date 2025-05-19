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
#define ZERO_GPIO_PIN 34 // Substitua 34 pelo número do pino correto

// Definição dos estados da máquina de estados
typedef enum {
    MOTOR_IDLE,         // Motor parado
    MOTOR_MOVING_FWD,   // Motor movendo para frente
    MOTOR_MOVING_BWD,   // Motor movendo para trás
    MOTOR_FIND_ZERO     // Motor procurando o ponto zero
} MotorState;

// Definição dos comandos
typedef enum {
    CMD_START = 1,
    CMD_STOP = 2,
    CMD_FIND_ZERO = 3
} MotorCommand;

void TaskMotor(void *pvParameters)
{
    stepper.setMaxSpeed(200);       // Velocidade máxima em passos por segundo
    stepper.setAcceleration(1000);  // Aceleração em passos por segundo^2
    stepper.setSpeed(100);          // Velocidade inicial

    MotorState currentState = MOTOR_IDLE; // Estado inicial do motor
    int targetPosition = 0;               // Posição alvo do motor

    MotorCommand motorCommand = CMD_STOP; // Instance of the MotorCommand enum

    while(1)
    {
        switch (currentState)
        {
            case MOTOR_IDLE:
            {
                // Motor está parado, aguarda comando
                // Aqui você pode adicionar lógica para mudar de estado, como um sinal externo
                targetPosition = 230; // Exemplo: define uma posição alvo
                currentState = MOTOR_MOVING_FWD; // Muda para o estado de movimento para frente
                stepper.setCurrentPosition(0);
                break;
            }

            case MOTOR_MOVING_FWD:
            {
                // Move o motor para frente até a posição alvo
                stepper.moveTo(targetPosition);

                if (stepper.distanceToGo() == 0)
                {
                    currentState = MOTOR_MOVING_BWD; // Muda para o estado de movimento para trás
                    targetPosition = 0;             // Define nova posição alvo
                }
                break;
            }

            case MOTOR_MOVING_BWD:
            {
                // Move o motor para trás até a posição inicial
                stepper.moveTo(targetPosition);

                if (stepper.distanceToGo() == 0)
    
                {
                    currentState = MOTOR_IDLE; // Retorna ao estado ocioso
                }
                break;
            }

            case MOTOR_FIND_ZERO:
            {
                // Procura o ponto zero do ângulo pelo pulso de uma GPIO
                if (digitalRead(ZERO_GPIO_PIN) == HIGH) // Verifica se o pulso foi detectado
                {
                    stepper.setCurrentPosition(0); // Define a posição atual como zero
                    currentState = MOTOR_IDLE;    // Retorna ao estado ocioso
                }

                stepper.moveTo(targetPosition);
                break;
            }

            default:
            {
                stepper.setCurrentPosition(0); // Define a posição atual como zero
                currentState = MOTOR_IDLE;    // Retorna ao estado ocioso
                break;
            }
        }

        stepper.run();

        uint32_t notificationValue;
        if (xTaskNotifyWait(0, 0, &notificationValue, 0))
        {
            motorCommand = static_cast<MotorCommand>(notificationValue);
            switch (motorCommand) 
            {
                case CMD_START:
                {
                    currentState = MOTOR_MOVING_FWD; // Inicia o movimento para frente
                    break;
                }

                case CMD_STOP:
                {
                    currentState = MOTOR_IDLE; // Para o motor
                    break;
                }

                case CMD_FIND_ZERO:
                {
                    currentState = MOTOR_IDLE; // Para o motor
                    break;
                }

                default:
                {
                    currentState = MOTOR_IDLE; // Para o motor
                    break;
                }
            }
        }
        
        vTaskDelay(1 / portTICK_PERIOD_MS); // Aguarda 1 segundo
    }
}