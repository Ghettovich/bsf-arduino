#include "src/IODevice.h"
#include <ArduinoJson.h>

const int relayBlockSize = 8;
IODevice relayArray[relayBlockSize];

const int relayLiftUp = 30, relayLiftDown = 31,
        relayLiftBinLoad = 32, relayBinDrop = 33,
        relayFeederFWD_1 = 34, relayFeederREV_1 = 35,
        relayFeederFWD_2 = 36, relayFeederREV_2 = 37;

int getRelayBlockSize() {
    return relayBlockSize;
}

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
    const int relayTypeId = 3;

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

void flipRelay(int relayId) {
    for (byte i = 0; i < relayBlockSize; i++) {
        if(relayArray[i].id == relayId) {

            break;
        }
    }
}

void toggleRelay(int relayId) {

    switch (relayId) {
        case relayLiftUp :
            onLiftUpRelay();
            break;
        case relayLiftDown :
            onLiftDownRelay();
            break;
        case relayLiftBinLoad :
            onBinLoad();
            break;
        case relayBinDrop :
            onBinDrop();
            break;
        case relayFeederFWD_1 :
            onValveFeederFwd_1();
            break;
        case relayFeederREV_1 :
            onValveFeederRev_1();
            break;
        case relayFeederFWD_2 :
            onValveFeederFwd_2();
            break;
        case relayFeederREV_2 :
            onValveFeederRev_2();
            break;
        default:
            printMessage("unkown id for relay");
            break;
    }
}

/** TURN ON/OFF RELAYS */
// received instruction to send lift UP to position BIN DROP
void onLiftUpRelay() {
    if (!digitalRead(relayLiftUp) == LOW) {
        if (isBinAtLoad()) {
            setState(StateCode::LIFT_ASC);
            digitalWrite(relayLiftUp, LOW);
        }
    } else {
        digitalWrite(relayLiftUp, HIGH);
    }
}

// received instruction to send lift DOWN to LOAD
void onLiftDownRelay() {
    if (!digitalRead(relayLiftDown) == LOW) {
        if (isBinAtDrop()) {
            setState(StateCode::LIFT_DESC);
            digitalWrite(relayLiftDown, LOW);
        }
    } else {
        digitalWrite(relayLiftDown, HIGH);
    }
}

void onBinLoad() {
    // TODO VERIFY SENSOR CHECK
    if (!digitalRead(relayLiftBinLoad) == LOW) {
        digitalWrite(relayLiftBinLoad, LOW);
    } else {
        digitalWrite(relayLiftBinLoad, HIGH);
    }
}

void onBinDrop() {
    // TODO VERIFY SENSOR CHECK
    if (!digitalRead(relayBinDrop) == LOW) {
        digitalWrite(relayBinDrop, LOW);
    } else {
        digitalWrite(relayBinDrop, HIGH);
    }
}

// FEEDERS 1 & 2
void onValveFeederFwd_1() {
    if (!digitalRead(relayFeederFWD_1) == LOW) {
        digitalWrite(relayFeederFWD_1, LOW);
    } else {
        digitalWrite(relayFeederFWD_1, HIGH);
    }
}

void onValveFeederRev_1() {
    if (!digitalRead(relayFeederREV_1) == LOW) {
        digitalWrite(relayFeederREV_1, LOW);
    } else {
        digitalWrite(relayFeederREV_1, HIGH);
    }
}

void onValveFeederFwd_2() {
    if (!digitalRead(relayFeederFWD_2) == LOW) {
        digitalWrite(relayFeederFWD_2, LOW);
    } else {
        digitalWrite(relayFeederFWD_2, HIGH);
    }
}

void onValveFeederRev_2() {
    if (!digitalRead(relayFeederREV_2) == LOW) {
        digitalWrite(relayFeederREV_2, LOW);
    } else {
        digitalWrite(relayFeederREV_2, HIGH);
    }
}

void flipLiftUpRelay() {
    flipRelay(relayLiftUp);
}

void flipLiftDownRelay() {
    flipRelay(relayLiftDown);
}

bool isLiftAsc() {
    for (byte i = 0; i <relayBlockSize ; ++i) {
        if(relayArray[i].id == relayLiftUp) {
            if(digitalRead(relayArray[i].pinNr) == LOW) {
                return true;
            } else if(digitalRead(relayArray[i].pinNr) == HIGH) {
                return false;
            }
        }
    }
    return false;
}

bool isLiftDesc() {
    for (byte i = 0; i <relayBlockSize ; ++i) {
        if(relayArray[i].id == relayLiftDown) {
            if(digitalRead(relayArray[i].pinNr) == LOW) {
                return true;
            } else if(digitalRead(relayArray[i].pinNr) == HIGH) {
                return false;
            }
        }
    }
    return false;
}
