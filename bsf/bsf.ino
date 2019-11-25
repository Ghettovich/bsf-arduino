#include <EtherCard.h>
#include <IPAddress.h>

#define ETHERCARD_UDPSERVER   1
#define RELAY_ARRAY_SIZE      8
#define SENSOR_ARRAY_SIZE     2
#define MESSAGE_ARRAY_SIZE   18

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
int sensorLiftBottom = 40, sensorLiftTop = 42;

// Relay States
int relayLiftUpState = 0, relayLiftBottomState = 0;
int lastRelayLiftUpState = 0, lastRelayLiftBottomState = 0;

// Sensor States
int sensorLiftBottomState = 0, sensorLiftTopState = 0;
int lastSensorLiftBottomState = 0, lastSensorLiftTopState = 0;


int relayArray[RELAY_ARRAY_SIZE];
int sensorArray[SENSOR_ARRAY_SIZE];

//const char *state = "";
//const char *relayStates = "";
const char *returnMessage = "";
const char *messages[MESSAGE_ARRAY_SIZE] = {"LIFT_UP"
                                            , "LIFT_DOWN"
                                            , "BIN_LOAD"
                                            , "BIN_DROP"
                                            , "VALVE_FEEDER_FWD_1"
                                            , "VALVE_FEEDER_REV_1"
                                            , "VALVE_FEEDER_FWD_2"
                                            , "VALVE_FEEDER_REV_2" // end relay block 1
                                            , "VALVE_SLIDER_OPEN"
                                            , "VALVE_SLIDER_CLOSE"
                                            , "VALVE_MIXER_UP"
                                            , "VALVE_MIXER_DOWN"
                                            , "HYRDAULIC_PUMP"
                                            , "VACUUM_PUMP"
                                            , "BELT_FORWARD"
                                            , "BELT_REVERSE" // end relay block 2
                                            , "RELAY_STATE"
                                            , "SENSOR_STATE"
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
      returnMessage = "ERROR";
    }
    else {
      returnMessage = "LOW";
      digitalWrite(relayValveLiftUp, LOW);
    }
  }
  else {
    returnMessage = "HIGH";
    digitalWrite(relayValveLiftUp, HIGH);
  }
}

void onLiftDownRelay() {
  if (!digitalRead(relayValveLiftDown) == LOW) {
    if (!isLiftDownFree())
    {
      returnMessage = "ERROR";
    }
    else
    {
      returnMessage = "LOW";
      digitalWrite(relayValveLiftDown, LOW);
    }
  }
  else {
    returnMessage = "HIGH";
    digitalWrite(relayValveLiftDown, HIGH);
  }
}

void onBinLoad() {
  if (!digitalRead(relayValveBinLoad) == LOW) {
    returnMessage = "LOW";
    digitalWrite(relayValveBinLoad, LOW);
  }
  else {
    returnMessage = "HIGH";
    digitalWrite(relayValveBinLoad, HIGH);
  }
}

void onBinDrop() {
  if (!digitalRead(relayValveBinDrop) == LOW) {
    returnMessage = "LOW";
    digitalWrite(relayValveBinDrop, LOW);
  }
  else {
    returnMessage = "HIGH";
    digitalWrite(relayValveBinDrop, HIGH);
  }
}

void onValveFeederFwd_1() {
  if (!digitalRead(relayValveFeederFwd_1) == LOW) {
    returnMessage = "LOW";
    digitalWrite(relayValveFeederFwd_1, LOW);
  }
  else {
    returnMessage = "HIGH";
    digitalWrite(relayValveFeederFwd_1, HIGH);
  }
}

void onValveFeederRev_1() {
  if (!digitalRead(relayValveFeederRev_1) == LOW) {
    returnMessage = "LOW";
    digitalWrite(relayValveFeederRev_1, LOW);
  }
  else {
    returnMessage = "HIGH";
    digitalWrite(relayValveFeederRev_1, HIGH);
  }
}

void onValveFeederFwd_2() {
  if (!digitalRead(relayValveFeederFwd_2) == LOW) {
    returnMessage = "LOW";
    digitalWrite(relayValveFeederFwd_2, LOW);
  }
  else {
    returnMessage = "HIGH";
    digitalWrite(relayValveFeederFwd_2, HIGH);
  }
}

void onValveFeederRev_2() {
  if (!digitalRead(relayValveFeederRev_2) == LOW) {
    returnMessage = "LOW";
    digitalWrite(relayValveFeederRev_2, LOW);
  }
  else {
    returnMessage = "HIGH";
    digitalWrite(relayValveFeederRev_2, HIGH);
  }
}

void onRequestRelayState() {
  returnMessage = "";
  char stateCharArray[RELAY_ARRAY_SIZE + 1] = "";

  for (int i = 0; i < RELAY_ARRAY_SIZE; i++) {
    if (digitalRead(relayArray[i]) == LOW) {
      stateCharArray[i] = '1';
      Serial.println("relay low");
    }
    else {
      stateCharArray[i] = '0';
      Serial.println("relay high");
    }
  }

  returnMessage = stateCharArray;
}

void onRequestSensorState() {
  returnMessage = "";
  char stateCharArray[SENSOR_ARRAY_SIZE + 1] = "";

  for (int i = 0; i < SENSOR_ARRAY_SIZE; i++) {
    if (digitalRead(sensorArray[i]) == LOW) {
      stateCharArray[i] = '1';
      Serial.println("sensor low");
    }
    else {
      stateCharArray[i] = '0';
      Serial.println("sensor high");
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
          //reserved for valve slider open
          break;
        case 9:
          //reserved for valve slider close
          break;
        case 10:
          //reserved for valve mixer up
          break;
        case 11:
          //reserved for valve mixer down
          break;
        case 12:
          //reserved for hydraulic pump
          break;
        case 13:
          //reserved for vacuum pump
          break;
        case 14:
          //reserved for belt fwd
          break;
        case 15:
          //reserved for belt rev
          break;
        case 16:
          onRequestRelayState();
          break;
        case 17:
          onRequestSensorState();
          break;
      }
    }
  }
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

void initializeSensorArray() {
  sensorArray[0] = sensorLiftBottom;
  pinMode(sensorArray[0], INPUT_PULLUP);
  sensorArray[1] = sensorLiftTop;
  pinMode(sensorArray[1], INPUT_PULLUP);

}

bool isMachineIdle() {
  // Check if ALL relay are HIGH
  if (!isRelayCompletelyOff) {
    returnMessage = "DETECTED LOW RELAY IN BLOCK";
    return false;
  }
  // Check if BIN is at BOTTOM
  if (!isBinAtLoad()) {
    returnMessage = "BIN NOT DETECTED AT BOTTOM";
    return false;
  }

  return true;
}

bool isRelayCompletelyOff() {
  onRequestRelayState();
  for (int i = 0; i < RELAY_ARRAY_SIZE; i++) {
    //char *c = ;
    if (returnMessage[i] == '1') {
      Serial.println("found relay with state LOW");
      return false;
    }
  }
  return true;
}
void setup() {
  // Register network adapter
  Serial.begin(57600);
  Serial.println(F("\n[bsf-arduino]"));
  // Change 'SS' to your Slave Select pin if you aren't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println("Failed init adapter");

  ether.staticSetup(myip, gwip, NULL, mask);
  ether.udpServerListenOnPort(&onListenUdpMessage, port);

  // Register IO
  initializeRelayArray();
  initializeSensorArray();
}

void loop() {

  if (digitalRead(relayValveLiftUp) == LOW) {
    if (digitalRead(sensorLiftBottom) == HIGH &&
        digitalRead(sensorLiftTop) == LOW) {
      digitalWrite(relayValveLiftUp, HIGH);
    }
  }

  // This must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
}
