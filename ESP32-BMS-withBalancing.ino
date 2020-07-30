// Tutorial Link: https://learn.adafruit.com/adafruit-io-basics-analog-input
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Some written by Todd Treece for Adafruit Industries
// The rest written by William Burk for himself.
// Sorry Todd, for the mess.
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

#define IO_USERNAME "yourusernamehere"
#define IO_KEY "yourkeyhere"
#define WIFI_SSID "yourSSID"
#define WIFI_PASS "yourwifipass"
#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

#define LEDPin 2
int cell = 0;
int analogIn[] = {36, 39, 34, 35};
int dischargePin[] = {32, 33, 25, 26};
bool dischargeState[] = {false, false, false, false}; 
int sensorInt;
float sensorValue = 0;
float allowedCellDiff = 0.02;
float lowBatt = 10;
unsigned long lastTxTime = 0;
unsigned int minTxTime = 60000;
unsigned int sampleTime = 1000;
String debugText = "0.00 0.00 0.00 0.00 ";
unsigned int dischargeTime[] = {0, 0, 0, 0};
int i;
float battVolt[4];
float cellVolt[4];
AdafruitIO_Feed *espbattery = io.feed("espbattery");
AdafruitIO_Feed *espbattery2 = io.feed("espbattery2");
AdafruitIO_Feed *espbattery3 = io.feed("espbattery3");
AdafruitIO_Feed *espbattery4 = io.feed("espbattery4");
AdafruitIO_Feed *debugtext = io.feed("debug-text");
AdafruitIO_Feed *battdischarge4 = io.feed("battdischarge4");
AdafruitIO_Feed *battdischargetime4 = io.feed("battdischargetime1");

void setup() {
  pinMode(LEDPin, OUTPUT);
  for (cell=1;cell<5;cell++) { 
    pinMode(dischargePin[cell-1], OUTPUT); // 0 references are annoying..you'll see -1 throughout.
  }

  for (i=0;i<50;i=i+1) { analogRead(analogIn[1-1]); }  // Prevent first couple of reads from being anomalous.

  Serial.begin(115200);

  Serial.print("Connecting to Adafruit IO");
  io.connect();   // connect to io.adafruit.com

  while(io.status() < AIO_CONNECTED) {    // wait for a connection
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

//  inverter->onMessage(inverter);   //saving these for when I add inverter control.
//  inverter->get();
}

void loop() {
  io.run();

  updateVoltage();

  lowBatt = 10;
  for (i=0;i<4;i++) {
    if (cellVolt[i] < lowBatt) { lowBatt = cellVolt[i]; }
  }

  digitalWrite(LEDPin, HIGH);
  delay(30);
  digitalWrite(LEDPin, LOW);
  
      debugText = "";
//      for (i=0;i<4;i=i+1) {
//        Serial.print(battVolt[i]); Serial.print(" ");
//        debugText += String(battVolt[i]);
//        debugText += String(" ");
//      }
      for (i=0;i<4;i=i+1) {
        Serial.print(cellVolt[i]); Serial.print(" ");
        debugText += String(cellVolt[i]);
        debugText += String(" ");
      }
      Serial.println();

  if(millis() > lastTxTime + minTxTime)
  {
    lastTxTime = millis();
    Serial.print("Sent Last");

    digitalWrite(LEDPin, HIGH);
    
    espbattery->save(cellVolt[1-1]);
    espbattery2->save(cellVolt[2-1]);
    espbattery3->save(cellVolt[3-1]);
    espbattery4->save(cellVolt[4-1]);

    debugtext->save(debugText);
    battdischarge4->save(dischargeState[4-1]);
    battdischargetime4->save(dischargeTime[4-1]);

    for (cell=4;cell<5;cell++) {  // only discharge testing cell 4 presently.
      if (cellVolt[cell-1] <= lowBatt || cellVolt[cell-1] < 3.0) {
        digitalWrite(dischargePin[cell-1], false);
        dischargeState[cell-1] = false;
        Serial.print("Battery "); Serial.print(cell); Serial.println(" not Draining");
      }
    }
    
    delay(100); // Long blink means transmitting.  Wait to give comms more time?

    digitalWrite(LEDPin, LOW);

//    inverter->get();  Get inverter status someday?

  }

  for (cell=4;cell<5;cell++) {  // just cell 4 for now.
    if (dischargeState[cell-1]) { dischargeTime[cell-1]++; }
  
    if ((millis() > 30000) && (cellVolt[cell-1] > (lowBatt + allowedCellDiff)) && (cellVolt[cell-1] > 3.0)) {
      digitalWrite(dischargePin[cell-1], true);
      dischargeState[cell-1] = true;
      Serial.print("Battery ");Serial.print(cell);Serial.println(" Draining");
    }
  }
  delay(sampleTime-250);

}

void updateVoltage() {

  for (cell=1;cell<5;cell++) {
 
    digitalWrite(dischargePin[cell-1], false);
    sensorInt = 0;
    for (i=0;i<5;i++) { analogRead(analogIn[cell-1]); }
    for (i=0;i<10;i++) {
      delay(15);
      sensorInt = sensorInt + analogRead(analogIn[cell-1]);
    }
    digitalWrite(dischargePin[cell-1], dischargeState[cell-1]);
    sensorInt = sensorInt / 10;
//  Serial.print(sensorInt);Serial.print(" ");
    if (cell == 1) {sensorValue = (sensorInt * 2.152 * 3.3) / 4096;  }
    else if (cell == 2) {sensorValue = (sensorInt * 3.348 * 3.3) / 4096;  }
    else if (cell == 3) {sensorValue = (sensorInt * 4.368 * 3.3) / 4096;  }
    else {sensorValue = (sensorInt * 5.679 * 3.3) / 4096;  }
    sensorInt = (sensorValue * 100);     // Round to two decimal places...
    sensorValue = sensorInt;
    sensorValue = sensorValue / 100;
  
    battVolt[cell-1] = sensorValue;

    cellVolt[cell-1] = (battVolt[cell-1]-(battVolt[cell-1-1]*(cell!=1)));

  }

//  cellVolt[1-1] = battVolt[1-1];
//  cellVolt[2-1] = (battVolt[2-1]-battVolt[1-1]);
//  cellVolt[3-1] = (battVolt[3-1]-battVolt[2-1]);
//  cellVolt[4-1] = (battVolt[4-1]-battVolt[3-1]);
}



//void inverter(AdafruitIO_Data *data) {
//  inverterStatus = data->toFloat();
//}
