#include <ArduinoJson.h>
#include "src/IODevice.h"

const int arduinoId = 1;

/** TIMERS */
unsigned long startTimeLiftUp, startTimeOperatorNotified;
long maxTimeLiftNotDetected = 3000; // 3 seconds

/** STATE TRANSITIONS */
int currentState = 0, prevState = 0;

/** ENUMS */
// refine with quadratic probing and add idle statecode
enum StateCode {
    READY = 0, LIFT_ASC, LIFT_DESC, BIN_LOADING, BIN_DUMPING, LIFT_STUCK, BIN_STUCK
};
// END GLOBAL VAR

/** SET STATE */
static void setState(StateCode newState) {
    prevState = currentState;
    currentState = newState;
}
// END SET STATE

static void determineCurrentState() {
//    if (isLiftAsc()) {
//        setState(StateCode::LIFT_ASC);
//    } else if (isLiftDesc()) {
//        setState(StateCode::LIFT_DESC);
//    } else {
//        setState(StateCode::READY);
//    }
    setState(StateCode::READY);
    // if timer lift passed it is stuck
}

/** ARDUINO SETUP AND LOOP METHOD */
void setup() {
    Serial.begin(115200);
    Serial.println("[BSF Lift and Feeders]");

    setupRelayArray();
    setupDetectionSensors();
    setupECN28J60Adapter();

    setState(StateCode::READY);

    //  //Start timer imediately
    //  startTimeLiftUp = millis();
    Serial.println("Ready.");
}

void loop() {
    // Listen for incoming packets
    sensorLoop();
    receiveEthernetPacketLoop();

    prevState = currentState;
}
