
void printEthernetAdapterInfo() {
    Serial.print("Our link-local address is: ");
    ether.linkLocalAddress().println();
    Serial.print("Our global address is: ");
    ether.globalAddress().println();
}

void printRemoteAddress() {
    Serial.print("Remote address: ");
    udp.remoteAddress().println();
}

void printMessage(String message) {
    Serial.println(message);
}
