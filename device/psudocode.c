//Created by Justin Whittam Geskes and Raagave Thangeswaran for 4TR3 
#include "LowPower.h" 
struct Packet{
  uint16_t id;
  byte type;
  byte state;
  //etc
}
#define Value int 

void setup(){
  char* info = getInfo();
  packet = makePacket();
  send(setup);
  send(info);
  id = response();
}

// main loop
void loop(){
  Value value = readValue();
  // in practice the following is completly dependant on the protocol used to communicate with the gateway
  Packet packet = makePacket(value);
  sendPacket(packet);
  
  // Sleep for 8 s with ADC module and BOD module off
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
