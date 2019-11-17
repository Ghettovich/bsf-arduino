#include <EtherCard.h>
#include <IPAddress.h>

#define ETHERCARD_UDPSERVER   1
#define RELAY_ARRAY_SIZE   8

/* Network configuration */
// Ethernet interface IP address
static byte myip[] = { 192, 168, 178, 23 };
// Gateway IP address
static byte gwip[] = { 192, 168, 178, 1 };
// ethernet interface ip netmask
static byte mask[] = { 255, 255, 255, 0 };
// Ethernet MAC address - must be unique on your network
static byte mymac[] = { 0x70, 0x69, 0x74, 0x2D, 0x30, 0x31 };

// init adapter
byte Ethernet::buffer[2000]; // TCP/IP send and receive buffer
// server port
int port = 12310;

// Relay pin definitions
int relayValveLiftUp = 22;
int relayValveLiftDown = 24;
int relayValveBinLoad = 26;
int relayValveBinDrop = 28;
int relayValveFeederFwd_1 = 30;
int relayValveFeederRev_1 = 32;
int relayValveFeederFwd_2 = 34;
int relayValveFeederRev_2 = 36;

// INPUT proximity switches !!! if LOW detection !!!
int sensorLiftBottom = 25;
int sensorLiftTop = 27;

int relayArray[RELAY_ARRAY_SIZE];
const char *relayStates = "";
const char *returnMessage = "";
const char *messages[] = {"RELAY_STATES"
, "BELT_FORWARD"
, "BELT_REVERSE"
, "LIFT_UP"
, "LIFT_DOWN"
, "DUMP_BIN"
, "LOAD_BIN"
, "VALVE_FEEDER_FWD_1"
, "VALVE_FEEDER_REV_1"
, "VALVE_FEEDER_FWD_2"
, "VALVE_FEEDER_REV_2"};

// Lift at BOTTOM  BIN at LOAD
bool isLiftUpFree() {
  if (digitalRead(sensorLiftBottom) == LOW &&
      digitalRead(sensorLiftTop) == HIGH)
    return true;
  else
    return false;
}

// Lift at TOP  BIN at LOAD
bool isLiftDownFree() {
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == LOW)
    return true;
  else
    return false;
}
// Lift at TOP and BIN at DROP
bool isBinAtDrop() {
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == LOW)
    return true;
  else
    return false;
}

bool isBinAtLoad() {
  if (digitalRead(sensorLiftBottom) == LOW)
    return true;
  else
    return false;
}

// Lift  between sensors operator action required
bool isLiftBetweenBottomAndTop() {
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == HIGH)
    return true;
  else
    return false;
}

// BIN between sensors operator action required
bool isBinBetweenLoadAndDrop() {
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == LOW)
    return true;
  else
    return false;
}

bool isMessageLiftUp(String msg) {
  if (msg.equals("LIFT_UP")) {
    return true;
  }
  return false;
}

bool isMessageLiftDown(String msg) {
  if (msg.equals("LIFT_DOWN")) {
    return true;
  }
  return false;
}

bool isMessageDumpBin(String msg) {
  if (msg.equals("DUMP_BIN")) {
    return true;
  }
  return false;
}

bool isMessageLoadBin(String msg) {
  if (msg.equals("LOAD_BIN")) {
    return true;
  }
  return false;
}

bool isMessageFeederFwd_1(String msg) {
  if (msg.equals("VALVE_FEEDER_FWD_1")) {
    return true;
  }
  return false;
}

bool isMessageFeederRev_1(String msg) {
  if (msg.equals("VALVE_FEEDER_REV_1")) {
    return true;
  }
  return false;
}

bool isMessageFeederFwd_2(String msg) {
  if (msg.equals("VALVE_FEEDER_FWD_2")) {
    return true;
  }
  return false;
}

bool isMessageFeederRev_2(String msg) {
  if (msg.equals("VALVE_FEEDER_REV_2")) {
    return true;
  }
  return false;
}

bool isMessageRelayStates(String msg) {
  if (msg.equals("RELAY_STATES")) {
    return true;
  }
  return false;
}

bool isMessageInMessages(const char *msg) {
  String _msg = String(msg);

  for (auto c : messages) {
    _msg = String(c);
    if (_msg.equals(msg)) {
      Serial.println("found message");
      return true;
    }
  }
  return false;
}

void onLiftUpRelay() {
  if (!digitalRead(relayValveLiftUp) == LOW) {
    // Relay is OFF
    if (!isLiftUpFree()) {
      returnMessage = "0";
    }
    else {
      returnMessage = "1";
      digitalWrite(relayValveLiftUp, LOW);
    }
  }
  else {
    returnMessage = "0";
    digitalWrite(relayValveLiftUp, HIGH);
  }
}

void onLiftDownRelay() {
  if (!digitalRead(relayValveLiftDown) == LOW) {
    if (!isLiftDownFree())
    {
      returnMessage = "0";
    }
    else
    {
      returnMessage = "1";
      digitalWrite(relayValveLiftDown, LOW);
    }
  }
  else {
    returnMessage = "0";
    digitalWrite(relayValveLiftDown, HIGH);
  }
}

void onBinLoad() {
  if (!digitalRead(relayValveBinLoad) == LOW) {
    returnMessage = "1";
    digitalWrite(relayValveBinLoad, LOW);
  }
  else {
    returnMessage = "0";
    digitalWrite(relayValveBinLoad, HIGH);
  }
}

void onBinDrop() {
  if (!digitalRead(relayValveBinDrop) == LOW) {
    returnMessage = "1";
    digitalWrite(relayValveBinDrop, LOW);
  }
  else {
    returnMessage = "0";
    digitalWrite(relayValveBinDrop, HIGH);
  }
}

void onValveFeederFwd_1() {
  if (!digitalRead(relayValveFeederFwd_1) == LOW) {
    returnMessage = "1";
    digitalWrite(relayValveFeederFwd_1, LOW);
  }
  else {
    returnMessage = "0";
    digitalWrite(relayValveFeederFwd_1, HIGH);
  }
}

void onValveFeederRev_1() {
  if (!digitalRead(relayValveFeederRev_1) == LOW) {
    returnMessage = "1";
    digitalWrite(relayValveFeederRev_1, LOW);
  }
  else {
    returnMessage = "0";
    digitalWrite(relayValveFeederRev_1, HIGH);
  }
}

void onValveFeederFwd_2() {
  if (!digitalRead(relayValveFeederFwd_2) == LOW) {
    returnMessage = "1";
    digitalWrite(relayValveFeederFwd_2, LOW);
  }
  else {
    returnMessage = "0";
    digitalWrite(relayValveFeederFwd_2, HIGH);
  }
}

void onValveFeederRev_2() {
  if (!digitalRead(relayValveFeederRev_2) == LOW) {
    returnMessage = "1";
    digitalWrite(relayValveFeederRev_2, LOW);
  }
  else {
    returnMessage = "0";
    digitalWrite(relayValveFeederRev_2, HIGH);
  }
}

void onRequestRelayState() {  
  String stateMsg = "";
  returnMessage = "";  

  char stateCharArray[RELAY_ARRAY_SIZE + 1] = "";
  
  for (int i = 0; i < RELAY_ARRAY_SIZE; i++) {
    if(digitalRead(relayArray[i]) == LOW) {
      stateMsg.concat('1');
      stateCharArray[i] = '1';
      Serial.println("relay low");
    }
    else {
      stateMsg.concat('0');
      stateCharArray[i] = '0';
      Serial.println("relay high");
    }
  }
  //Serial.println(stateMsg.c_str());
  //strcat(returnMessage,stateMsg); 
  returnMessage = stateCharArray;
}

void onListenUdpConfig(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len) {
  IPAddress src(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

  if (isMessageInMessages(data)) {
    String msg = String(data);
    Serial.println(msg);

    if (isMessageLiftUp(msg)) {
      onLiftUpRelay();
    }
    else if (isMessageLiftDown(msg)) {
      onLiftDownRelay();
    }
    else if (isMessageDumpBin(msg)) {
      onBinDrop();
    }
    else if (isMessageLoadBin(msg)) {
      onBinLoad();
    }
    else if (isMessageFeederFwd_1(msg)) {
      onValveFeederFwd_1();
    }
    else if (isMessageFeederRev_1(msg)) {
      onValveFeederRev_1();
    }
    else if (isMessageFeederFwd_2(msg)) {
      onValveFeederFwd_2();
    }
    else if (isMessageFeederRev_2(msg)) {
      onValveFeederRev_2();
    }
    else if (isMessageRelayStates(msg)) {
      onRequestRelayState();
    }
  }
  else {
    returnMessage = "Unable to find message in messages";
    Serial.println("Unable to find message in messages");
  }
  Serial.println(returnMessage);
  ether.makeUdpReply(returnMessage, strlen(returnMessage), src_port);
}

void initializeRelayArray() {
  relayArray[0] = relayValveLiftUp;
  relayArray[1] = relayValveLiftDown;
  relayArray[2] = relayValveBinLoad;
  relayArray[3] = relayValveBinDrop;
  relayArray[4] = relayValveFeederFwd_1;
  relayArray[5] = relayValveFeederRev_1;
  relayArray[6] = relayValveFeederFwd_2;
  relayArray[7] = relayValveFeederRev_2;

  for (int i = 0; i < RELAY_ARRAY_SIZE; i++) {
    pinMode(relayArray[i], OUTPUT);
    if (!digitalRead(relayArray[i]) == HIGH) {
      digitalWrite(relayArray[i], HIGH);
    }
  }
}

void setup() {
  // Register network adapter
  Serial.begin(57600);
  Serial.println(F("\n[bsf-arduino]"));
  // Change 'SS' to your Slave Select pin if you aren't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println("Failed init adapter");
    
  ether.staticSetup(myip, gwip, NULL, mask);
  ether.udpServerListenOnPort(&onListenUdpConfig, port);

  // Register IO
  initializeRelayArray();
  //pinMode(sensorLiftTop, INPUT_PULLUP);
  //pinMode(sensorLiftBottom, INPUT_PULLUP);
  
}

void loop() {
  // This must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
}
