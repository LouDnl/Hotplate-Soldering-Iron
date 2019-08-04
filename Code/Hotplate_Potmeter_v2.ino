/* 
 * Board Manager package: ESP8266 Modules
 * Board: WeMos D1 R2 & mini
 * Board part number: -
 * 
 * Based completely on the code by Maker Moekoe
 * https://github.com/makermoekoe/Hotplate-Soldering-Iron
 * 
 * Edited by LouD - 2019
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <max6675.h>
#include <TactileButton.h>

#define COOLDOWN_TIME 180 // in seconds
#define PREHEAT_TIME 60
#define REFLOW_TIME 60

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int thermoDO = 12;
int thermoCS = 13;
int thermoCLK = 14;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

const int solidstate = 15;
const int poti = A0;
Button myButton(2);

const int temp_preheat = 140; // 150
const int temp_reflow = 200; // 220

int temp_now = 0;
int temp_next = 0;
int temp_poti = 0;
int temp_poti_old = 0;

String state[] = {"OFF", "PREHEAT", "REFLOW", "COOLING"};
int state_now = 0;

int time_count = 0;
int perc = 0;

int offset = 0;

void setup() {
  //Serial.begin(9600);
  myButton.begin();
  pinMode(solidstate, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  // wait for MAX chip to stabilize
  delay(500);
}

// Display & Solder time
long t = millis();
long t_solder = millis();

// Thermocouple
long previousMillis = 0; 
long interval = 230;

void loop() {

  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;  
    temp_now = thermocouple.readCelsius();
  }
  
  temp_poti = map(analogRead(poti), 1023, 0, temp_preheat, temp_reflow);

  if (temp_poti != temp_poti_old) {
    int v = 0;
    while (v < 100) {
      temp_poti = map(analogRead(poti), 1023, 0, temp_preheat, temp_reflow);
      if (temp_poti > temp_poti_old + 1 ||temp_poti < temp_poti_old - 1) {
        display.fillScreen(WHITE);
        display.setTextColor(BLACK);
        display.setTextSize(1);
        display.setCursor(46.50, 2.40);
        display.println("REFLOW");
        display.setTextSize(2);
        display.setCursor(30, 22);
        display.println(String(temp_poti)  + " " + ((char)247) + "C");
        display.display();
        temp_poti_old = temp_poti;
        v = 0;
      }
      v++;
      delay(10);
    }
    temp_poti_old = temp_poti;
  }

  if (millis() > t + 200 || millis() < t) {
    PrintScreen(state[state_now], temp_next, temp_now, time_count, perc);
    t = millis();
  }

  if (myButton.isReleased()) {
    int c = 0;
    while (myButton.isReleased()) {
      c++;
      if (c > 150) {
            digitalWrite(solidstate, LOW);
            state_now = 0;
            display.fillScreen(WHITE);
            display.setTextColor(BLACK);
            display.setTextSize(2);
            display.setCursor(0, 0);
            display.println(state[state_now]);
            display.display();
        while (myButton.isReleased()) millis() + 1;
        return;
      }
    }

    t_solder = millis();
    perc = 0,
    state_now++;
    if (state_now == 0) temp_next = 0;
    else if (state_now == 1) temp_next = temp_preheat;
    else if (state_now == 2) temp_next = temp_poti;
    else if (state_now == 3) temp_next = 0;
    else if (state_now == 4) {
      state_now = 0;
      temp_next = 0;
    }
  }

  if (state_now == 1) { // Preheat
    regulate_temp(temp_now, temp_next);

    perc = int((float(temp_now) / float(temp_next)) * 100.00);
  }
  else if (state_now == 2) { // Reflow
    regulate_temp(temp_now, temp_next);

    perc = int((float(temp_now) / float(temp_next)) * 100.00);
    if (perc >= 100) {
      state_now = 3;
      t_solder = millis();
      perc = 0;
      temp_next = 0;
    }
  }
  else if (state_now == 3) { // Cooling
    digitalWrite(solidstate, LOW);

    time_count = int((t_solder + COOLDOWN_TIME * 1000 - millis()) / 1000);
    if (time_count <= 0) {
      state_now = 0;
    }
  }
  else { // Any other state
    digitalWrite(solidstate, LOW);
    time_count = 0;
  }
}

void regulate_temp(int temp, int should) {
  if (should <= temp - offset) {
    digitalWrite(solidstate, LOW);
  }
  else if (should > temp + offset) {
    digitalWrite(solidstate, HIGH);
  }
}

void PrintScreen(String state, int soll_temp, int ist_temp, int tim, int percentage) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(state);

  display.setCursor(92, 0);
  String str = String(soll_temp) + " " + ((char)247) + "C";
  display.println(str);

  if (tim != 0) {
    display.setCursor(0, 56);
    str = String(tim) + " sec";
    display.println(str);
  }

  if (percentage != 0) {
    display.setCursor(98, 56);
    str = String(percentage) + " %";
    display.println(str);
  }

  display.setTextSize(2);
  display.setCursor(30, 22);
  str = String(ist_temp) + " " + ((char)247) + "C";
  display.println(str);

  display.display();
}
