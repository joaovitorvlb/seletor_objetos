#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "task_wifi.h"
#include "interface.h"
#include "nvs.h"
#include "pages/header/index_html.h"
#include <freertos/task.h>

extern TaskHandle_t xTaskHttpClientHandle;


String urlDecode(const String& input);


void TaskWifi(void *pvParameters) 
{
    // === CONFIG WIFI ===
    char ssid[20];       // Substitua pelo nome da sua rede Wi-Fi
    char password[20]; // Substitua pela senha da sua rede Wi-Fi

    char url[40];
    char rota1[20];
    char rota2[20];
    
    String ssid_ap = "ESP32-AP";
    String pass_ap = "12345678";

    enum ComandoHttpClient 
    {
        CMD_API_RELOAD
    };

    // === SERVIDOR WEB ===
    WebServer server(80);
    
    // === FLAG DE MODO ===
    volatile int modoWifi = 0;  // 0 = STA, 1 = AP

    ler_servidor_config(url, rota1, rota2);
    
    // === VARIÁVEIS RECEBIDAS DO FORM ===
    String nome = "";
    int idade = 0;
    uint8_t estado_wifi = 0;

    // Função para lidar com requisições HTTP
    server.on("/", [&server]() 
    {
        server.send(200, "text/html; charset=utf-8", index_html);
    });

     // Função para lidar com formularios
    server.on("/config_wifi", [&server, &ssid, &password, &estado_wifi]() 
    {
        String new_ssid = server.arg("ssid");
        String new_password = server.arg("password");

        if (!new_ssid.isEmpty() && !new_password.isEmpty()) 
        {
            strncpy(ssid, new_ssid.c_str(), sizeof(ssid) - 1);
            ssid[sizeof(ssid) - 1] = '\0'; // Ensure null-termination

            strncpy(password, new_password.c_str(), sizeof(password) - 1);
            password[sizeof(password) - 1] = '\0'; // Ensure null-termination

            estado_wifi = WIFI_ESTADO_SETUP; // Reinicia o processo de conexão com o novo SSID e senha
            WiFi.disconnect(true);

            salvar_wifi_credenciais(ssid, password);

            server.send(200, "text/html; charset=utf-8", "Configurações salvas! Reiniciando conexão...");
        } 
        else 
        {
            server.send(400, "text/html; charset=utf-8", "SSID ou senha inválidos!");
        }
    });

    server.on("/config_servidor", [&server, &url, &rota1, &rota2]() 
    {
        // Decodifica os parâmetros recebidos
        String new_url = urlDecode(server.arg("url1"));
        String new_rota1 = urlDecode(server.arg("rota1"));
        String new_rota2 = urlDecode(server.arg("rota2"));

        Serial.println("URL: " + new_url);

        if (!new_url.isEmpty() && !new_rota1.isEmpty() && !new_rota2.isEmpty()) 
        {
            strncpy(url, new_url.c_str(), sizeof(url) - 1);
            url[sizeof(url) - 1] = '\0'; // Ensure null-termination

            strncpy(rota1, new_rota1.c_str(), sizeof(rota1) - 1);
            rota1[sizeof(rota1) - 1] = '\0'; // Ensure null-termination

            strncpy(rota2, new_rota2.c_str(), sizeof(rota2) - 1);
            rota2[sizeof(rota2) - 1] = '\0'; // Ensure null-termination

            Serial.println("URL: " + String(url));


            salvar_servidor_config(url, rota1, rota2);

            xTaskNotify(xTaskHttpClientHandle, CMD_API_RELOAD, eSetValueWithOverwrite);

            server.send(200, "text/html; charset=utf-8", "Configurações do servidor salvas!");
        } 
        else 
        {
            server.send(400, "text/html; charset=utf-8", "URL ou rotas inválidas!");
        }
    });

    server.on("/get_config", [&server, &ssid, &password, &url, &rota1, &rota2]() 
    {
        ler_servidor_config(url, rota1, rota2);

        Serial.println("URL: " + String(url));

        // Cria um JSON com os valores recebidos
        String jsonResponse = "{";
        jsonResponse += "\"ssid\":\"" + String(ssid) + "\",";
        jsonResponse += "\"password\":\"" + String(password) + "\",";
        jsonResponse += "\"url1\":\"" + String(url) + "\",";
        jsonResponse += "\"rota1\":\"" + String(rota1) + "\",";
        jsonResponse += "\"rota2\":\"" + String(rota2) + "\"";
        jsonResponse += "}";

        // Envia a resposta JSON
        server.send(200, "application/json", jsonResponse);
    });


    while (true) 
    {
        switch(estado_wifi)
        {
           
            case WIFI_ESTADO_SETUP:
            {
                String saved_ssid, saved_password;
                ler_wifi_credenciais(saved_ssid, saved_password);

                Serial.println(saved_ssid);
                Serial.println(saved_password);
    
                if (!saved_ssid.isEmpty() && !saved_password.isEmpty()) 
                {
                    strncpy(ssid, saved_ssid.c_str(), sizeof(ssid) - 1);
                    ssid[sizeof(ssid) - 1] = '\0'; // Ensure null-termination
    
                    strncpy(password, saved_password.c_str(), sizeof(password) - 1);
                    password[sizeof(password) - 1] = '\0'; // Ensure null-termination
                }

                WiFi.begin(ssid, password);
                Serial.print("Conectando ao Wi-Fi");
                estado_wifi = WIFI_ESTADO_CONECTANDO;
                break;
            }
            
            case WIFI_ESTADO_CONECTANDO:
            {
                int retryCount = 0;
                const int maxRetries = 10;

                while (WiFi.status() != WL_CONNECTED && retryCount < maxRetries) 
                {
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    Serial.print(".");
                    retryCount++;
                } 
                
                if (WiFi.status() == WL_CONNECTED)
                {
                    estado_wifi = WIFI_ESTADO_CONECTADO;
                    Serial.println("\nWi-Fi conectado!");
                    Serial.print("Endereço IP: ");
                    Serial.println(WiFi.localIP());
                    server.begin(); // Inicia o servidor web no modo STA
                }
                else if (retryCount >= maxRetries)
                {
                    Serial.println("\nFalha ao conectar ao Wi-Fi. Alternando para o modo AP.");
                    estado_wifi = WIFI_ESTADO_AP_MODE;
                    WiFi.disconnect(true); // Desativa o modo STA
                    WiFi.mode(WIFI_AP);    // Configura o modo AP

                    WiFi.softAP(ssid_ap.c_str(), pass_ap.c_str());
                    Serial.println("Modo AP ativado!");
                    Serial.print("Endereço IP do AP: ");
                    Serial.println(WiFi.softAPIP());
                    server.begin(); // Inicia o servidor web no modo AP
                }
                
                break;
            }

            case WIFI_ESTADO_CONECTADO:
            {
                if(WiFi.status() != WL_CONNECTED)
                {
                    estado_wifi = WIFI_ESTADO_SETUP;
                    server.stop(); // Para o servidor web ao perder conexão
                }
                server.handleClient(); // Lida com requisições HTTP no modo STA
                break;
            }

            case WIFI_ESTADO_AP_MODE:
            {
                    server.handleClient(); // Lida com requisições HTTP no modo AP
                break;
            }
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

String urlDecode(const String& input) {
    String decoded = "";
    char temp[] = "0x00";
    unsigned int len = input.length();
    unsigned int i = 0;
  
    while (i < len) {
      char c = input.charAt(i);
      if (c == '+') {
        decoded += ' ';
      } else if (c == '%' && i + 2 < len) {
        temp[2] = input.charAt(i + 1);
        temp[3] = input.charAt(i + 2);
        decoded += (char) strtol(temp, nullptr, 16);
        i += 2;
      } else {
        decoded += c;
      }
      i++;
    }
    return decoded;
  }