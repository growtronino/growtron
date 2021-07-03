#define BLYNK_PRINT Serial    // Comente isto para desabilitar prints e economizar espaco

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <WidgetRTC.h>

// BLYNK AUTHENTICATION
char auth[] = "";
char ssid[] = "";
char pass[] = "";

// DHT TEMPERATURE AND HUMIDIDTY
#define DHTPIN 15     // definicao do pino do sensor de umidade e temperatura dht
#define DHTTYPE DHT22 // mudar para DHT11 caso esteja usando outra versao do sensor
DHT dht(DHTPIN, DHTTYPE);

// TIMER, ALARMS AND RTC
BlynkTimer timer;     // usamos o timer do blynk
WidgetRTC rtc;        // o real time clock do blynk nos da o horario atualizado via wifi

void alarmTimerEvent() {
  digitalClockDisplay();  // debug do horario
  Alarm.delay(0);   // necesario para sincronizacao do alarme
}

// isso e chamado quando o esp32 se conecta ao blynk server
BLYNK_CONNECTED() {
  rtc.begin();        // iniciamos o real time clock do blynk
  Alarm.delay(1000);  // necessario para os alarmes ficarem sincronizados
}

// esse evento e chamado a cada segundo para se ter um dado robusto 
// de umidade e temperatura a mostrar no graph do blynk. o graph do blynk
// usa automaticamente os valores dos virtual pins para gerar os graficos
// nao havendo necessidade de codificar 
void dhtTimerEvent() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // ou dht.readTemperature(true) para Fahrenheit

  // verificamos caso o sensor tenha gerado um dado descartavel
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT22 sensor!");
    return;
  }

  // atualizamos a umidade
  Blynk.virtualWrite(V1, h);
  // e atualizamos a temperatura
  Blynk.virtualWrite(V0, t);
}

void setup() {
Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  setSyncInterval(10 * 60);   // intervalo de sincronizacao do horario a cada 10 minutos(em segundos)
  setTime(hour(),minute(),second(),month(),day(),year());
  Serial.print("setup RTC Updated Time ");
  timer.setInterval(2500L, dhtTimerEvent);    // Chamado a cada segundo
  timer.setInterval(5000L, alarmTimerEvent);  // display digital atualiza a cada 5 segundos
  digitalClockDisplay();

  // inicializa o sensor de umidade e temperatura dht
  dht.begin();
}

// usando o blynk e importante nao usar o loop
// caso precise de um loop, use os timer event
void loop() {
  Blynk.run();
  timer.run();
}

// debug do relogio
void digitalClockDisplay() {
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}

// metodo que auxilia a printar o horario 
void printDigits(int digits) {
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
