# Growtron

## Grow automatizado para suas plantas
https://www.instagram.com/growtron.ino/

Funcionalidades:
* Liga e desliga as luzes do grow nos horários configurados no Blink App.
* Monitora a temperatura e umidade ambiente
* Monitora a umidade do solo
* Exibe as informações localmente no OLED
* Mede a luminosidade emitida pelas luzes do grow em Lux
* Timelapse configuravel com a ESP32-CAM

Componentes necessários para executar o projeto:
* ESP32 30 pinos
* Modulo Rélé 4 canais
* Sensor de Temperatura e Umidade DHT22
* Blynk app

Componentes opcionais:
* Sensor capacitivo de umidade do solo
* Sensor de luminosidade GY30
* Display OLED 0.91"
* ESP32-CAM


# Instalando a placa ESP32 no Arduino IDE

* Inicie o Arduino IDE e abra a janela de Preferências
* Inclua em "URLs Adicionais para Genrenciadores de Placas" o url da ESP32 https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json 
* Abra o Gerenciador de Placas em Ferramentas > Placa
* Procure por esp32 by Espressif Systems e instale
* Selecione sua a esp32 em Ferramentas > Placa > ESP32 Arduino > ESP32 Dev Module

# Instalando as bibliotecas necessárias

