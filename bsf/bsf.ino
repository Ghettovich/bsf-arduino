#include <EtherCard.h>
#include <IPAddress.h>

#define ETHERCARD_UDPSERVER     1
#define RELAY_ARRAY_SIZE        8
#define RELAY_STATE_MSG_SIZE   10
#define SENSOR_ARRAY_SIZE       2
#define SENSOR_STATE_MSG_SIZE   4
#define MESSAGE_ARRAY_SIZE     18

/* Network configuration */
// ETHERNET interface IP address
static uint8_t myip[] = { 192, 168, 178, 23 };
// GATEWAY IP address
static uint8_t gwip[] = { 192, 168, 178, 1 };
// NETMASK
static uint8_t mask[] = { 255, 255, 255, 0 };
// Ethernet MAC address - must be unique on your network
static uint8_t mymac[] = { 0x70, 0x69, 0x74, 0x2D, 0x30, 0x31 };
// SERVER IP
static uint8_t serverIP[] = { 192, 168, 178, 174 };
// TCP/IP send and receive buffer
byte Ethernet::buffer[2000];

// PORT
const int port = 12310;
const int destPort = 12300;

// RELAY pin definitions
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

// ARRAY definitions
int relayArray[RELAY_ARRAY_SIZE];
int sensorArray[SENSOR_ARRAY_SIZE];

// STATES
int liftDownSensorState = 0, prevLiftDownSensorState = 0;
int liftUpSensorState = 0, prevLiftUpSensorState = 0;

// MESSAGES
//char * reply;

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

// IO types
enum IODeviceType {
  WEIGHTSENSOR = 1
  , DETECTIONSENSOR = 2
  , RELAY = 3
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
// Lift at LOAD position ready for BELT
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

char * buildRelayStateMsg() {
  char * buf = (char *) malloc(RELAY_STATE_MSG_SIZE);
  // +1 for NULL termination
  char stateCharArray[RELAY_STATE_MSG_SIZE + 1] = "";
  String deviceType = String(IODeviceType::RELAY);
  stateCharArray[0] = deviceType.charAt(0);
  stateCharArray[1] = ',';
  int posInArray;

  // str len is usged because first position is needed for number, second for comma (,)
  for (int i = strlen(stateCharArray); i < RELAY_STATE_MSG_SIZE; i++) {
    if (digitalRead(relayArray[posInArray]) == LOW) {
      stateCharArray[i] = '1';
      Serial.println("relay low");
    }
    else {
      stateCharArray[i] = '0';
      Serial.println("relay high");
    }
    posInArray++;
  }

  strcpy(buf, stateCharArray);
  return buf;
}

char * buildDetectionSensorStateMsg() {
  char * buf = (char *) malloc(SENSOR_STATE_MSG_SIZE);
  // +1 for NULL termination
  char sensorStateCharArray[SENSOR_STATE_MSG_SIZE + 1]= "";
  String deviceType = String(IODeviceType::DETECTIONSENSOR);
  sensorStateCharArray[0] = deviceType.charAt(0);
  sensorStateCharArray[1] = ',';
  int posInArray;

  // str len is usged because first position is needed for number, second for comma (,)

  // HERE STATE MSG SIZE IS USED
  for (int i = strlen(sensorStateCharArray); i < SENSOR_STATE_MSG_SIZE; i++) {
    if (digitalRead(sensorArray[posInArray]) == LOW) {
      sensorStateCharArray[i] = '1';
      Serial.println("detection sensor low");
    }
    else {
      sensorStateCharArray[i] = '0';
      Serial.println("detection sensor high");
    }
    posInArray++;
  }

  strcpy(buf, sensorStateCharArray);
  return buf;
}


void onListenUdpMessage(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len) {
  IPAddress src(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);
  Serial.println(data);
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
          returnMessage = buildRelayStateMsg();
          //onRequestRelayState();
          break;
        case 17:
          returnMessage = buildDetectionSensorStateMsg();
          //onRequestDetectionSensorState();
          break;
      }
    }
  }
  Serial.println(returnMessage);
  // Send reply
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

void setup() {
  // Register network adapter
  Serial.begin(57600);
  Serial.println(F("\n[bsf-arduino]"));
  // Change 'SS' to your Slave Select pin if you aren't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println("Failed init adapter");
  // SETUP STATIC IP
  ether.staticSetup(myip, gwip, mask);
  // BIND EVENTS
  ether.udpServerListenOnPort(&onListenUdpMessage, port);

  // SET SERVER CONFIG
  ether.copyIp(ether.hisip, serverIP);
  ether.printIp("srv", ether.hisip);

  // Register IO
  initializeRelayArray();
  initializeSensorArray();
}

void loop() {
  // This must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());

  //SENSORS
  liftDownSensorState = digitalRead(sensorLiftBottom);
  if (liftDownSensorState != prevLiftDownSensorState) {
    char * reply = buildDetectionSensorStateMsg();
    ether.sendUdp(reply, strlen(reply), destPort, ether.hisip, destPort);
    Serial.println(reply);
    Serial.println("lift down sensor flipped");
  }

  liftUpSensorState = digitalRead(sensorLiftTop);
  if (liftUpSensorState != prevLiftUpSensorState) {
    char * reply = buildDetectionSensorStateMsg();
    ether.sendUdp(reply, strlen(reply), destPort, ether.hisip, destPort);
    Serial.println(reply);
    Serial.println("lift down sensor flipped");
  }

  // RELAYS
  if (digitalRead(relayValveLiftUp) == LOW && isBinAtDrop()) {
    digitalWrite(relayValveLiftUp, HIGH);
    char * reply = buildRelayStateMsg();
    Serial.println("change detected");
    Serial.println(reply);
    //createRelayStateArray(stateCharArray);
    ether.sendUdp(reply, strlen(reply), destPort, ether.hisip, destPort);
    //free(reply);
  }
  if (digitalRead(relayValveLiftDown) == LOW && isBinAtLoad()) {
    digitalWrite(relayValveLiftDown, HIGH);
    //char stateCharArray[RELAY_ARRAY_SIZE + 1] = "";
    char * reply = buildRelayStateMsg();
    Serial.println("change detected");
    Serial.println(reply);
    //createRelayStateArray(stateCharArray);
    ether.sendUdp(reply, strlen(reply), destPort, ether.hisip, destPort );
    //free(reply);
  }


  // SET last States
  prevLiftDownSensorState = liftDownSensorState;
  prevLiftUpSensorState = liftUpSensorState;
}
