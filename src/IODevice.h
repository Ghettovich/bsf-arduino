#ifndef BSF_ARDUINO_IODEVICE_H
#define BSF_ARDUINO_IODEVICE_H

enum IODeviceType {
    UNKOWN = 0, WEIGHT_SENSOR, DETECTION_SENSOR, RELAY
};

struct IODevice {
    int id;
    int typeId;
    int pinNr;
    IODeviceType ioDeviceType;
};
#endif //BSF_ARDUINO_IODEVICE_H
