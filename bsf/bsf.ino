#include <EtherCard.h>
#include <IPAddress.h>

#define ETHERCARD_UDPSERVER   1
#define RELAY_ARRAY_SIZE      8
#define MESSAGE_ARRAY_SIZE   11

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
int sensorLiftBottom = 40;
int sensorLiftTop = 42;

int relayArray[RELAY_ARRAY_SIZE];
const char *relayStates = "";
const char *returnMessage = "";
const char *messages[MESSAGE_ARRAY_SIZE] = {"LIFT_UP"
                                            , "LIFT_DOWN"
                                            , "BIN_LOAD"
                                            , "BIN_DROP"
                                            , "VALVE_FEEDER_FWD_1"
                                            , "VALVE_FEEDER_REV_1"
                                            , "VALVE_FEEDER_FWD_2"
                                            , "VALVE_FEEDER_REV_2"
                                            , "BELT_FORWARD"
                                            , "BELT_REVERSE"
                                            , "RELAY_STATE"
                                           };

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
    if (digitalRead(relayArray[i]) == LOW) {
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
  
  returnMessage = stateCharArray;
}


void onListenUdpMessage(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len) {
  IPAddress src(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

  String _msg = String(data);

  for (int i = 0; i < MESSAGE_ARRAY_SIZE; i++) {
    String c = String(messages[i]);

    if (_msg.equals(c)) {
      switch (i) {
        case 0:
          onLiftUpRelay();
          break;
        case 1:
          onLiftDownRelay();
          break;
        case 2:
          onBinLoad();
          break;
        case 3:
          onBinDrop();
          break;
        case 4:
          onValveFeederFwd_1();
          break;
        case 5:
          onValveFeederRev_1();
          break;
        case 6:
          onValveFeederFwd_2();
          break;
        case 7:
          onValveFeederRev_2();
          break;
        case 8:
          //reserved for belt fwd
          break;
        case 9:
          //reserved for belt rev
          break;
        // !! Will eventually be 17 when other relay block is connected !!
        case 10:
          onRequestRelayState();
          break;
      }
    }
  }
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
  //ether.udpServerListenOnPort(&onListenUdpConfig, port);
  ether.udpServerListenOnPort(&onListenUdpMessage, port);

  // Register IO
  initializeRelayArray();
  pinMode(sensorLiftTop, INPUT_PULLUP);
  pinMode(sensorLiftBottom, INPUT_PULLUP);

}

void loop() {
  // This must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
}
