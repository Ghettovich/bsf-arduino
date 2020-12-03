byte mac[] = {0xE8, 0xD4, 0x43, 0x00, 0xA8, 0x3A};


#define AIO_SERVER      "192.168.178.242"
#define AIO_SERVERPORT  1883

//Set up the ethernet client
EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, "proximity/binload");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, "toggle/relay");

void mqttSetup() {
  Serial.begin(57600);

  Serial.println(F("Adafruit MQTT demo"));

  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(mac);
  delay(1000); //give the ethernet a second to initialize
  

  mqtt.subscribe(&onoffbutton);
}

uint32_t x=0;

void mqttLoop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
  }

  // Now we can publish stuff!
  const char data[] = "A not so long string";
  Serial.print(F("\nSending proximity bin load val "));
  Serial.print(x);
  Serial.print("...");
  if (! photocell.publish(data)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
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
