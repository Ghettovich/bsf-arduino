
#include <EtherCard.h>
#include <IPAddress.h>

#define ETHERCARD_UDPSERVER   1

/* Network configuration */

// Ethernet interface IP address
static byte myip[] = { 192, 168, 178, 19 };
// Gateway IP address
static byte gwip[] = { 192, 168, 178, 1 };
// ethernet interface ip netmask
static byte mask[] = { 255, 255, 255, 0 };
// Ethernet MAC address - must be unique on your network
static byte mymac[] = { 0x70, 0x69, 0x74, 0x2D, 0x30, 0x31 };
// remote website ip address and port
static byte webRelayIP[] = { 192, 168, 178, 18 };
// remote website ip address and port
static byte serverIP[] = { 192, 168, 178, 20 };
// remote website name
const char webrelay[] = "192.168.178.18";
// init adapter
byte Ethernet::buffer[2000]; // TCP/IP send and receive buffer
// server port
int port = 12300;
int configPort = 12309;
int liftPort = 12310;
int binPort = 12320;

/* IO */
int relayValveLiftUp = 25;
int relayValveLiftDown = 27;
int relayBinLoad = 24;
int relayBinDrop = 26;

// INPUT proximity switches !!! if LOW detection !!!
int sensorLiftBottom = 22;
int sensorLiftTop = 23;

//int sensorBinLoad = 26;
//int sensorBinDrop = 28;

// Lift at BOTTOM  BIN at LOAD
bool isLiftUpFree()
{
  if (digitalRead(sensorLiftBottom) == LOW &&
      digitalRead(sensorLiftTop) == HIGH)
  {
    return true;
  }
  else
  {
    return false;
  }
}

// Lift at TOP  BIN at LOAD
bool isLiftDownFree()
{
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == LOW)
  {
    return true;
  }
  else
  {
    return false;
  }
}
// Lift at TOP and BIN at DROP
bool isBinAtDrop()
{
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == LOW)
    //digitalRead(sensorBinLoad) == HIGH &&
    //digitalRead(sensorBinDrop) == LOW)
    return true;
  else
    return false;
}

bool isBinAtLoad()
{
  if (digitalRead(sensorLiftBottom) == LOW)
    return true;
  else
    return false;
}

// Lift  between sensors operator action required
bool isLiftBetweenBottomAndTop()
{
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == HIGH)
    //digitalRead(sensorBinLoad) == LOW &&
    //digitalRead(sensorBinDrop) == HIGH)
    return true;
  else
    return false;
}

// BIN between sensors operator action required
bool isBinBetweenLoadAndDrop()
{
  if (digitalRead(sensorLiftBottom) == HIGH &&
      digitalRead(sensorLiftTop) == LOW)
    //digitalRead(sensorBinLoad) == HIGH &&
    //digitalRead(sensorBinDrop) == HIGH)
    return true;
  else
    return false;
}

void onListenUdpConfig(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len)
{
  IPAddress src(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);
  const char* response = "config error";
  String msg = String(data);
  Serial.println(msg);

  if (msg.equals("INIT")) {
    //ether.browseUrl(PSTR("/30000/"), "56", webrelay, my_result_cb);
  }
  //ether.copyIp(ether.hisip, serverIP);
  ether.makeUdpReply(response, strlen(response), src_port);
}

void onListenUdpLift(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len)
{
  IPAddress src(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

  const char* response = "lift error";
  String msg = String(data);

  if (msg.equals("LIFT_UP_STOP"))
  {
    response = "1";
    digitalWrite(relayValveLiftUp, HIGH);
  }
  else if (msg.equals("LIFT_UP"))
  {
    if (!isLiftUpFree())
    {
      response = "0";
    }
    else
    {
      response = "1";
      digitalWrite(relayValveLiftUp, LOW);
    }
  }

  else if (msg.equals("LIFT_DOWN_STOP"))
  {
    response = "1";
    digitalWrite(relayValveLiftDown, HIGH);
  }
  //lift at TOP  BIN at LOAD)
  else if (msg.equals("LIFT_DOWN"))
  {
    if (!isLiftDownFree())
    {
      response = "0";
    }
    else
    {
      response = "1";
      digitalWrite(relayValveLiftDown, LOW);
    }
  }
  else
  {
    Serial.println("could not read message data");
  }
  Serial.println(response);
  ether.makeUdpReply(response, strlen(response), src_port);
}

void onListenUdpBin(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len)
{
  IPAddress src(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

  const char* response = "bin error";
  String msg = String(data);

  if (msg.equals("BIN_LOAD_STOP"))
  {
    response = "1";
    digitalWrite(relayBinLoad, HIGH);
  }
  else if (msg.equals("BIN_LOAD"))
  {
    response = "1";
    digitalWrite(relayBinLoad, LOW);
  }
  else if (msg.equals("BIN_DROP_STOP"))
  {
    response = "1";
    digitalWrite(relayBinDrop, HIGH);
  }
  else if (msg.equals("BIN_DROP"))
  {
    response = "1";
    digitalWrite(relayBinDrop, LOW);
  }
  else
  {
    Serial.println("could not read message data");
  }
}

void setup() {
  // Register network adapter
  Serial.begin(57600);
  Serial.println(F("\n[webClient]"));
  // Change 'SS' to your Slave Select pin if you aren't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println("Failed init adapter");
  ether.staticSetup(myip, gwip, NULL, mask);

  // copy webRelay to hisip so it knows where to send the reply
  // TODO: INSPECT WHEN REPLYING
  ether.copyIp(ether.hisip, webRelayIP);
  // Register udpSerialPrint() to port
  ether.udpServerListenOnPort(&onListenUdpConfig, configPort);
  ether.udpServerListenOnPort(&onListenUdpLift, liftPort);
  ether.udpServerListenOnPort(&onListenUdpBin, binPort);

  // Register IO
  pinMode(sensorLiftTop, INPUT_PULLUP);
  pinMode(sensorLiftBottom, INPUT_PULLUP);

  pinMode(relayValveLiftUp, OUTPUT);
  digitalWrite(relayValveLiftUp, HIGH);

  pinMode(relayValveLiftDown, OUTPUT);
  digitalWrite(relayValveLiftDown, HIGH);
}

void loop() {
  // This must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
}
