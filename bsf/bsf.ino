#include <EtherCard.h>
#include <IPAddress.h>

#define ETHERCARD_UDPSERVER   1

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

/* IO */
int relayValveLiftUp = 25;
int relayValveLiftDown = 27;
int relayBinLoad = 24;
int relayBinDrop = 26;
int relayTest = 30;

// INPUT proximity switches !!! if LOW detection !!!
int sensorLiftBottom = 22;
int sensorLiftTop = 23;
//int sensorBinLoad = 26;
//int sensorBinDrop = 28;

const char *returnMessage = "";
const char *messages[] = {"BELT_FORWARD", "BELT_REVERSE", "DOSE_VALVE_SAND", "LIFT_UP", "LIFT_DOWN", "DUMP_BIN", "LOAD_BIN", "START_MIXER", "STEP_UP_MIXER_1"};

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
  if (!digitalRead(relayBinLoad) == LOW) {
    returnMessage = "1";
    digitalWrite(relayBinLoad, LOW);
  }
  else {
    returnMessage = "0";
    digitalWrite(relayBinLoad, HIGH);
  }
}

void onBinDrop() {
  if (!digitalRead(relayBinDrop) == LOW) {
    returnMessage = "1";
    digitalWrite(relayBinDrop, LOW);
  }
  else {
    returnMessage = "0";
    digitalWrite(relayBinDrop, HIGH);
  }
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
  }
  else {
    returnMessage = "Unable to find message in messages";
    Serial.println("unable to find message in messages");
  }
  Serial.println(returnMessage);
  ether.makeUdpReply(returnMessage, strlen(returnMessage), src_port);
}

void setup() {
  // Register network adapter
  Serial.begin(57600);
  Serial.println(F("\n[webClient]"));
  // Change 'SS' to your Slave Select pin if you aren't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println("Failed init adapter");
  ether.staticSetup(myip, gwip, NULL, mask);

  // copy webRelay to hisip so it knows where to send the reply
  // TODO: INSPECT WHEN REPLYING, line should is not necessary?? IP is known on reply
  //ether.copyIp(ether.hisip, webRelayIP);
  // Register udpSerialPrint() to port
  ether.udpServerListenOnPort(&onListenUdpConfig, port);

  // Register IO
  //pinMode(sensorLiftTop, INPUT_PULLUP);
  //pinMode(sensorLiftBottom, INPUT_PULLUP);

  pinMode(relayValveLiftUp, OUTPUT);
  digitalWrite(relayValveLiftUp, HIGH);

  pinMode(relayValveLiftDown, OUTPUT);
  digitalWrite(relayValveLiftDown, HIGH);

  pinMode(relayBinLoad, OUTPUT);
  digitalWrite(relayBinLoad, HIGH);

  pinMode(relayBinDrop, OUTPUT);
  digitalWrite(relayBinDrop, HIGH);

  pinMode(relayTest, OUTPUT);
  digitalWrite(relayTest, HIGH);
  
}

void loop() {
  // This must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
}
