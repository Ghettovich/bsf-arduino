#include "src/IODevice.h"
#include <EtherSia.h>
#include <ArduinoJson.h>

/** ENC28J60 Ethernet Interface */
const int etherSS = 53, port = 6677;
EtherSia_ENC28J60 ether(etherSS);
/** Define HTTP server */
HTTPServer http(ether);

/** Define UDP socket with port to listen on */
UDPSocket udp(ether, port);
const char * serverIP = "fd54:d174:8676:1:e076:66fd:7be6:2d36";

/**
   Initialize ethernet adapter
 * */
void setupECN28J60Adapter() {
  pinMode(etherSS, OUTPUT);
  digitalWrite(etherSS, HIGH);

  MACAddress macAddress("70:69:74:2d:30:31");
  macAddress.println();

  // Start Ethernet
  if (ether.begin(macAddress) == false) {
    Serial.println("Failed to configure Ethernet");
  }

  if (udp.setRemoteAddress(serverIP, port)) {
    Serial.print("Remote address: ");
    udp.remoteAddress().println();
  }

  Serial.print("Our link-local address is: ");
  ether.linkLocalAddress().println();
  Serial.print("Our global address is: ");
  ether.globalAddress().println();
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
  // 1-99 relay range
  else if (http.isGet(F("/relay/?")) || http.isGet(F("/relay/??"))) {
    int num = pathToNum();
    toggleRelay(num);

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
  Serial.println("printing payload on udp broadcast");
  Serial.println(payload);

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
  Serial.println("printing payload on tcp reply");
  Serial.println(payload);

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
   Get the output number from the path of the HTTP request

   @return the output number between 1-99, or -1 if it isn't valid
*/
int pathToNum() {
  int id;
  char relay_id[3];
  int count = 0;

  String path = http.path();
  Serial.println(path);

  for (int i = 7; i < path.length(); i++) {
    char c = path.charAt(i);
    if (isDigit(c)) {
      relay_id[count] = c;
      count++;
    }
  }

  relay_id[count] = '\0';

  if (count > 0) {
    sscanf(relay_id, "%d",  &id);
  }

  // relay block size is currently 8 and start at id 30 check and test method again when second relay block added
  if (id >= getMinRelayId() && id <= getMaxRelayId()) {
    return id;
  } else {
    http.notFound();
    return -1;
  }
}
