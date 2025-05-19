#ifndef __INTERFACE_H__
#define __INTERFACE_H__

// === HTML DA PÁGINA ===
const char* htmlPage = R"rawliteral(
    <!DOCTYPE html>
    <html>
        <head>
            <title>ESP32 Web</title>

            <script>
                async function fetchData() {
                    try {
                        const response = await fetch('/get_config');
                        if (response.ok) {
                            const data = await response.json();
                 
                            document.querySelector('input[name="ssid"]').value = data.ssid || '';
                            document.querySelector('input[name="password"]').value = data.password || '';
                            document.querySelector('input[name="url1"]').value = data.url1 || '';
                            document.querySelector('input[name="rota1"]').value = data.rota1 || '';
                            document.querySelector('input[name="rota2"]').value = data.rota2 || '';
                        } else {
                            console.error('Failed to fetch data:', response.status);
                        }
                    } catch (error) {
                        console.error('Error fetching data:', error);
                    }
                }

                window.onload = fetchData;
            </script>

        </head>
        <body>
            <h1>Configuração</h1>
            <form action="/config_wifi" method="GET">
                SSID: <input type="text" name="ssid" size="30"><br>
                PASSWORD: <input type="text" name="password" size="30"><br>
                <input type="submit" value="Enviar">
            </form>

            <h1>Configuração D Servidor</h1>
            <form action="/config_servidor" method="GET">
                URL: <input type="text" name="url1" size="40"><br>
                ROTA 1: <input type="text" name="rota1" size="15"><br>
                ROTA 2: <input type="text" name="rota2" size="15"><br>
                <input type="submit" value="Enviar">
            </form>
        </body>
    </html>
    )rawliteral";

#endif