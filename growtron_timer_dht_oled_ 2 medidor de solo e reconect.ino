#define BLYNK_PRINT Serial    // Comente isto para desabilitar prints e economizar espaco

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <WidgetRTC.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BLYNK AUTHENTICATION
char auth[] = "2AISZwia-HFhVkS9S8pm9wDiRQZIoPXc";
char ssid[] = "Caveira-2.4G";
char pass[] = "elite24horas";

// SOIL MOISURE SENSOR
const int VASO_A_PIN = 27;  // fio verde
const int VASO_B_PIN = 26;  // fio azul
float dry = 3500;
float wet = 1455;
float dryA = 2500;
float wetA = 1500;
float dryB = 2500;
float wetB = 1300;
float dryMap = 0.0;
float wetMap = 100.0;
float vasoA = 0;
float vasoB = 0;
float vasoC = 0;

void soilTimerEvent() {
  vasoA = 0;
  vasoB = 0;

  // fazemos 100 leituras e pegamos a media para 
  // ter uma leitura mais estavel
  for (int i = 0; i <= 100; i++) {
    vasoA = vasoA + analogRead(VASO_A_PIN);
    vasoB = vasoB + analogRead(VASO_B_PIN);
    Alarm.delay(1);
  }

  vasoA = vasoA/100;
  vasoB = vasoB/100;

  // mapeamos o valor analogico para um valor mais amigavel
  float mapVasoA = map(vasoA, dryA, wetA, dryMap, wetMap);
  float mapVasoB = map(vasoB, dryB, wetB, dryMap, wetMap);

  mapVasoA = capValue(mapVasoA);
  mapVasoB = capValue(mapVasoB);

  Blynk.virtualWrite(V11, mapVasoA);
  Blynk.virtualWrite(V12, mapVasoB);
}
// DHT TEMPERATURE AND HUMIDIDTY
#define DHTPIN 15     // definicao do pino do sensor de umidade e temperatura dht
#define DHTTYPE DHT22 // mudar para DHT11 caso esteja usando outra versao do sensor
DHT dht(DHTPIN, DHTTYPE);

// TIMER, ALARMS AND RTC
BlynkTimer timer;     // usamos o timer do blynk
WidgetRTC rtc;        // o real time clock do blynk nos da o horario atualizado via wifi

int periodo = 1 * 60; // tempo do efeito emmerson em segundos
WidgetLED ledEmersonMorning(V7);
WidgetLED ledMain(V8);
WidgetLED ledEmersonAfternoon(V9);
bool morning = true;  
int emersonValue;
int tempEmersonValue = HIGH;

// RELAY
const int LEDS = 33;     
const int LED_EMMERSON = 32;

// OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);//Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 // do not change this. Error in video
#define LOGO16_GLCD_WIDTH  16 // do not change this. Error in video

#define LOGO_HEIGHT   25
#define LOGO_WIDTH    25
static const unsigned char PROGMEM logo_bmp[] =
{ B00000000,B00001000,B00000000,B00000000,
  B00000000,B00001000,B00000000,B00000000,
  B00000000,B00011100,B00000000,B00000000,
  B00000000,B00011100,B00000000,B00000000,
  B00000000,B00011100,B00000000,B00000000,
  B00000000,B00111110,B00000000,B00000000,
  B00000000,B00111110,B00000000,B00000000,
  B00000000,B00111110,B00000000,B00000000,
  B10000000,B01111111,B00000000,B10000000,
  B11100000,B01111111,B00000011,B10000000,
  B01111000,B01111111,B00001111,B00000000,
  B01111110,B01111111,B00111111,B00000000,
  B00111111,B00111110,B01111110,B00000000,
  B00111111,B11111111,B11111110,B00000000,
  B00011111,B11111111,B11111100,B00000000,
  B00001111,B11111111,B11111000,B00000000,
  B00000111,B11111111,B11110000,B00000000,
  B00000011,B11111111,B11100000,B00000000,
  B00001111,B11111111,B11111000,B00000000,
  B00111111,B11111111,B11111110,B00000000,
  B00111111,B11111111,B11111110,B00000000,
  B00000111,B11111111,B11110000,B00000000,
  B00000000,B11100011,B10000000,B00000000,
  B00000001,B11001001,B11000000,B00000000,
  B00000001,B10000000,B11000000,B00000000};

    // look at line 27 to 30 of Adafruit_SSD1306.h inside the library to select the dimensions
#if (SSD1306_LCDHEIGHT != 32) // 
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// evento chamado a cada 5 segundos para atualizar o display do relogio
void alarmTimerEvent() {
  digitalClockDisplay();  // debug do horario
  Alarm.delay(0);   // necesario para sincronizacao do alarme
}

// isso e chamado quando o esp32 se conecta ao blynk server
BLYNK_CONNECTED() {
  rtc.begin();        // iniciamos o real time clock do blynk
  Alarm.delay(1000);  // necessario para os alarmes ficarem sincronizados
}

// isto e chamado pelo blynk quando um virtual pin e alterado no app
// nesse caso o v2 virtual pin 2 esta configurado no widget de time input
// com start-stop time, para ter um alarme de inicio e fim do fotoperiodo
BLYNK_WRITE(V2) {
  TimeInputParam t(param);
  
  // processa o horario de acender os LEDs
  if (t.hasStartTime()) {
    Alarm.free(0);
    Alarm.alarmRepeat(t.getStartHour(),
                      t.getStartMinute(),
                      t.getStartSecond(), 
                      LEDInitialAlarm);

    if (emersonValue == HIGH) {
      Alarm.enable(1);
      // configura o inicio do efeito emerson subtraindo 
      // o periodo do efeito do horario dos LEDs acender
      Alarm.write(1, Alarm.read(0) - periodo);
    }
  }

  // processa o horario de desligar os LEDs
  if (t.hasStopTime()) {    
    Alarm.free(2);
    Alarm.alarmRepeat(t.getStopHour(),
                      t.getStopMinute(),
                      t.getStopSecond(), 
                      LEDFinalAlarm);

    if (emersonValue == HIGH) {
      Alarm.enable(3);
      // configura o fim do efeito emerson para o horario 
      // que os LEDs apagam mais o periodo do efeito
      Alarm.write(3, Alarm.read(2) + periodo);
    }
  }

  // sincroniza os reles de acordo com o horario atual
  // e o fotoperiodo configurado
  syncRelays((hour() * 60 * 60) + (minute() * 60) + second());
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
  Blynk.virtualWrite(V5, h);
  // e atualizamos a temperatura
  Blynk.virtualWrite(V6, t);

  displayOLED(t, h);
}

void setup() {
Serial.begin(9600);
  // inicializamos o OLED
  setupOLED();

  Blynk.begin(auth, ssid, pass);

  setSyncInterval(10 * 60);   // intervalo de sincronizacao do horario a cada 10 minutos(em segundos)
  setTime(hour(),minute(),second(),month(),day(),year());
  Serial.print("setup RTC Updated Time ");
  timer.setInterval(2500L, dhtTimerEvent);    // Chamado a cada 2.5 segundos
  timer.setInterval(5000L, alarmTimerEvent);  // display digital atualiza a cada 5 segundos
  digitalClockDisplay();

  // inicializa o sensor de umidade e temperatura dht
  dht.begin();          

  // inicializamos os alarmes e reles
  setupAlarms();
  setupRelays();

  Blynk.syncVirtual(V2);
}

// usando o blynk e importante nao usar o loop
// caso precise de um loop, use os timer event
void loop() {
  Blynk.run();
  timer.run();
}

// aciona os reles para acender o LED
void LEDInitialAlarm() {
  digitalWrite(LEDS, HIGH);
  ledMain.on();

  if (emersonValue == HIGH) {
    EmmersonFinalAlarm();
  }
}

// aciona os reles para desligar o LED
void LEDFinalAlarm() {
  digitalWrite(LEDS, LOW);
  ledMain.off();
  morning = false;

  if (emersonValue == HIGH) {
    EmmersonInitialAlarm();
  }
}

// aciona os reles para ligar o efeito emmerson
void EmmersonInitialAlarm() {
  if (emersonValue == HIGH) {
    digitalWrite(LED_EMMERSON, HIGH);

    if (morning) {
      ledEmersonMorning.on();
    } else {
      ledEmersonAfternoon.on();
    }
  }
}

// aciona os reles para desligar o efeito emmerson
void EmmersonFinalAlarm() {
  if (emersonValue == HIGH) {
    digitalWrite(LED_EMMERSON, LOW);

    if (morning) {
      ledEmersonMorning.off();
    } else {
      ledEmersonAfternoon.off();
    }
  }
  if (!morning) {
    morning = true;
  }
}

// configuracao inicial dos alarmes atribuindo seus IDs e funcoes atreladas
void setupAlarms() {
  Alarm.alarmRepeat(0, 0, 0, LEDInitialAlarm);
  Alarm.alarmRepeat(0, 0, 0, EmmersonInitialAlarm);
  Alarm.alarmRepeat(0, 0, 0, LEDFinalAlarm);
  Alarm.alarmRepeat(0, 0, 0, EmmersonFinalAlarm);
  
  Alarm.disable(0);
  Alarm.disable(1);
  Alarm.disable(2);
  Alarm.disable(3);
}

// configuracao inicial dos reles
void setupRelays() {
  pinMode(LEDS, OUTPUT);
  pinMode(LED_EMMERSON, OUTPUT);
}

void syncRelays(long timeInSeconds) {
  int ledsValue, ledsEmersonValue;

  if (Alarm.read(0) < Alarm.read(3)) {
    Serial.println("Os alarmes estao no mesmo dia.");
    if (timeInSeconds > Alarm.read(0) && timeInSeconds < Alarm.read(3)) {
      ledsValue = HIGH;
      ledsEmersonValue = HIGH;
      ledMain.on();
    } else {
      ledsValue = LOW;
      ledsEmersonValue = LOW;
      ledMain.off();
    }
  } else {
    Serial.println("Os alarmes estao em dias diferentes.");
    if (timeInSeconds > Alarm.read(3) && timeInSeconds < Alarm.read(0)) {
      ledsValue = LOW;
      ledsEmersonValue = LOW;
      ledMain.off();
    } else {
      ledsValue = HIGH;
      ledsEmersonValue = HIGH;
      ledMain.on();
    }
  }
  
  digitalWrite(LEDS, ledsValue);
  digitalWrite(LED_EMMERSON, ledsEmersonValue);  
}

// configuracao do OLED
void setupOLED() {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  display.clearDisplay(); 
  
  displayLogo();
}

void displayOLED(float temp, float umi) {
  display.clearDisplay();
  char strTemp[6];     //result string 5 positions + \0 at the end
  char strUmi[6];     //result string 5 positions + \0 at the end
  dtostrf(temp, 5, 1, strTemp);
  dtostrf(umi, 5, 1, strUmi);

  oledText(strTemp, -8, 0, 2, false);
  oledText("C", 53, 7, 1, false);
  
  oledText(strUmi, 54, 0, 2, false);
  oledText("%", 117, 7, 1, false);
  
  display.drawRect(1, 25, 126, 7, WHITE);
  display.display();
}

void oledText(String text, int x, int y,int size, boolean d) {
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x,y);
  display.println(text);
  if(d){
    display.display();
  }
}

void displayLogo(void) {
  display.clearDisplay();
  display.drawBitmap(
    0,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  
  oledText("Growtron", 30, 13, 2, true);
}

// debug do relogio
void digitalClockDisplay() {
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}
float capValue(float value) {
  if (value > 100) {
    return 100;
  } else if (value < 0) {
    return 0;
  } else {
    return value;
  }
}
// metodo que auxilia a printar o horario 
void printDigits(int digits) {
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
