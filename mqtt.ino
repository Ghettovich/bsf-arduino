bool flagPublishProximity = false;
byte macMqtt[] = {0xE8, 0xD4, 0x43, 0x00, 0xA8, 0x3A};

#define SERVER   "192.168.178.242"
#define PORT     1883

//Set up the ethernet client
EthernetClient client;
Adafruit_MQTT_Client mqtt(&client, SERVER, PORT);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish proximityPublish = Adafruit_MQTT_Publish(&mqtt, "proximity/lift");
Adafruit_MQTT_Publish toggleRelayPublish = Adafruit_MQTT_Publish(&mqtt, "relay/states");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe toggleRelaySubcription = Adafruit_MQTT_Subscribe(&mqtt, "toggle/relay", MQTT_QOS_1);

void setupMqttClient() {
  Serial.begin(57600);

  Serial.println(F("Adafruit MQTT demo"));

  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(macMqtt);
  delay(1000); //give the ethernet a second to initialize


  mqtt.subscribe(&toggleRelaySubcription);
}

uint32_t x = 0;

void mqttLoop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &toggleRelaySubcription) {
      deserializeToggleRelayMessage();
    }
  }

  if (flagPublishProximity) {
    publishMessageProximityChanged();

    flagPublishProximity = false;
  }

  // ping the server to keep the mqtt connection alive
  if (! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void deserializeToggleRelayMessage() {
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(3) + 40;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, (char *)toggleRelaySubcription.lastread);
  
  int toggleRelayId = doc["toggle"];

  if (toggleRelayId) {
    toggleRelay(toggleRelayId);
  }
}

void publishMessageProximityChanged() {
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(3);
  char payload[capacity];
  DynamicJsonDocument doc(capacity);
  JsonArray sensors = doc.createNestedArray("proximity");

  addBinDropToJsonArray(sensors);
  addBinLoadToJsonArray(sensors);

  serializeJson(doc, payload);

  if (! proximityPublish.publish(payload)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

void publishMessageRelayStates() {
  const size_t capacity = JSON_ARRAY_SIZE(8) + JSON_OBJECT_SIZE(1) + 8*JSON_OBJECT_SIZE(2);
  char payload[capacity];
  DynamicJsonDocument doc(capacity);
  JsonArray items = doc.createNestedArray("relays");

  addRelayArrayToJsonArray(items);

  serializeJson(doc, payload);

  if (! toggleRelayPublish.publish(payload)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}
