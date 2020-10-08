#include "src/IODevice.h"
#include <ArduinoJson.h>

const int relayBlockSize = 8, relayTypeId = 3;
IODevice relayArray[relayBlockSize];

const int relayLiftUp = 30, relayLiftDown = 31,
          relayLiftBinLoad = 32, relayBinDrop = 33,
          relayFeederFWD_1 = 34, relayFeederREV_1 = 35,
          relayFeederFWD_2 = 36, relayFeederREV_2 = 37;


void createRelayStub() {
  int pinNrStart = 22;
  // Relay id's start at 30 in database, this is the first block
  const int idIndexStart = 30;

  for (int i = 0; i < relayBlockSize; ++i) {
    IODevice ioDevice = IODevice();
    // map i to id so that it matches ID in database
    int id = idIndexStart + i;
    ioDevice.id = id;
    // Set pin nr
    ioDevice.pinNr = pinNrStart;
    ioDevice.ioDeviceType = IODeviceType::RELAY;
    // Set pin mode
    pinMode(pinNrStart, OUTPUT);
    digitalWrite(pinNrStart, HIGH);
    // add to device array
    relayArray[i] = ioDevice;
    // Because of cable management pin numbers have an offset of 2
    pinNrStart += 2;
  }
}

void setupRelayArray() {
  createRelayStub();
}

void addRelayArrayToJsonArray(JsonArray items) {

  for (byte i = 0; i < relayBlockSize; i++) {
    JsonObject obj = items.createNestedObject();
    obj["id"] = relayArray[i].id;
    obj["typeId"] = relayTypeId;

    if (digitalRead(relayArray[i].pinNr) == LOW) {
      obj["low"] = 1;
    }
    else if (digitalRead(relayArray[i].pinNr) == HIGH) {
      obj["low"] = 0;
    }
  }
}

void toggleRelay(int relayId) {
  Serial.print("relay id = ");
  Serial.println(relayId);
  IODevice relayToToggle;

  for (byte i = 0; i < relayBlockSize ; ++i) {
    if (relayArray[i].id == relayId) {
      relayToToggle = relayArray[i];
    }
  }

  if (relayToToggle.id > 0) {

    switch (relayId) {
      case relayLiftUp :
        onLiftUpRelay(relayToToggle);
        break;
      case relayLiftDown :
        onLiftDownRelay(relayToToggle);
        break;
      case relayLiftBinLoad :
        onBinLoad(relayToToggle);
        break;
      case relayBinDrop :
        onBinDrop(relayToToggle);
        break;
      case relayFeederFWD_1 :
        onValveFeederFwd_1(relayToToggle);
        break;
      case relayFeederREV_1 :
        onValveFeederRev_1(relayToToggle);
        break;
      case relayFeederFWD_2 :
        onValveFeederFwd_2(relayToToggle);
        break;
      case relayFeederREV_2 :
        onValveFeederRev_2(relayToToggle);
        break;
      default:
        Serial.println("unkown id for relay");
        break;
    }

  } else {
    Serial.println("unable to find matching id for relay.");
  }
}


/** TURN ON/OFF RELAYS */
// received instruction to send lift UP to position BIN DROP
void onLiftUpRelay(IODevice relayToToggle) {
  if (!digitalRead(relayToToggle.pinNr) == LOW) {
    if (isBinAtLoad()) {
      setState(StateCode::LIFT_ASC);
      digitalWrite(relayToToggle.pinNr, LOW);
    }
  } else {
    digitalWrite(relayToToggle.pinNr, HIGH);
  }
}

// received instruction to send lift DOWN to LOAD
void onLiftDownRelay(IODevice relayToToggle) {
  if (!digitalRead(relayToToggle.pinNr) == LOW) {
    if (isBinAtDrop()) {
      setState(StateCode::LIFT_DESC);
      digitalWrite(relayToToggle.pinNr, LOW);
    }
  } else {
    digitalWrite(relayToToggle.pinNr, HIGH);
  }
}

void onBinLoad(IODevice relayToToggle) {
  // TODO VERIFY SENSOR CHECK
  if (!digitalRead(relayToToggle.pinNr) == LOW) {
    digitalWrite(relayToToggle.pinNr, LOW);
  } else {
    digitalWrite(relayToToggle.pinNr, HIGH);
  }
}

void onBinDrop(IODevice relayToToggle) {
  // TODO VERIFY SENSOR CHECK
  if (!digitalRead(relayToToggle.pinNr) == LOW) {
    digitalWrite(relayToToggle.pinNr, LOW);
  } else {
    digitalWrite(relayToToggle.pinNr, HIGH);
  }
}

// FEEDERS 1 & 2
void onValveFeederFwd_1(IODevice relayToToggle) {
  if (!digitalRead(relayToToggle.pinNr) == LOW) {
    digitalWrite(relayToToggle.pinNr, LOW);
  } else {
    digitalWrite(relayToToggle.pinNr, HIGH);
  }
}

void onValveFeederRev_1(IODevice relayToToggle) {
  if (!digitalRead(relayToToggle.pinNr) == LOW) {
    digitalWrite(relayToToggle.pinNr, LOW);
  } else {
    digitalWrite(relayToToggle.pinNr, HIGH);
  }
}

void onValveFeederFwd_2(IODevice relayToToggle) {
  if (!digitalRead(relayToToggle.pinNr) == LOW) {
    digitalWrite(relayToToggle.pinNr, LOW);
  } else {
    digitalWrite(relayToToggle.pinNr, HIGH);
  }
}

void onValveFeederRev_2(IODevice relayToToggle) {
  Serial.println("Hoi rev2");
  if (!digitalRead(relayToToggle.pinNr) == LOW) {
    digitalWrite(relayToToggle.pinNr, LOW);
  } else {
    digitalWrite(relayToToggle.pinNr, HIGH);
  }
}

void flipLiftUpRelay() {
  toggleRelay(relayLiftUp);
}

void flipLiftDownRelay() {
  toggleRelay(relayLiftDown);
}

bool isLiftAsc() {
  for (byte i = 0; i < relayBlockSize ; ++i) {
    if (relayArray[i].id == relayLiftUp) {
      if (digitalRead(relayArray[i].pinNr) == LOW) {
        return true;
      } else if (digitalRead(relayArray[i].pinNr) == HIGH) {
        return false;
      }
    }
  }
  return false;
}

bool isLiftDesc() {
  for (byte i = 0; i < relayBlockSize ; ++i) {
    if (relayArray[i].id == relayLiftDown) {
      if (digitalRead(relayArray[i].pinNr) == LOW) {
        return true;
      } else if (digitalRead(relayArray[i].pinNr) == HIGH) {
        return false;
      }
    }
  }
  return false;
}
