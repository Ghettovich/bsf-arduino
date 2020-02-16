#include "src/IODevice.h"
#include <EtherSia.h>
#include <ArduinoJson.h>

/** ENC28J60 Ethernet Interface */
const int etherSS = 53, remotePort = 6677;
EtherSia_ENC28J60 ether(etherSS);
/** Define HTTP server */
HTTPServer http(ether);

/** Define UDP socket with port to listen on */
UDPSocket udp(ether, remotePort);
const char * serverIP = "fe80::3d15:b791:18be:a308";

/**
 * Initialize ethernet adapter
 * */
void setupECN28J60Adapter() {
    MACAddress macAddress("70:69:74:2d:30:31");
    macAddress.println();

    // Start Ethernet
    if (ether.begin(macAddress) == false) {
        printMessage("Failed to configure Ethernet");
    }

    printEthernetAdapterInfo();

    udp.setRemoteAddress(serverIP, remotePort);
    printRemoteAddress();
}

void receiveEthernetPacket() {
    ether.receivePacket();

    if (http.isGet(F("/"))) {
        //    http.printHeaders(http.typeHtml);
        //    http.println(F("<h1>Hello World</h1>"));
        //    http.sendReply();
        Serial.println("Sending full state on reply.");
        sendFullStatePayloadPacket();
    }
    else if (http.isGet(F("/relay/?"))) {
        int8_t num = pathToNum();
        Serial.println(num);
    }
    else {
        // Some other packet, reply with rejection
        ether.rejectPacket();
    }
    //  static unsigned long nextMessage = millis();
    //  if ((long)millis() - startTimeLiftUp > nextMessage) {
    //    Serial.println("Lift timer passed.\nReset timer and send udp packet");
    //    nextMessage = millis() + 30000;
    //    sendFullStatePayloadUdpPacket();
    //  }
}

/** SOCKET SEND/REPLY */
// BROADCAST PAYLOAD
void sendFullStatePayloadUdpPacket() {
    char payload[ETHERSIA_MAX_PACKET_SIZE];
    StaticJsonDocument<ETHERSIA_MAX_PACKET_SIZE> doc;
    JsonObject info = doc.to<JsonObject>();
    JsonObject ioDevices = doc.createNestedObject("iodevices");
    JsonArray items = ioDevices.createNestedArray("items");

    createFullStateJsonPayload(doc, info, ioDevices, items);
    serializeJson(doc, payload);
    printMessage("printing payload on udp broadcast");
    printMessage(payload);

    udp.print(payload);
    udp.send();
}
// TCP HTTP REPLY
void sendFullStatePayloadPacket() {
    char payload[ETHERSIA_MAX_PACKET_SIZE];
    StaticJsonDocument<ETHERSIA_MAX_PACKET_SIZE> doc;
    JsonObject info = doc.to<JsonObject>();
    JsonObject ioDevices = doc.createNestedObject("iodevices");
    JsonArray items = ioDevices.createNestedArray("items");

    createFullStateJsonPayload(doc, info, ioDevices, items);
    serializeJson(doc, payload);
    printMessage("printing payload on tcp reply");
    printMessage(payload);

    http.printHeaders(http.typeJson);
    http.println(payload);
    http.sendReply();
}

void createFullStateJsonPayload(DynamicJsonDocument doc, JsonObject info, JsonObject ioDevices, JsonArray items) {
    //determineCurrentState();

    info["arduinoId"] = arduinoId;
    info["state"] = currentState;

    addRelayArrayToJsonArray(items);
    addBinDropToJsonArray(items);
    addBinLoadToJsonArray(items);
}

/**
 * Get the output number from the path of the HTTP request
 *
 * @return the output number, or -1 if it isn't valid
 */
int8_t pathToNum()
{
    // /outputs/X
    // 0123456789
    int8_t num = http.path()[9] - '1';
    // relay block size is currently 8, check and test method again when second relay block added
    if (0 <= num && num < getRelayBlockSize()) {
        return num;
    } else {
        http.notFound();
        return -1;
    }
}
