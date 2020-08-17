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


#define IO_USERNAME "NukeHunter"
#define IO_KEY "yourkeyhere"
#define WIFI_SSID "yourssidhere"
#define WIFI_PASS "yourpasswordhere"
#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

#define LEDPin 2
#define inverterPin 27
int cell = 0;
int analogIn[] = {36, 39, 34, 35};
int dischargePin[] = {32, 33, 25, 26};
bool dischargeState[] = {false, false, false, false}; 
unsigned int sensorInt;
float sensorValue = 0;
float allowedCellDiff = 0.04;  /// 0.02 for big pack?
float lowBatt = 10;
unsigned long lastTxTime = 0;
unsigned int minTxTime = 60000;
//unsigned int sampleTime = 1000;
String debugText = "0.00 0.00 0.00 0.00 ";
//unsigned int dischargeTime[] = {0, 0, 0, 0};
int i;
float battVolt[4];
float cellVolt[4];
// bool inverterControl = false;  // not used yet

AdafruitIO_Feed *espbattery = io.feed("espbattery");
AdafruitIO_Feed *espbattery2 = io.feed("espbattery2");
AdafruitIO_Feed *espbattery3 = io.feed("espbattery3");
AdafruitIO_Feed *espbattery4 = io.feed("espbattery4");
//AdafruitIO_Feed *debugtext = io.feed("debug-text");
AdafruitIO_Feed *inverterstatus = io.feed("inverterstatus");
AdafruitIO_Feed *battdischarge1 = io.feed("battdischarge1");
AdafruitIO_Feed *battdischarge2 = io.feed("battdischarge2");
AdafruitIO_Feed *battdischarge3 = io.feed("battdischarge3");
AdafruitIO_Feed *battdischarge4 = io.feed("battdischarge4");
//AdafruitIO_Feed *battdischargetime4 = io.feed("battdischargetime4");
AdafruitIO_Feed *packvoltage = io.feed("packvoltage");

void setup() {
  pinMode(inverterPin, OUTPUT);
  pinMode(LEDPin, OUTPUT);
  analogSetClockDiv(20);
  for (cell=1;cell<5;cell++) {
    pinMode(dischargePin[cell-1], OUTPUT);
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

  inverterstatus->onMessage(inverterStatusChange);
  inverterstatus->get();
}

void loop() {
  io.run();

  updateVoltage();

  //Check for over discharged cell.
  for (cell=1;cell<5;cell++) {
    if (cellVolt[cell-1] < 2.5) {
      digitalWrite(dischargePin[cell-1], false);
      dischargeState[cell-1] = false;
      digitalWrite(inverterPin, false);
      inverterstatus->save(false);
      Serial.print("Battery "); Serial.print(cell); Serial.println(" Forced draining off...and inverter forced off.");
    }
  }

  
  // Blink to show sampling
  digitalWrite(LEDPin, HIGH); delay(30); digitalWrite(LEDPin, LOW);

  
// Do this every minute

  if(millis() > lastTxTime + minTxTime)
  {
    lastTxTime = millis();

    digitalWrite(LEDPin, HIGH);  // Long blink means transmitting and checking (non-balancing voltage)
    
    // Turn off discharging circuits.
    for (cell=1;cell<5;cell++) { 
      digitalWrite(dischargePin[cell-1], false);
    }

    delay(500); // Give cells time to settle.

    updateVoltage();

    // Get Low Cell Voltage
    lowBatt = 10; for (i=0;i<4;i++) { if (cellVolt[i] < lowBatt) { lowBatt = cellVolt[i]; } }
    Serial.print("Low Batt is at ");Serial.println(lowBatt);

    if (lowBatt > 3.5) { digitalWrite(inverterPin,true); }  // If battery has some juice, turn on the inverter.
  
    // Check for balance and turn on/off balancers
    for (cell=1;cell<5;cell++) {
  
      if ((cellVolt[cell-1] > (lowBatt + allowedCellDiff)) && (cellVolt[cell-1] > 3.0)) {
        digitalWrite(dischargePin[cell-1], true);
        dischargeState[cell-1] = true;
        Serial.print("Battery ");Serial.print(cell);Serial.println(" Draining");
      }
      else {
        digitalWrite(dischargePin[cell-1], false);
        dischargeState[cell-1] = false;
        Serial.print("Battery ");Serial.print(cell);Serial.println(" Not Draining");
      }

//      if (dischargeState[cell-1]) { dischargeTime[cell-1]++; } // Track minutes of discharging.

    }

    
    updateAdafruit();  // Here so we have accurate voltages as well as discharge status.
    
    digitalWrite(LEDPin, LOW);

  }

  delay(200); //Adjust to keep sample time around 1 second.
}


void updateAdafruit() {
  Serial.println("Sending Data");

  espbattery->save(cellVolt[1-1]);
  espbattery2->save(cellVolt[2-1]);
  espbattery3->save(cellVolt[3-1]);
  espbattery4->save(cellVolt[4-1]);
  inverterstatus->save(digitalRead(inverterPin));
//  debugtext->save(debugText);
  battdischarge1->save(dischargeState[1-1]);
  battdischarge2->save(dischargeState[2-1]);
  battdischarge3->save(dischargeState[3-1]);
  battdischarge4->save(dischargeState[4-1]);
//  battdischargetime4->save(dischargeTime[4-1]);
  packvoltage->save(battVolt[4-1]);


  //    inverter->get();  Get inverter status someday?
}


void updateVoltage() {
  
  for (cell=1;cell<5;cell++) {
 
    sensorInt = 0;
    for (i=0;i<10;i++) { analogRead(analogIn[cell-1]); }
    for (i=0;i<100;i++) {
      delay(3);
      sensorInt = sensorInt + analogRead(analogIn[cell-1]);
    }

    sensorInt = sensorInt / 100;

    if (cell == 1) {sensorValue = (sensorInt * 2.193 * 3.3) / 4096;  }
    else if (cell == 2) {sensorValue = (sensorInt * 3.471 * 3.3) / 4096;  }
    else if (cell == 3) {sensorValue = (sensorInt * 4.452 * 3.3) / 4096;  }
    else {sensorValue = (sensorInt * 6.225 * 3.3) / 4096;  }
    
    sensorInt = (sensorValue * 100);     // Round to two decimal places...
    sensorValue = sensorInt;
    sensorValue = sensorValue / 100;

    battVolt[cell-1] = sensorValue;

    cellVolt[cell-1] = (battVolt[cell-1]-(battVolt[cell-1-1]*(cell!=1)));

  }

  
      debugText = "";
      for (i=0;i<4;i=i+1) {
        Serial.print(battVolt[i]); Serial.print(" ");
        debugText += String(battVolt[i]);
        debugText += String(" ");
      }
      Serial.println();
}



void inverterStatusChange(AdafruitIO_Data *data) {
//  inverterStatus = data->toInt();
Serial.print("Inverter: ");Serial.println(data->toPinLevel());
  digitalWrite(inverterPin,data->toPinLevel());
}
