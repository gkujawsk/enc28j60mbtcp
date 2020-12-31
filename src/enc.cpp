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

static void gotPinged (byte* ptr);
void setup();
void loop();
#line 11 "/Users/gkujawsk/Documents/PlatformIO/Projects/enc/src/enc.ino"
SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(MANUAL);

Stash stash;


static const byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static const byte myip[] = { 10,10,10,10 };
static const byte mask[] = { 255,255,255,0 };
static const byte gwip[] = { 10,10,10,1 };

static const byte hisip[] = { 10,10,10,1 };

const char query[128] = {0x41};


static byte session;

bool ready = true;


byte Ethernet::buffer[200];   // a very small tcp/ip buffer is enough here
static long timer;

// called when a ping comes in (replies to it are automatic)
static void gotPinged (byte* ptr) {
  ether.printIp(">>> ping from: ", ptr);
}

// setup() runs once, when the device is first turned on.
void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial.begin(9600);
  while(!Serial.available()) {};
  Serial.read();
  Serial.println("[START]");

  Serial.println("\n[getStaticIP]");

  if (ether.begin(sizeof Ethernet::buffer, mymac, D5) == 0)
    Serial.println("Failed to access Ethernet controller"); 
  ether.staticSetup(myip, gwip, NULL, mask);
  ether.copyIp(ether.hisip, hisip);
  ether.hisport = 8080;
  ether.printIp("Server: ", ether.hisip);
  ether.persistTcpConnection(false);
  ether.registerPingCallback(gotPinged);
  // stash.prepare("GET / HTTP/1.1");
  // session = ether.tcpSend();
  // Serial.printlnf("SESSION NO: %d", session);

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  static uint32_t ctout = 0;

  if(ether.isLinkUp() && ready) {
    ready = false;
    stash.prepare(query);
    session = ether.tcpSend();
    ctout = millis();
    Serial.printlnf("SESSION NO: %d", session);
  }

  if(ctout != 0 && millis() - ctout > 5000) {
    Serial.println("connection timeout");
    ctout = 0;
    ready = true;
  }

  // The core of your code will likely live here.
  word len = ether.packetReceive(); // go receive new packets
  word pos = ether.packetLoop(len); // respond to incoming pings
  const char* reply = ether.tcpReply(session);
    if (reply != 0) {
      Serial.println(len);
    for(uint16_t i=0; i < len; i++) {
      if(i!=0 && i%16 == 0) Serial.print("\r\n");
      if(i!=0 && i%8 == 0 && i%16 != 0) Serial.print("\t");
      Serial.printf("[0x%02x] ", Ethernet::buffer[i]);
    }
    Serial.print("\r\n\n");
    ready = true;
  }

  if(ready) {
    uint32_t tout = millis();
    while(millis() - tout < 2000) {ether.packetLoop(ether.packetReceive());};
  } 

}