#pragma once

#ifndef HOTPLATE_H
#define HOTPLATE_H

#include <Arduino.h>

// CONSTANTS
#define COOLDOWN_TIME 300  // 5 minutes in seconds count down
#define PREHEAT_TIME 600  // 10 minutes in seconds count up (not used)
#define REFLOW_TIME 600  // 10 minutes in seconds count up (not used)
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET 0  // D3/IO0/GPIO0
#define BUTTON_PIN 16  // D0/IO16/GPIO2

// PIN SETUP
const int thermoDO = 12;  //D6; // 12 data out
const int thermoCS = 13;  //D7; // 13 chip select
const int thermoCLK = 14;  //D5; // 14 clock
const int solidState = 15;  //D8; // 15 solid state pin
const int potMeter = A0;  // potmeter on analog 0
const int ledPin = 2;  //D4/IO2

// DEFAULT HOTPLATE TEMP IN CELCIUS
const int tempPreheat = 180;  // 180 preheat temperature
const int tempReflow = 235;  // 235 max reflow temperature

// POTMETER TEMPERATURE VALUES
int tempNow = 0; 
int tempNext = 0;
int tempPoti = 0;
int tempPotiOld = 0;

// BUTTON STATES
volatile bool buttonState = false;
volatile bool releasedState = true;

// HEATER STATES
String state[] = {"OFF", "PREHEAT", "REFLOW", "COOLING"};
int stateNow = 0;
int timeCount = 0;
int perc = 0;
int tempOffset = 0;

// TIMERS
unsigned long currentTime = millis();  // Set current time variable once at bootup/restart
unsigned long seconds = 0;
unsigned long seconds2 = 0;
unsigned long mils = 0;
unsigned long wasmils = 0;
unsigned long t = millis();  // display
unsigned long tSolder = millis();  // solder
unsigned long previousMillis = 0;  // thermocouple
unsigned long interval = 230;  // thermocouple

// DISPLAY VARS
//static char lineArrayX[128] = {1};
//static char lineArrayY[128] = {0};
//int lineMin = 54;  // min oled height (bottom)
//int lineMax = 8;  // max oled height (top)
//int lineBegin = 0;  // min oled screen width (left)
//int lineEnd = 128;  // max oled screen width (right)
//int lineCounter = 0;  // counter
//int roomTemp = 25;  // average room temperature of hotplate
//int tempStore = roomTemp;  // temperature storage variable

#endif
