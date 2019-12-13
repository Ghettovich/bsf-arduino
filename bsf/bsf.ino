
#include <EtherSia.h>

#define RELAY_STATE_MSG_SIZE     10
#define DSENSOR_STATE_MSG_SIZE    4
#define IO_DEVICE_COUNT          10
#define STATE_MSG_SIZE           70


/** W5100 Ethernet Interface (optional parameter pin nr. default ok. */
EtherSia_ENC28J60 ether;

/** Define HTTP server */
HTTPServer http(ether);

/** Define UDP socket */
UDPSocket udp(ether);
const char * serverIP = "fd54:d174:8676:1:653f:56d7:bd7d:c238";

/** TIMERS */
unsigned long startTimeLiftUp, startTimeOperatorNotified;
long maxTimeLiftNotDetected = 3000; // 3 seconds

/** STATE TRANSITIONS */
int currentState = 0, prevState = 0;

/** STRUCTS */
struct IODevice {
  int id;
  int actionId;
  int typeId;
  int pinNr;
};

/** ARRAYS */
IODevice devices[10]; // 10 devices for now 8x relay 2x detection sensor !! MISSING WEIGHT SENSORS
//int relayArray[RELAY_ARRAY_SIZE];
//int sensorArray[SENSOR_ARRAY_SIZE];

/** RELAY PIN NUMBERS */
// RELAY pin definitions
int relayValveLiftUp = 22, relayValveLiftDown = 24;
int relayValveBinLoad = 26, relayValveBinDrop = 28;
int relayValveFeederFwd_1 = 30, relayValveFeederRev_1 = 32;
int relayValveFeederFwd_2 = 34, relayValveFeederRev_2 = 36;

/** !! INPUT proximity switches IF LOW detection = TRUE */
int sensorLiftBottom = 20, sensorLiftTop = 21;

/** ENUMS */
enum StateCode { READY = 0
, LIFT_ASC = 1
, LIFT_DESC = 2
, BIN_LOADING = 3
, BIN_DUMPING = 4
, LIFT_STUCK = 90
, BIN_STUCK = 91};
enum IODeviceType {WEIGHTSENSOR = 1
, DETECTIONSENSOR = 2
, RELAY = 3};

// END GLOBAL VAR

/** BOOL OPERATIONS LIFT DETECTION */
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
  if (digitalRead(sensorLiftTop) == HIGH &&
      digitalRead(sensorLiftBottom) == LOW)
    return true;
  else
    return false;
}
// Lift  between sensors operator action required
bool isBinDetected() {
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == HIGH) {
    return false;
  }
  if (isBinAtDrop()) {
    return true;
  }
  if (isBinAtLoad()) {
    return true;
  }
  else {
    Serial.println("unable to determine bin position");
    return false;
  }
}
// END BOOL OPERATIONS LIFT

/** INITIALIZE STRUCTS */
// Initialize IODevice struct stub
static void initializeIODeviceStructStub() {
  // ID, ACTION_ID, TYPE_ID, PIN NR
  // values mapped on table io_device
  devices[0] = {5, 20, 2, sensorLiftBottom};
  pinMode(sensorLiftBottom, INPUT_PULLUP);
  devices[1] = {6, 21, 2, sensorLiftTop};
  pinMode(sensorLiftTop, INPUT_PULLUP);
  devices[2] = {7, 1, 3, relayValveLiftUp};
  devices[3] = {8, 2, 3, relayValveLiftDown};
  devices[4] = {9, 3, 3, relayValveBinLoad};
  devices[5] = {10, 4, 3, relayValveBinDrop};
  devices[6] = {11, 5, 3, relayValveFeederFwd_1};
  devices[7] = {12, 6, 3, relayValveFeederRev_1};
  devices[8] = {13, 7, 3, relayValveFeederFwd_2};
  devices[9] = {14, 8, 3, relayValveFeederRev_2};

  // ATTACH INTERRUPT EVENTS
  attachInterrupt(digitalPinToInterrupt(sensorLiftBottom), onChangeSensorLiftBottom, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sensorLiftTop), onChangeSensorLiftTop, CHANGE);

  // DEFINE AS OUTPUT AND SET ALL RELAYS HIGH
  for (auto device : devices) {
    if (device.typeId == IODeviceType::RELAY) {
      pinMode(device.pinNr, OUTPUT);
      if (!digitalRead(device.pinNr) == HIGH) {
        digitalWrite(device.pinNr, HIGH);
      }
    }
  }
}
// END STRUCT

/** CREATE STATE CHAR ARRAY */
static void createIOTypeStateArray(int typeId, char buf[]) {  
  String stateMessage = "";

  for (int i = 0; i < IO_DEVICE_COUNT; i++) {
   
   stateMessage.concat(devices[i].id);
   stateMessage.concat(',');
   stateMessage.concat(devices[i].actionId);
   stateMessage.concat(',');
   stateMessage.concat(devices[i].typeId);
   stateMessage.concat(',');
    
   if(digitalRead(devices[i].pinNr) == LOW) {
     stateMessage.concat('1');
     Serial.println("got low val");
   }
   else if(digitalRead(devices[i].pinNr) == HIGH) {
     stateMessage.concat('0');
   }
   // Mark EOL
   stateMessage.concat(" ");
  }
  
  int len = stateMessage.length();
  stateMessage.toCharArray(buf, len + 1); // + 1 in case we reached max for null termination
}

/** SEND NEW STATE WITH UDP */
// SENSOR DETECTION
static void sendDetectionSensorStatePacket() {
  char stateArray[STATE_MSG_SIZE]; // TODO: should replace with global var since it's available anyway.
  createIOTypeStateArray(IODeviceType::DETECTIONSENSOR, stateArray);
  
  http.printHeaders(http.typePlain);  
  http.println(stateArray);
  http.sendReply();
}
// RELAY
static void sendRelayStatePacket() {
  char stateArray[STATE_MSG_SIZE]; // TODO: should replace with global var since it's available anyway.
  createIOTypeStateArray(IODeviceType::RELAY, stateArray);
  
  http.printHeaders(http.typePlain);  
  http.println(stateArray);
  http.sendReply();
}
// FULL STATE
static void sendFullStatePayloadPacket() {  
  char statePayload[STATE_MSG_SIZE]; // TODO: VALIDATE MSG LENGTH!!   
  // 0 for full payload
  createIOTypeStateArray(IODeviceType::RELAY, statePayload);
  
  http.printHeaders(http.typePlain);
  http.println(statePayload);
  http.sendReply();
}

// END SEND NEW STATE WITH UDP

/** SENSOR STATE CHANGE (ATTACHED INTERRUPTS) */
// ISR CALL, called when sensor BOTTOM flipped
void onChangeSensorLiftBottom() {
  Serial.println("sensor btm flipped");
}
// ISR CALL, called when sensor TOP flipped
void onChangeSensorLiftTop() {
  Serial.println("sensor top flipped");
}

/** ARDUINO SETUP AND LOOP METHOD */
void setup() {
  MACAddress macAddress("70:69:74:2d:30:31");

  // Setup serial port
  Serial.begin(115200);
  Serial.println("[EtherSia MiniHTTPServer]");
  macAddress.println();

  // Start Ethernet
  if (ether.begin(macAddress) == false) {
    Serial.println("Failed to configure Ethernet");
  }

  if (udp.setRemoteAddress(serverIP, 6677)) {
    Serial.print("Remote address: ");
    udp.remoteAddress().println();
  }

  Serial.print("Our link-local address is: ");
  ether.linkLocalAddress().println();
  Serial.print("Our global address is: ");
  ether.globalAddress().println();

  //Start timer imediately
  startTimeLiftUp = millis();

  //init arrays
  //initializeRelayArray();

  initializeIODeviceStructStub();
  Serial.println("Ready.");
}

void loop() {
  ether.receivePacket();

  if (http.isGet(F("/"))) {
    sendFullStatePayloadPacket();
//    http.printHeaders(http.typeHtml);
//    http.println(F("<h1>Reserve for state request?</h1>"));
//    http.sendReply();
  }
  else if (http.isGet(F("/relay-state"))) {
    sendRelayStatePacket();
  }
  else if (http.isGet(F("/sensor-state"))) {
    sendDetectionSensorStatePacket();
  }
  else if (http.havePacket()) {
    // Some other HTTP request, return 404
    http.notFound();
  }
  else {
    // Some other packet, reply with rejection
    ether.rejectPacket();
  }

  static unsigned long nextMessage = millis();
  if ((long)millis() - startTimeLiftUp > nextMessage) {
    Serial.println("Lift timer passed.\nReset timer and send udp packet");
    nextMessage = millis() + 10000;

    Serial.println("Sending UDP.");
    char stateArray[STATE_MSG_SIZE];
    createIOTypeStateArray(0, stateArray);
    Serial.println(stateArray);
    udp.println(stateArray);
    udp.send();

  }
}
