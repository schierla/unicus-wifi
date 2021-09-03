// WLAN-Gateway für Unicus-Liedanzeiger
// Copyright (C) 2020-2021 Andreas Schierl

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <ESP8266WiFi.h>
#define SDO D6
#define SDI D7
#define SCK D5
#define NSEL D1
#define NIRQ D2
#define LED D4

// available on request
#define SEND_DATA(data) ;

// copy WifiSettings.h.example to WifiSettings.h and 
// customize it with your WiFi credentials
#include "WifiSettings.h"

#define PORT 2022
#define MAX_CLIENTS 5
#define RX_LEN 16
WiFiServer server(PORT);
WiFiClient clients[MAX_CLIENTS];
String rxbuf[MAX_CLIENTS];

void setup() {
  pinMode(SDO, INPUT);
  pinMode(SDI, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(NSEL, OUTPUT);
  pinMode(NIRQ, INPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(SDI, HIGH);
  digitalWrite(SCK, LOW);
  digitalWrite(NSEL, HIGH);

  // start wifi stuff
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  server.begin();

  delay(100);
  Serial.begin(115200);
  Serial.println("Starting up...");

  setupForReceive();
  Serial.println("Ready...");
  Serial.println("");
}

void setupForReceive() {
  writeRegister(0x07, 0xF0); // Operating & Function Control 1
  writeRegister(0x07, 0x00); // Operating & Function Control 1
  writeRegister(0x05, 0x00); // Interrupt Enable 1
  writeRegister(0x06, 0x00); // Interrupt Enable 2
  writeRegister(0x09, 0x7F); // Crystal Oscillator Load Capacitance
  writeRegister(0x0A, 0x02); // Microcontroller Output Clock
  writeRegister(0x0B, 0x00); // GPIO0 Configuration
  writeRegister(0x0C, 0x00); // GPIO1 Configuration
  writeRegister(0x0D, 0x00); // GPIO2 Configuration
  writeRegister(0x0E, 0x00); // I/O Port Configuration
  writeRegister(0x0F, 0x70); // ADC Configuration
  writeRegister(0x10, 0x00); // ADC Sensor Amplifier Offset
  writeRegister(0x12, 0x00); // Temperature Sensor Control
  writeRegister(0x13, 0x00); // Temperature Value Offset
  writeRegister(0x1C, 0x1D); // IF Filter Bandwidth
  writeRegister(0x1D, 0x40); // AFC Loop Gearshift Override
  writeRegister(0x1E, 0x0A); // AFC Timing Control
  writeRegister(0x1F, 0x03);
  writeRegister(0x20, 0x41); // Clock Recovery Oversampling Ratio
  writeRegister(0x21, 0x60); // Clock Recovery Offset 2
  writeRegister(0x22, 0x27); // Clock Recovery Offset 1
  writeRegister(0x23, 0x52); // Clock Recovery Offset 0
  writeRegister(0x24, 0x00); // Clock Recovery Timing Loop Gain 1
  writeRegister(0x25, 0x06); // Clock Recovery Timing Loop Gain 0
  writeRegister(0x27, 0x80); // RSSI Threshold for Clear Channel Indicator
  writeRegister(0x2A, 0x1E);
  writeRegister(0x30, 0x8C); // Data Access Control
  writeRegister(0x32, 0x0F); // Header Control 1
  writeRegister(0x33, 0x42); // Header Control 2
  writeRegister(0x34, 0x10); // Preamble Length
  writeRegister(0x35, 0x2A); // Preamble Detection Control
  writeRegister(0x36, 0x2D); // Sync Word 3
  writeRegister(0x37, 0xD4); // Sync Word 2
  writeRegister(0x38, 0xAA); // Sync Word 1
  writeRegister(0x39, 0xAA); // Sync Word 0
  writeRegister(0x3A, 0x68); // Transmit Header 3
  writeRegister(0x3B, 0x6F); // Transmit Header 2
  writeRegister(0x3C, 0x70); // Transmit Header 1
  writeRegister(0x3D, 0x65); // Transmit Header 0
  writeRegister(0x3E, 0x12); // Transmit Packet Length
  writeRegister(0x3F, 0x68); // Check Header 3
  writeRegister(0x40, 0x6F); // Check Header 2
  writeRegister(0x41, 0x70); // Check Header 1
  writeRegister(0x42, 0x65); // Check Header 0
  writeRegister(0x43, 0xFF); // Header Enable 3
  writeRegister(0x44, 0xFF); // Header Enable 2
  writeRegister(0x45, 0xFF); // Header Enable 1
  writeRegister(0x46, 0xFF); // Header Enable 0
  writeRegister(0x69, 0x60); // AGC Override 1
  writeRegister(0x6D, 0x0F); // TX Power
  writeRegister(0x70, 0x2C); // Modulation Mode Control 1
  writeRegister(0x71, 0x22); // Modulation Mode Control 2
  writeRegister(0x72, 0x48); // Frequency Deviation
  writeRegister(0x73, 0x00); // Frequency Offset 1
  writeRegister(0x74, 0x00); // Frequency Offset 2
  writeRegister(0x75, 0x73); // Frequency Band Select
  writeRegister(0x76, 0x77); // Nominal Carrier Frequency 1
  writeRegister(0x77, 0x4F); // Nominal Carrier Frequency 0
  writeRegister(0x79, 0x00); // Frequency Hopping Channel Select
  writeRegister(0x7A, 0x00); // Frequency Hopping Step Size
  writeRegister(0x6E, 0x13); // TX Data Rate 1
  writeRegister(0x6F, 0xA9); // TX Data Rate 0
  writeRegister(0x08, 0x03); // Operating & Function Control 2
  writeRegister(0x08, 0x00); // Operating & Function Control 2
  writeRegister(0x05, 0x03); // Interrupt Enable 1
  writeRegister(0x07, 0x06); // Operating & Function Control 1
}


String letters = "01ACEFHLPU";
String numbers = "0123456789";
String prev = "";

bool doSend(String number) {
  unsigned char data[12];
  data[0] = 0x0F;
  data[1] = 0x1F;
  data[2] = 0x2F;
  data[3] = 0x3F;
  data[4] = 0x4F;
  data[5] = 0x5F;
  data[6] = 0x60;
  data[7] = 0x7F;
  data[8] = 0x8F;
  data[9] = 0x9F;

  int numberStart = 0; 
  int numberEnd = number.length();
  int stropheStart = number.length();
  int stropheEnd = number.length();
  int digit = 0;
  if(number.indexOf(", ") != -1) {
    numberEnd = number.indexOf(", ");
    stropheStart = numberEnd + 1;
  }
  if(numberEnd > 0) {
    int pos = letters.indexOf(number.charAt(0));
    if(pos > 1) { 
      data[8] = 0x80 + pos; 
      numberStart++;
    }
    for(int i = numberEnd - 1; i >= numberStart; i--) {
      if(number.charAt(i) == '.') {
        if(i >= numberEnd - 2) {
          data[6] |= 4;
          digit = 1;
        } else {
          return false;
        }
      } else if(number.charAt(i) == ' ') {
        digit++;        
      } else {
        pos = numbers.indexOf(number.charAt(i));
        if(pos > -1) {
          switch(digit) {
            case 0: data[2] = 0x20 + pos; break;
            case 1: data[1] = 0x10 + pos; break;
            case 2: data[0] = 0x00 + pos; break;
            case 3: data[9] = 0x90 + pos; break;
            default: return false;
          }
          digit++;
        } else {
          return false;
        }
      }
    }
  }
  digit = 0;
  for(int i = stropheEnd - 1; i > stropheStart; i--) {
    if(number.charAt(i) == '+') {
      if(digit == 0 || digit == 1) {
        if(data[4] == 0x41) data[4] = 0x48; 
        else if(data[4] == 0x4f) data[4] = 0x40; 
        else return false;
        digit = 1;
      } else if(digit == 2) {
        if((data[6] & 1) == 0) data[6] |= 1; 
        else return false;
      } else {
        return false;
      }
    } else if(number.charAt(i) == '-') {
      if(digit == 0 || digit == 1) {
        if(data[4] == 0x41) data[4] = 0x49; 
        else if(data[4] == 0x4f) data[4] = 0x47; 
        else return false;
        digit = 1;
      } else if(digit == 2) {
        if((data[6] & 2) == 0) data[6] |= 2;
        else return false;
      } else {
        return false;
      }
    } else if(number.charAt(i) == ' ') {
      digit++;
    } else {
      if(number.charAt(i) == '1' && digit == 1 && data[4] == 0x4f) {
        data[4] = 0x41;
      } else {
        int pos = numbers.indexOf(number.charAt(i));
        if(pos > -1) {
          switch(digit) {
            case 0: data[5] = 0x50 + pos; break;
            case 1: data[3] = 0x30 + pos; break;
            case 2: data[7] = 0x70 + pos; break;  
            default: return false;
          }
          digit++;
        } else {
          return false;
        }
      }
    }
  }
  
  int crc = crc16(data, 10);
  data[10] = (crc & 0xFF);
  data[11] = ((crc >> 8) & 0xFF);

  SEND_DATA(data);
}

void doReceive() {
  unsigned char data[12];
  bool valid = false;
  byte int1 = readRegister(0x03); // Interrupt Status 1
  byte int2 = readRegister(0x04); // Interrupt Status 2
  if (int1 & 0x02) {
    int i = 0;
    while(i < 12) {
      if(readRegister(0x02) & 0x20) break;
      data[i] = readRegister(0x7F);
      i++;
    }
    if((readRegister(0x02) & 0x20) && i == 12) {
      int crc = crc16(data, 10);
      if(((crc & 0xFF) == (data[10] & 0xFF)) && 
          (((crc >> 8) & 0xFF) == (data[11] & 0xFF))) {
        valid = true;
        String lied = "";
        if(data[8] >= 0x80 && data[8] <= 0x89) lied += (letters.charAt(data[8] - 0x80));
        if(data[9] >= 0x90 && data[9] <= 0x99) lied += (numbers.charAt(data[9] - 0x90));
        if(data[0] >= 0x00 && data[0] <= 0x09) lied += (numbers.charAt(data[0] - 0x00));
        if(data[1] >= 0x10 && data[1] <= 0x19) lied += (numbers.charAt(data[1] - 0x10));
        if((data[6] & 0xF4) == 0x64) lied += (".");
        if(data[2] >= 0x20 && data[2] <= 0x29) lied += (numbers.charAt(data[2] - 0x20));
        String strophe = "";
        if(data[7] >= 0x70 && data[7] <= 0x79) strophe += (numbers.charAt(data[7] - 0x70));
        if((data[6] & 0xF3) == 0x61) strophe += ("+");
        if((data[6] & 0xF3) == 0x62) strophe += ("-");
        if(data[3] >= 0x30 && data[3] <= 0x39) strophe += (numbers.charAt(data[3] - 0x30));
        if(data[4] == 0x40) strophe += ("+");
        if(data[4] == 0x41) strophe += ("1");
        if(data[4] == 0x47) strophe += ("-");
        if(data[4] == 0x48) strophe += ("+1");
        if(data[4] == 0x49) strophe += ("-1");
        if(data[5] >= 0x50 && data[5] <= 0x59) strophe += (numbers.charAt(data[5] - 0x50));
        if(strophe != "") lied = lied + ", " + strophe;
        if(lied != prev) {
          reportNumber(lied);
          prev = lied;
        }
      }
    }

    if(!valid) {
      Serial.print("* ERROR PARSING ");
      for(int j=0; j<i; j++) {
        Serial.print(data[j], HEX); Serial.print(" ");
      }
      Serial.println(" *");
    }
    writeRegister(0x08, 0x02); // Operating & Function Control 1
    writeRegister(0x08, 0x00); // Operating & Function Control 1
    writeRegister(0x07, 0x06); // Operating & Function Control 1
  }
}

void reportNumber(String number) {
  Serial.println(number);
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if(clients[i] && clients[i].availableForWrite()) {
      clients[i].println(number);
    } 
  }
}

unsigned short crc16(const unsigned char* data_p, unsigned char length){
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc ^ 0xE304;
}

void writeRegister(byte reg, byte value) {
  digitalWrite(NSEL, LOW);
  delayMicroseconds(5);
  shiftOut(SDI, SCK, MSBFIRST, reg | 0x80);
  shiftOut(SDI, SCK, MSBFIRST, value);
  delayMicroseconds(5);
  digitalWrite(NSEL, HIGH);
}

byte readRegister(byte reg) {
  digitalWrite(NSEL, LOW);
  delayMicroseconds(5);
  shiftOut(SDI, SCK, MSBFIRST, reg & 0x7F);
  byte value = shiftIn(SDO, SCK, MSBFIRST);
  delayMicroseconds(5);
  digitalWrite(NSEL, HIGH);
  return value;
}

void serverHandle() {
  if(server.hasClient()) {
    int i;
    for(i = 0; i < MAX_CLIENTS; i++) {
      if(!clients[i]) {
        clients[i] = server.available();
        rxbuf[i] = "";
        clients[i].println("READY");
        clients[i].println("");
        break;
      }
    }
    if(i == MAX_CLIENTS) {
      server.available().println("BUSY");
    }
  }
  
  for(int i = 0; i < MAX_CLIENTS; i++) {
    if(clients[i].available()) {
      int ch = clients[i].read();
      if(ch == '\r') {
        // ignore
      } else if(ch == '\n') {
        bool success = (rxbuf[i].length() < RX_LEN) && doSend(rxbuf[i]);
        if(success) {
          clients[i].println("* OK");
          reportNumber(rxbuf[i]);
        } else {
          clients[i].println("* INVALID");
        }
        rxbuf[i] = "";
      } else if(rxbuf[i].length() >= RX_LEN) {
      } else {
        rxbuf[i] += (char)ch; 
      }
    }
  }
}

void loop() {
  serverHandle();
  if (digitalRead(NIRQ) == LOW) {
    digitalWrite(LED, LOW);
    doReceive();
    digitalWrite(LED, HIGH);
  }
}
