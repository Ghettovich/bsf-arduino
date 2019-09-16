
#include <EtherCard.h>
#include <IPAddress.h>

#define ETHERCARD_UDPSERVER   1

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

// We Are in Development!

static void my_result_cb (byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.println((const char*) Ethernet::buffer + off);
}

void onListenUdpConfig(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len)
{

  IPAddress src(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

  Serial.print("dest_port: ");
  Serial.println(dest_port);
  Serial.print("src_port: ");
  Serial.println(src_port);

  Serial.print("src_ip: ");
  ether.printIp(src_ip);
  Serial.println("data: ");
  Serial.println(data);

  const char* response = "config error";
  String msg = String(data);

  Serial.println(msg);

  if (msg.equals("INIT")) {
    ether.browseUrl(PSTR("/30000/"), "56", webrelay, my_result_cb);
  }
  //ether.copyIp(ether.hisip, serverIP);
  ether.makeUdpReply(response, strlen(response), src_port);
}

void onListenUdpLift(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len)
{
  IPAddress src(src_ip[0], src_ip[1], src_ip[2], src_ip[3]);

  const char* response = "lift error";
  String msg = String(data);

  if (msg.equals("LIFT_UP_STOP")) {
    response = "stop going up";
    ether.browseUrl(PSTR("/30000/"), "0", webrelay, my_result_cb);
  }
  else if (msg.equals("LIFT_UP")) {
    response = "going up";
    ether.browseUrl(PSTR("/30000/"), "1", webrelay, my_result_cb);
  }
  else if (msg.equals("LIFT_DOWN_STOP")) {
    response = "stop going down";
    ether.browseUrl(PSTR("/30000/"), "2", webrelay, my_result_cb);
  }
  else if (msg.equals("LIFT_DOWN")) {
    response = "going down";
    ether.browseUrl(PSTR("/30000/"), "3", webrelay, my_result_cb);
  }
  else {
    Serial.println("could not convert data");
  }
  
  ether.makeUdpReply(response, strlen(response), src_port);
}

void onListenUdpBin(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len)
{

}

void setup() {

  Serial.begin(57600);
  Serial.println(F("\n[webClient]"));
  // Change 'SS' to your Slave Select pin if you aren't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
    Serial.println("Failed init adapter");

  ether.staticSetup(myip, gwip, NULL, mask);

  // copy webRelay to hisip so it knows where to send the reply
  // TODO: INSPECT WHEN REPLYING
  ether.copyIp(ether.hisip, webRelayIP);
  ether.printIp("Server: ", ether.hisip);

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);
  // Register udpSerialPrint() to port
  ether.udpServerListenOnPort(&onListenUdpConfig, configPort);
  ether.udpServerListenOnPort(&onListenUdpLift, liftPort);
  ether.udpServerListenOnPort(&onListenUdpBin, binPort);
}

void loop() {
  // This must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
}
