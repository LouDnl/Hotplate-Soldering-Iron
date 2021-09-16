
/* 
 * Board Manager package: ESP8266 Modules
 * Board: WeMos D1 R2 & mini
 * Board part number: -
 * 
 * Based on the code by Maker Moekoe:
 * https://github.com/makermoekoe/Hotplate-Soldering-Iron
 * 
 * max6675.cpp / delay.h fix included from:
 * https://github.com/adafruit/MAX6675-library/issues/9
 * 
 * button.cpp / button.h library:
 * https://github.com/LennartHennigs/Button2
 * 
 * by LouD - 2021
 */

// INCLUDES
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>  // Adafruit graphics library
#include <Adafruit_SSD1306.h>  // Adafruit OLED-SSD1306 controller library
#include "max6675.h"  // use changed library because of issue with included delay.h
#include "button.h"  // Button2 by LennartHennigs
#include "hotplate.h"  // Hotplate defines

// BUTTON
Button2 button = Button2(BUTTON_PIN);

// OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// THERMOCOUPLE
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

void tcReader() {  // read thermocouple temperature
  unsigned long currentMillis = millis();  // thermocouple timer
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;  
    tempNow = thermocouple.readCelsius();
  }
}

long potMeterReader() {  // potmeter value reader
  tempPoti = map(analogRead(potMeter), 1023, 0, tempPreheat, tempReflow);
  return tempPoti;
}

void potMeterDraw() {  // potmeter screen action
  if (tempPoti != tempPotiOld) {
    int v = 0;
    while (v < 100) {
      potMeterReader();
      if (tempPoti > tempPotiOld + 1 ||tempPoti < tempPotiOld - 1) {
        display.fillScreen(WHITE);
        display.setTextColor(BLACK);
        display.setTextSize(1);
        display.setCursor(46.50, 2.40);
        display.println("REFLOW");
        display.setTextSize(2);
        display.setCursor(30, 22);
        display.println(String(tempPoti)  + " " + ((char)247) + "C");
        display.display();
        tempPotiOld = tempPoti;
        v = 0;
      }
      v++;
      delay(10);
    }
    tempPotiOld = tempPoti;
//    DEBUG_MSG("POT TEMP SET: %d\n", tempPoti);
  }
}

void regulateTemp(int temp, int should) {  // temperature regulator
  if (should <= temp - tempOffset) {
    digitalWrite(solidState, LOW);
  }
  else if (should > temp + tempOffset) {
    digitalWrite(solidState, HIGH);
  }
}

//void released(Button2& btn) {  // button released handler
//  if (btn.wasPressedFor() <= 500) {
//    releasedState = !releasedState;//true;
////    buttonState = false;
//    Serial.println("Button was pressed");
//  }
//  Serial.println("Button released");
//}

void click(Button2& btn) {  // button click handler
  buttonState = true;
//  releasedState = false;
  Serial.println("Button clicked");
}

void doubleClick(Button2& btn) {  // button doubleclick handler
    stateNow = 0;
    tempNext = 0;
    buttonState = false;
//    buttonState = !buttonState;
//    releasedState = true;
    Serial.println("Button doubleClicked");
}

void writeDisplay(int stateN, int setTemp, int nowTemp, int tim, int percentage) {  // oled display handler  
  String stateDisplay = state[stateN];
  display.setTextColor(WHITE);
  display.clearDisplay();

  // display status in top left
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(stateDisplay);

  // display target temperature setting in top right
  display.setCursor(92, 0);
  String str = String(setTemp) + " " + ((char)247) + "C";
  display.println(str);

  // display timer in bottom left
  if (tim != 0) {
    display.setCursor(0, 56);
    str = String(tim) + " seconds";
    display.println(str);
  }

  // display heatup percentage in bottom right
  if (percentage != 0) {
    display.setCursor(98, 56);
    str = String(percentage) + " %";
    display.println(str);
  }

  // display temperature in middle of screen
  display.setTextSize(2);
  display.setCursor(30, 22);
  str = String(nowTemp) + " " + ((char)247) + "C";
  display.println(str);

  display.display();
}

void buttonAction() {  // buttonaction handler
  if (buttonState == true) {
//    buttonState = false;
    tSolder = millis();
    perc = 0,
    stateNow++;

    switch(stateNow) {  // buttonState switch handler
      case 0:  // off
        tempNext = 0; 
        buttonState = false;
        digitalWrite(ledPin, HIGH);
        break;
      case 1:  // preheat
        tempNext = tempPreheat; 
        buttonState = false;
        digitalWrite(ledPin, LOW);
        break;
      case 2:  // reflow
        tempNext = tempPoti; 
        buttonState = false;
        digitalWrite(ledPin, LOW);
        break;
      case 3:  // cooling
        tempNext = 0; 
        buttonState = false;
        digitalWrite(ledPin, HIGH);
        break;
      case 4:  // revert to off
        stateNow = 0;
        tempNext = 0;
        buttonState = false;
        digitalWrite(ledPin, HIGH);
      default:
        break;
    }
  }

  switch(stateNow) {  // hotplate state handler
    case 1:  // preheat
      regulateTemp(tempNow, tempNext);
      perc = int((float(tempNow) / float(tempNext)) * 100.00);
      mils = millis();
      if ( mils - wasmils >= 1000 ) { seconds ++; wasmils = mils; }
      timeCount = seconds;
      break;
    case 2:  // reflow
      seconds = 0;
      regulateTemp(tempNow, tempNext);
      perc = int((float(tempNow) / float(tempNext)) * 100.00);
      if (perc >= 100) {
        stateNow = 3;
        tSolder = millis();
        perc = 0;
        tempNext = 0;
      }
      mils = millis();
      if ( mils - wasmils >= 1000 ) { seconds2 ++; wasmils = mils; }
      timeCount = seconds2;       
      break;    
    case 3:  // cooling
      seconds2 = 0;
      digitalWrite(solidState, LOW);
      timeCount = int((tSolder + COOLDOWN_TIME * 1000 - millis()) / 1000);
      if (timeCount <= 0) {
        stateNow = 0;
      }      
      break;    
    default:  // off
      digitalWrite(solidState, LOW);
      timeCount = 0;
      seconds = 0;
      seconds2 = 0;
//      memset(lineArrayX,0,sizeof(lineArrayX));  // clear array to all zeros if state is off
//      memset(lineArrayY,1,sizeof(lineArrayY));  // clear array to all zeros if state is off
      break;    
  }

  // print state to display
  if (millis() > t + 200 || millis() < t) {
//    writeDisplay(state[stateNow], tempNext, tempNow, timeCount, perc);
      writeDisplay(stateNow, tempNext, tempNow, timeCount, perc);
    t = millis();
  }
}

void setup() {
  Serial.begin(115200);

  // button.setReleasedHandler(released);
  // button.setClickHandler(click);
  button.setLongClickDetectedHandler(click);
  button.setDoubleClickHandler(doubleClick);
  
  pinMode(solidState, OUTPUT);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  
  // wait for MAX chip to stabilize
  delay(500);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

}

void loop() {
  button.loop();  // button state reader
  tcReader();  // read thermocouple
  buttonAction();  // button state handler
  potMeterReader();  // read potentiometer value
  potMeterDraw();  // draw potmeter value on display  
}
