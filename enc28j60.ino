#include "src/IODevice.h"
#include "src/ReplyEnum.h"
#include <ArduinoJson.h>

const int serverPort = 5001;
const int maxPayloadSize = 1500;
char jsonPayload[maxPayloadSize];

ReplyWithCode replyCode;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xE8, 0xD4, 0x43, 0x00, 0xA8, 0x3A
};

IPAddress ip(192, 168, 178, 21);
byte serverIP[] = {192, 168, 178, 242};

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(serverPort);
EthernetClient client;

/**
   Initialize ethernet adapter
 * */
void setupECN28J60Adapter() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("[BSF Arduino Bin & Lift]");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


/**
   Listen for incoming ethernet packets
*/
void receiveEthernetPacketLoop() {
  // listen for incoming clients
  client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {

        int index = 0;

        // Here is where the payload data is.
        while (client.available())
        {
          jsonPayload[index] = client.read();
          index++;
        }

        deserializeJsonPayload();
        sendReply();
        break;
      }

    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");

  }
}

void sendReply() {

  switch (replyCode) {
    case FULL_STATE_RPLY:
      sendFullStatePayloadPacket();
      break;
    default:
      sendFullStatePayloadPacket();
      break;
  }
}


/**
   Deserialize the JSON document
*/
void deserializeJsonPayload() {
  Serial.println("copying payload...");
  StaticJsonDocument <maxPayloadSize> doc;
  DeserializationError error = deserializeJson(doc, jsonPayload);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  } else {
    int replyCodeJson = doc["replyCode"];
    Serial.print("Reply state code = ");
    Serial.println(replyCodeJson);

    if (replyCodeJson) {
      replyCode = identifyReplyCode(replyCodeJson);
    }

    int toggleRelayId = doc["toggleRelayId"];
    Serial.print("Relay to toggle = ");
    Serial.println(toggleRelayId);

    if (toggleRelayId) {
      toggleRelay(toggleRelayId);
    }

  }

}

/** SOCKET SEND/REPLY */
// BROADCAST PAYLOAD
void sendNewStateToServer() {

  char payload[maxPayloadSize];
  StaticJsonDocument<maxPayloadSize> doc;
  JsonObject info = doc.to<JsonObject>();
  JsonObject ioDevices = doc.createNestedObject("iodevices");
  JsonArray items = ioDevices.createNestedArray("items");

  createFullStateJsonPayload(info, items);
  serializeJson(doc, payload);


  // if you get a connection, report back via serial:
  if (client.connect(serverIP, serverPort)) {
    Serial.print("connected to ");
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    client.println("REEEE");
    client.println();

    client.stop();

  } else {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }

}
// TCP HTTP REPLY
void sendFullStatePayloadPacket() {
  char payload[maxPayloadSize];
  StaticJsonDocument<maxPayloadSize> doc;
  JsonObject info = doc.to<JsonObject>();
  JsonObject ioDevices = doc.createNestedObject("iodevices");
  JsonArray items = ioDevices.createNestedArray("items");

  createFullStateJsonPayload(info, items);
  serializeJson(doc, payload);
  //  Serial.println("printing payload on tcp reply");
  //  Serial.println(payload);

  client.println(payload);
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
