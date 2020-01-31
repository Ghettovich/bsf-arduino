
#include <EtherSia.h>
#include <ArduinoJson.h>

#define RELAY_STATE_MSG_SIZE     10
#define DSENSOR_STATE_MSG_SIZE    4
#define IO_DEVICE_COUNT          10
#define STATE_MSG_SIZE         600

int etherSS = 53;
const int arduinoId = 1;
/** W5100 Ethernet Interface (optional parameter pin nr. default ok. */
EtherSia_ENC28J60 ether(etherSS);

/** Define HTTP server */
HTTPServer http(ether);

/** Define UDP socket with port to listen on */
UDPSocket udp(ether, 6677);
const char * serverIP = "fd54:d174:8676:1:c4cf:5953:ac2c:8924";

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

/** RELAY PIN NUMBERS */
// RELAY pin definitions
int relayValveLiftUp = 22, relayValveLiftDown = 24;
int relayValveBinLoad = 26, relayValveBinDrop = 28;
int relayValveFeederFwd_1 = 30, relayValveFeederRev_1 = 32;
int relayValveFeederFwd_2 = 34, relayValveFeederRev_2 = 36;

/** !! INPUT proximity switches IF LOW detection = TRUE */
int sensorLiftBottom = 20, sensorLiftTop = 21;

/** ENUMS */
// refine with quadratic probing and add idle statecode
enum StateCode { READY = 0
, LIFT_ASC = 1
, LIFT_DESC = 2
, BIN_LOADING = 3
, BIN_DUMPING = 4
, LIFT_STUCK = 90
, BIN_STUCK = 91
               };
enum IODeviceType {WEIGHTSENSOR = 1
, DETECTIONSENSOR = 2
, RELAY = 3
                  };

// END GLOBAL VAR

/** BOOL OPERATIONS LIFT DETECTION */
// Lift at TOP and BIN at DROP
static bool isBinAtDrop() {
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == LOW)
    return true;
  else
    return false;
}
// Lift at LOAD position ready for BELT
static bool isBinAtLoad() {
  if (digitalRead(sensorLiftTop) == HIGH &&
      digitalRead(sensorLiftBottom) == LOW)
    return true;
  else
    return false;
}
// Lift  between sensors operator action required
static bool isBinDetected() {
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
static bool isLiftAsc() {
  if (digitalRead(relayValveLiftUp) == LOW)
    return true;
  else
    return false;
}
static bool isLiftDesc() {
  if (digitalRead(relayValveLiftDown) == LOW)
    return true;
  else
    return false;
}
// END BOOL OPERATIONS LIFT

/** SET STATE */
static void setState(StateCode newState) {
  prevState = currentState;
  currentState = newState;
}
// END SET STATE

/** INITIALIZE STRUCTS */
// Initialize IODevice struct stub
static void initializeIODeviceStructStub() {
  // ID, ACTION_ID, TYPE_ID, PIN NR
  // values mapped on table io_device
  devices[1] = {5, 21, 2, sensorLiftTop};
  pinMode(sensorLiftTop, INPUT_PULLUP);
  devices[0] = {6, 20, 2, sensorLiftBottom};
  pinMode(sensorLiftBottom, INPUT_PULLUP);
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

/** SOCKET SEND/REPLY */
// BROADCAST PAYLOAD
static void sendFullStatePayloadUdpPacket() {
  char payload[ETHERSIA_MAX_PACKET_SIZE];
  StaticJsonDocument<1000> doc;
  JsonObject info = doc.to<JsonObject>();
  JsonObject ioDevices = doc.createNestedObject("iodevices");
  JsonArray items = ioDevices.createNestedArray("items");

  createFullStateJsonPayload(doc, info, ioDevices, items);
  serializeJson(doc, payload);
  Serial.println("printing payload on udp broadcast");
  Serial.println(payload);

  udp.print(payload);
  udp.send();
}
// TCP HTTP REPLY
static void sendFullStatePayloadPacket() {
  char payload[ETHERSIA_MAX_PACKET_SIZE];
  StaticJsonDocument<1000> doc;  
  JsonObject info = doc.to<JsonObject>();
  JsonObject ioDevices = doc.createNestedObject("iodevices");
  JsonArray items = ioDevices.createNestedArray("items");

  createFullStateJsonPayload(doc, info, ioDevices, items);
  serializeJson(doc, payload);
  Serial.println("printing payload on tcp reply");
  Serial.println(payload);

  http.printHeaders(http.typeJson);
  http.println(payload);
  http.sendReply();
}

static void createFullStateJsonPayload(DynamicJsonDocument doc, JsonObject info, JsonObject ioDevices, JsonArray items) {
  determineCurrentState();  
    
  info["arduinoId"] = arduinoId;
  info["state"] = currentState;
  info["stateReply"] = 0;

  for (int i = 0; i < IO_DEVICE_COUNT; i++) {
    JsonObject obj = items.createNestedObject();
    obj["id"] = devices[i].id;
    obj["actionId"] = devices[i].actionId;
    obj["typeId"] = devices[i].typeId;
    if (digitalRead(devices[i].pinNr) == LOW) {
      obj["low"] = 1;
    }
    else if (digitalRead(devices[i].pinNr) == HIGH) {
      obj["low"] = 0;
    }
  }
}

// END SEND NEW STATE WITH UDP

/** SENSOR STATE CHANGE (ATTACHED INTERRUPTS) */
// ISR CALL, called when sensor BOTTOM flipped
void onChangeSensorLiftBottom() {
  Serial.println("sensor btm flipped");
  if (isLiftDesc()) {
    digitalWrite(relayValveLiftDown, HIGH);
  }
  sendFullStatePayloadUdpPacket();
}
// ISR CALL, called when sensor TOP flipped
void onChangeSensorLiftTop() {
  Serial.println("sensor top flipped");
  if (isLiftAsc()) {
    digitalWrite(relayValveLiftUp, HIGH);
  }
  sendFullStatePayloadUdpPacket();
}

// END SENSOR STATE CHANGE

/** TURN ON/OFF RELAYS */
// received instruction to send lift UP to position BIN DROP
void onLiftUpRelay() {
  if (!digitalRead(relayValveLiftUp) == LOW) {
    if (isBinAtLoad()) {
      setState(StateCode::LIFT_ASC);
      digitalWrite(relayValveLiftUp, LOW);
    }
    else {
      Serial.println("bin is not at load");
    }
  }
  else {
    digitalWrite(relayValveLiftUp, HIGH);
  }
}
// received instruction to send lift DOWN to LOAD
void onLiftDownRelay() {
  if (!digitalRead(relayValveLiftDown) == LOW) {
    if (isBinAtDrop()) {
      setState(StateCode::LIFT_DESC);
      digitalWrite(relayValveLiftDown, LOW);
    }
    else {
      Serial.println("bin is not at drop");
    }
  }
  else {
    // set state to ready here??
    digitalWrite(relayValveLiftDown, HIGH);
  }
}
void onBinLoad() {
  // TODO VERIFY SENSOR CHECK
  if (!digitalRead(relayValveBinLoad) == LOW) {
    digitalWrite(relayValveBinLoad, LOW);
  }
  else {
    digitalWrite(relayValveBinLoad, HIGH);
  }
}
void onBinDrop() {
  // TODO VERIFY SENSOR CHECK
  if (!digitalRead(relayValveBinDrop) == LOW) {
    digitalWrite(relayValveBinDrop, LOW);
  }
  else {
    digitalWrite(relayValveBinDrop, HIGH);
  }
}
// FEEDERS 1 & 2
void onValveFeederFwd_1() {
  if (!digitalRead(relayValveFeederFwd_1) == LOW) {
    digitalWrite(relayValveFeederFwd_1, LOW);
  }
  else {
    digitalWrite(relayValveFeederFwd_1, HIGH);
  }
}
void onValveFeederRev_1() {
  if (!digitalRead(relayValveFeederRev_1) == LOW) {
    digitalWrite(relayValveFeederRev_1, LOW);
  }
  else {
    digitalWrite(relayValveFeederRev_1, HIGH);
  }
}
void onValveFeederFwd_2() {
  if (!digitalRead(relayValveFeederFwd_2) == LOW) {
    digitalWrite(relayValveFeederFwd_2, LOW);
  }
  else {
    digitalWrite(relayValveFeederFwd_2, HIGH);
  }
}
void onValveFeederRev_2() {
  if (!digitalRead(relayValveFeederRev_2) == LOW) {
    digitalWrite(relayValveFeederRev_2, LOW);
  }
  else {
    digitalWrite(relayValveFeederRev_2, HIGH);
  }
}

static void determineCurrentState() {
  if (isLiftAsc()) {
    setState(StateCode::LIFT_ASC);
  }
  else if (isLiftDesc()) {
    setState(StateCode::LIFT_DESC);
  }
  else {
    setState(StateCode::READY);
  }
  // if timer lift passed it is stuck
}

/** ARDUINO SETUP AND LOOP METHOD */
void setup() {
  //pinMode(etherSS, OUTPUT);
  //digitalWrite(etherSS, HIGH);

  // Setup serial port
  Serial.begin(115200);
  Serial.println("[BSF Lift and Feeders]");

  MACAddress macAddress("70:69:74:2d:30:31");
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

  //init arrays
  initializeIODeviceStructStub();

  setState(StateCode::READY);
  
  //Start timer imediately
  startTimeLiftUp = millis();
  Serial.println("Ready.");
}

void loop() {
  ether.receivePacket();

  if (http.isGet(F("/"))) {
    Serial.println("Sending full state on reply.");
    sendFullStatePayloadPacket();
  }
  else if (http.isGet(F("/relay/lift-up"))) {
    onLiftUpRelay();
    sendFullStatePayloadPacket();
  }
  else if (http.isGet(F("/relay/lift-down"))) {
    onLiftDownRelay();
    sendFullStatePayloadPacket();
  }
  else if (http.isGet(F("/relay/bin-load"))) {
    onBinLoad();
    sendFullStatePayloadPacket();
  }
  else if (http.isGet(F("/relay/bin-drop"))) {
    onBinDrop();
    sendFullStatePayloadPacket();
  }
  else if (http.isGet(F("/relay/valv-feeder-fwd-1"))) {
    onValveFeederFwd_1();
    sendFullStatePayloadPacket();
  }
  else if (http.isGet(F("/relay/valv-feeder-rev-1"))) {
    onValveFeederRev_1();
    sendFullStatePayloadPacket();
  }
  else if (http.isGet(F("/relay/valv-feeder-fwd-2"))) {
    onValveFeederFwd_2();
    sendFullStatePayloadPacket();
  }
  else if (http.isGet(F("/relay/valv-feeder-rev-2"))) {
    onValveFeederRev_2();
    sendFullStatePayloadPacket();
  }
  else if (http.havePacket()) {
    // Some other HTTP request, return 404
    Serial.println("unrecognized request");
    http.notFound();
  }
  else {
    // Some other packet, reply with rejection
    ether.rejectPacket();
  }

  static unsigned long nextMessage = millis();
  if ((long)millis() - startTimeLiftUp > nextMessage) {
    Serial.println("Lift timer passed.\nReset timer and send udp packet");
    nextMessage = millis() + 30000;
    sendFullStatePayloadUdpPacket();
  }

  prevState = currentState;
}
