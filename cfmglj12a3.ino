#include "src/IODevice.h"
#include <ArduinoJson.h>

// ISR flags
volatile bool flagBinAtDrop = false;
volatile bool flagBinAtLoad = false;

// id's correspond to table in database
const int detectionSensorTypeId = 2;
const int sensorDropId = 10, sensorLoadId = 11;
const int sensorBinLoadPinNr = 21, sensorBinDropPinNr = 2;

IODevice detectionSensorBinDrop;
IODevice detectionSensorBinLoad;

void setupDetectionSensors() {
  /** !! INPUT proximity switches IF LOW detection = TRUE */
  // set pin mode
  pinMode(sensorBinDropPinNr, INPUT_PULLUP);
  pinMode(sensorBinLoadPinNr, INPUT_PULLUP);

  detectionSensorBinDrop.id = sensorDropId;
  detectionSensorBinDrop.pinNr = sensorBinDropPinNr;
  detectionSensorBinDrop.ioDeviceType = IODeviceType::DETECTION_SENSOR;

  detectionSensorBinLoad.id = sensorLoadId;
  detectionSensorBinLoad.pinNr = sensorBinLoadPinNr;
  detectionSensorBinLoad.ioDeviceType = IODeviceType::DETECTION_SENSOR;

  // ATTACH INTERRUPT EVENTS
  attachInterrupt(digitalPinToInterrupt(sensorBinLoadPinNr), onChangeSensorLiftBottom, CHANGE);
  attachInterrupt(digitalPinToInterrupt(sensorBinDropPinNr), onChangeSensorLiftTop, CHANGE);
}

void addBinDropToJsonArray(JsonArray items) {
  JsonObject obj = items.createNestedObject();
  obj["id"] = detectionSensorBinDrop.id;
  obj["typeId"] = detectionSensorTypeId;

  if (digitalRead(detectionSensorBinDrop.pinNr) == LOW) {
    obj["low"] = 1;
  }
  else if (digitalRead(detectionSensorBinDrop.pinNr) == HIGH) {
    obj["low"] = 0;
  }
}

void addBinLoadToJsonArray(JsonArray items) {
  JsonObject obj = items.createNestedObject();
  obj["id"] = detectionSensorBinLoad.id;
  obj["typeId"] = detectionSensorTypeId;

  if (digitalRead(detectionSensorBinLoad.pinNr) == LOW) {
    obj["low"] = 1;
  }
  else if (digitalRead(detectionSensorBinLoad.pinNr) == HIGH) {
    obj["low"] = 0;
  }
}

/** SENSOR STATE CHANGE (ATTACHED INTERRUPTS) */
// ISR CALL, called when sensor BOTTOM flipped
void onChangeSensorLiftBottom() {
  flagBinAtLoad = true;
}

// ISR CALL, called when sensor TOP flipped
void onChangeSensorLiftTop() {
  flagBinAtDrop = true;
}

/** BOOL OPERATIONS LIFT DETECTION */
// Lift at TOP and BIN at DROP
bool isBinAtDrop() {
  if (digitalRead(detectionSensorBinLoad.pinNr) == HIGH &&
      digitalRead(detectionSensorBinDrop.pinNr) == LOW)
    return true;
  else
    return false;
}

// Lift at LOAD position ready for BELT
bool isBinAtLoad() {
  if (digitalRead(detectionSensorBinDrop.pinNr) == HIGH &&
      digitalRead(detectionSensorBinLoad.pinNr) == LOW)
    return true;
  else
    return false;
}

// Lift  between sensors operator action required
bool isBinDetected() {
  if (digitalRead(detectionSensorBinLoad.pinNr) == HIGH &&
      digitalRead(detectionSensorBinDrop.pinNr) == HIGH) {
    return false;
  }
  if (isBinAtDrop()) {
    return true;
  }
  if (isBinAtLoad()) {
    return true;
  } else {
    Serial.println("unable to determine bin position");
    return false;
  }
}

void sensorLoop() {
  if (flagBinAtDrop) {
    Serial.println("sensor top flipped");
    Serial.print("Value = ");
    Serial.println(digitalRead(sensorBinDropPinNr));

    if (isLiftAsc()) {
      flipLiftUpRelay();
    }
    sendFullStatePayloadUdpPacket();

    flagBinAtDrop = false;
  }
  if (flagBinAtLoad) {
    Serial.println("sensor btm flipped");
    Serial.print("Value = ");
    Serial.println(digitalRead(sensorBinLoadPinNr));
    if (isLiftDesc()) {
      flipLiftDownRelay();
    }
    sendFullStatePayloadUdpPacket();

    flagBinAtLoad = false;
  }
}
