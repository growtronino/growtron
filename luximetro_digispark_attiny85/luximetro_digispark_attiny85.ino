/*
 * Tiny4kOLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x32 displays
 *
 * Based on ssd1306xled, re-written and extended by Stephen Denne
 * from 2017-04-25 at https://github.com/datacute/Tiny4kOLED
 *
 * This example shows a full screen rectangle,
 * writes the rectangle size inside the rectangle,
 * and scrolls the size off the screen.
 *
 */

#include <TinyI2CMaster.h>
#include <Tiny4kOLED.h>
#include <BH1750FVI.h> // BH1750FVI Light sensor library

BH1750FVI LightSensor(BH1750FVI::k_DevModeContHighRes2);
unsigned long lux;

uint8_t width = 128;
uint8_t height = 32;

#include "font16x32digits.h"
const DCfont *currentFont = FONT16X32DIGITS;

//delay to millis()
int period = 1000;
unsigned long time_now = 0;

const int buttonPin = 4;
int buttonState;             
int lastButtonState = LOW;   
const int ledPin = 1;      
int ledState = HIGH;         
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

const int holdButtonPin = 1;
int holdButtonState;             
int lastHoldButtonState = LOW;    // the debounce time; increase if the output flickers
unsigned long lastHoldDebounceTime = 0;  // the last time the output pin was toggled

int sensorMode = 2;
int hold = LOW;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledState);

  pinMode(holdButtonPin, INPUT);
  
  LightSensor.begin();
  
//  oled.begin(width, height, sizeof(tiny4koled_init_128x32br), tiny4koled_init_128x32br);
  oled.begin();
  oled.setFont(currentFont);
  oled.clear();
  oled.on();
  oled.switchRenderFrame();
}

void loop() {

  buttonDebounce();
  holdDebounce();

//  if(millis() > time_now + period){
//    time_now = millis();

    if (!hold) {
      lux = LightSensor.GetLightIntensity();
    }
    oled.clear();
    oled.print(lux);
//    oled.switchFrame();
//  }

  for (uint8_t y = 1; y < (sensorMode + 2); y++) {
    oled.setCursor(125, y);
    oled.startData();
    oled.sendData(0b11111111);
//    oled.repeatData(0b11111111, 8);
    oled.endData();
  }

  if (hold) {
    for (uint8_t y = 1; y < 4; y++) {
    oled.setCursor(121, y);
    oled.startData();
    oled.sendData(0b11111111);
//    oled.repeatData(0b11111111, 8);
    oled.endData();
  }
  }
    oled.switchFrame();
}

void buttonDebounce() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        
        if (sensorMode < 2) {
          sensorMode++;  
        } else {
          sensorMode = 0;
        }
        
        if (sensorMode == 0) {
          LightSensor.SetMode(BH1750FVI::k_DevModeContLowRes);
        } else if (sensorMode == 1) {
          LightSensor.SetMode(BH1750FVI::k_DevModeContHighRes);  
        } else if (sensorMode == 2) {
          LightSensor.SetMode(BH1750FVI::k_DevModeContHighRes2);  
        }
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

void holdDebounce() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(holdButtonPin);

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastHoldButtonState) {
    // reset the debouncing timer
    lastHoldDebounceTime = millis();
  }

  if ((millis() - lastHoldDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != holdButtonState) {
      holdButtonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (holdButtonState == HIGH) {
        hold = !hold;
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastHoldButtonState = reading;
}
