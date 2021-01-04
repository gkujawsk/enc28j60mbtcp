/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/gkujawsk/Documents/PlatformIO/Projects/enc/src/enc.ino"
/*
 * Project enc
 * Description:
 * Author:
 * Date:
 */

#include "Particle.h"
#include "lib/EtherCard.h"
#include <lib/ModbusTCP.h>
void chipSelectLow(uint8_t pin);
void chipSelectHigh();
static void gotPinged (byte* ptr);
void setup();
void loop();
#line 11 "/Users/gkujawsk/Documents/PlatformIO/Projects/enc/src/enc.ino"
#pragma GCC optimize ("O0")


SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(MANUAL);
STARTUP(cellular_credentials_set("playmetric", "", "", NULL)); 

Stash stash;



void chipSelectLow(uint8_t pin) {
    noInterrupts();
    pinResetFast(pin);
}

void chipSelectHigh() {
    pinSetFast(D5);
    interrupts();
}

static const byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static const byte myip[] = { 10,0,0,3 };
static const byte mask[] = { 255,255,255,0 };
static const byte gwip[] = { 10,0,0,1 };

static const byte hisip[] = { 10,0,0,1 };


bool ready = true;

byte Ethernet::buffer[200];   // a very small tcp/ip buffer is enough here

ModbusTCP node(1);                            // Unit Identifier.


// called when a ping comes in (replies to it are automatic)
static void gotPinged (byte* ptr) {
  ether.printIp(">>> ping from: ", ptr);
}

// setup() runs once, when the device is first turned on.
void setup() {
  // Particle.connect();
  // Put initialization like pinMode and begin functions here.
  Serial.begin(9600);
  while(!Serial.available()) {};
  Serial.read();
  Serial.println("[START]");

  Serial.println("\n[getStaticIP]");

  if (ether.begin(sizeof Ethernet::buffer, mymac, D5, chipSelectLow, chipSelectHigh) == 0)
    Serial.println("eth failed"); 
  ether.staticSetup(myip, gwip, NULL, mask);
  ether.persistTcpConnection(false);
  ether.registerPingCallback(gotPinged);
  while(!ether.isLinkUp()) {}
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  static uint32_t ctr = 0;
  static uint32_t ctout = 0;

    node.setTransactionID(ctr++); 
    node.setServerIPAddress(hisip);
    byte result = node.readInputRegisters(210, 2);

    if(result == 0) { // success
      Serial.print("RESPONSE: ");
      for(byte i = 0; i < node.getResponseBufferLength(); i++) {
        Serial.printf("[0x%02X] ", node.getResponseBuffer(i));
      }
      Serial.print("\n\r");
    } else {
      Serial.printlnf("ERROR: 0x%04X", result);
      while(!Serial.available()) {};
      Serial.read();
    }
    node.clearResponseBuffer();

  ctout = millis();
  while(millis() - ctout < 500 ) 
  {
    ether.packetLoop(ether.packetReceive());
    // if(ether.tcpClosed()) break;
  }
  // ether.packetReceive();

}