#include "src/IODevice.h"
#include <EtherSia.h>
#include <ArduinoJson.h>

/** ENC28J60 Ethernet Interface */
const int etherSS = 53, port = 6677;
EtherSia_ENC28J60 ether(etherSS);
/** Define HTTP server */
HTTPServer http(ether);

/** Define UDP socket with port to listen on */
UDPSocket udp(ether);
const char * serverIP = "2a02:a213:9f81:4e80:2aab:51a2:d551:1c33";

/**
   Initialize ethernet adapter
 * */
void setupECN28J60Adapter() {
  pinMode(etherSS, OUTPUT);
  digitalWrite(etherSS, HIGH);

  MACAddress macAddress("70:69:74:2d:30:31");
  macAddress.println();

  //ether.disableAutoconfiguration();

  // Start Ethernet
  if (ether.begin(macAddress) == false) {
    Serial.println("Failed to configure Ethernet");
  }

  // Configure a static global address and router addresses
//  ether.setGlobalAddress("2001:1234::5000");
//  if (ether.setRouter("fe80::4a8f:5aff:fe60:b7fe") == false) {
//    Serial.println("Failed to configure router address");
//  }

  if (udp.setRemoteAddress(serverIP, port)) {
    Serial.print("Remote address: ");
    udp.remoteAddress().println();
  }

  Serial.print("Our link-local address is: ");
  ether.linkLocalAddress().println();
  Serial.print("Our global address is: ");
  ether.globalAddress().println();
}

/** SOCKET SEND/REPLY */
// BROADCAST PAYLOAD
void sendFullStatePayloadUdpPacket() {
  char payload[ETHERSIA_MAX_PACKET_SIZE];
  StaticJsonDocument<ETHERSIA_MAX_PACKET_SIZE> doc;
  JsonObject info = doc.to<JsonObject>();
  JsonObject ioDevices = doc.createNestedObject("iodevices");
  JsonArray items = ioDevices.createNestedArray("items");

  createFullStateJsonPayload(info, items);
  serializeJson(doc, payload);
  Serial.println("printing payload on udp broadcast");
  Serial.println(payload);

  // ToDo add check if remoteAddress is found (see weightstation sketch)

  udp.println(payload);
  udp.send();
}
// TCP HTTP REPLY
void sendFullStatePayloadPacket() {
  char payload[ETHERSIA_MAX_PACKET_SIZE];
  StaticJsonDocument<ETHERSIA_MAX_PACKET_SIZE> doc;
  JsonObject info = doc.to<JsonObject>();
  JsonObject ioDevices = doc.createNestedObject("iodevices");
  JsonArray items = ioDevices.createNestedArray("items");

  createFullStateJsonPayload(info, items);
  serializeJson(doc, payload);
  Serial.println("printing payload on tcp reply");
  Serial.println(payload);

  http.printHeaders(http.typeJson);
  http.println(payload);
  http.sendReply();
}

// ToDO: REMOVE IO DEVICES PARAMATER, SAME FOR DOC (UNUSED!)
void createFullStateJsonPayload(JsonObject info, JsonArray items) {
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
  sscanf(relay_id, "%d",  &id);

  // relay block size is currently 8 and start at id 30 check and test method again when second relay block added
  if (id >= getMinRelayId() && id <= getMaxRelayId()) {
    return id;
  } else {
    http.notFound();
    return -1;
  }
}

/**
   Listen for incoming ethernet packets
*/
void receiveEthernetPacketLoop() {
  ether.receivePacket();

  if (http.isGet(F("/"))) {
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
