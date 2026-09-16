#include "ethernet_communication.h"

#include <Ethernet.h>

#include "config.h"
#include "error_handling.h"
#include "serial_communication.h"

namespace {

#ifdef WIZNET_CS_PIN
constexpr int kWiznetCsPin = WIZNET_CS_PIN;
#else
constexpr int kWiznetCsPin = 17;
#endif

EthernetServer server(TCP_PORT);
EthernetClient client;
bool ethernetStarted = false;

void printEthernetStatus(const String& message) {
  if (isSerialHostConnected()) {
    Serial.println(message);
  }
}

void stopClient() {
  if (client) {
    client.stop();
  }
  client = EthernetClient();
}

void configureEthernetStatic() {
  const IPAddress staticIp(DEFAULT_IP[0], DEFAULT_IP[1], DEFAULT_IP[2], DEFAULT_IP[3]);
  const IPAddress subnet(DEFAULT_SUBNET[0], DEFAULT_SUBNET[1], DEFAULT_SUBNET[2], DEFAULT_SUBNET[3]);
  const IPAddress gateway(DEFAULT_GATEWAY[0], DEFAULT_GATEWAY[1], DEFAULT_GATEWAY[2], DEFAULT_GATEWAY[3]);
  const IPAddress dns(DEFAULT_GATEWAY[0], DEFAULT_GATEWAY[1], DEFAULT_GATEWAY[2], DEFAULT_GATEWAY[3]);

  Ethernet.begin(const_cast<uint8_t*>(ETHERNET_MAC_ADDRESS), staticIp, dns, gateway, subnet);

  if (Ethernet.localIP() == IPAddress(0, 0, 0, 0)) {
    raiseError(Serial, ErrorCode::Transport, "TRANSPORT",
               "Static Ethernet configuration failed");
    return;
  }

  printEthernetStatus("Static Ethernet configured");
}

void startEthernet() {
  if (ethernetStarted) {
    return;
  }

  Ethernet.init(kWiznetCsPin);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    raiseError(Serial, ErrorCode::Transport, "TRANSPORT",
               "Ethernet hardware not detected");
    return;
  }

  printEthernetStatus("Initializing Ethernet via DHCP...");
  printEthernetStatus("Using MAC: " + String(ETHERNET_MAC_ADDRESS[0], HEX) + ":" + String(ETHERNET_MAC_ADDRESS[1], HEX) + ":" +
                      String(ETHERNET_MAC_ADDRESS[2], HEX) + ":" + String(ETHERNET_MAC_ADDRESS[3], HEX) + ":" +
                      String(ETHERNET_MAC_ADDRESS[4], HEX) + ":" + String(ETHERNET_MAC_ADDRESS[5], HEX));

  if (ETHERNET_USE_DHCP) {
    Ethernet.begin(const_cast<uint8_t*>(ETHERNET_MAC_ADDRESS));
  } else {
    configureEthernetStatic();
  }

  if (Ethernet.localIP() == IPAddress(0, 0, 0, 0)) {
    stopClient();
    ethernetStarted = false;
    printEthernetStatus("DHCP not ready yet; waiting for cable / DHCP server");
    return;
  }

  server.begin();
  ethernetStarted = true;

  if (isSerialHostConnected()) {
    Serial.print("Ethernet DHCP ready: ");
    Serial.println(Ethernet.localIP());
  }
}

}  // namespace

void initializeEthernetCommunication() {
  startEthernet();

  if (!ethernetStarted) {
    raiseError(Serial, ErrorCode::Transport, "TRANSPORT",
               "Ethernet cable disconnected or DHCP failed; USB remains active");
  }
}

bool ethernetClientConnected() {
  return client && client.connected();
}

bool ethernetInterfaceReady() {
  return ethernetStarted && (Ethernet.localIP() != IPAddress(0, 0, 0, 0));
}

IPAddress ethernetLocalIP() {
  return Ethernet.localIP();
}

void pollEthernetCommunication(SystemState& state) {
  if (state.transport_mode != TransportMode::Ethernet) {
    stopClient();
    return;
  }

  if (Ethernet.linkStatus() != LinkON) {
    stopClient();
    ethernetStarted = false;
    raiseError(Serial, ErrorCode::Transport, "TRANSPORT",
               "Ethernet cable disconnected; falling back to USB");
    if (isSerialHostConnected()) {
      state.transport_mode = TransportMode::Usb;
    }
    return;
  }

  startEthernet();

  if (!client || !client.connected()) {
    stopClient();
    client = server.available();
  }

  if (client && client.connected()) {
    handleSerialCommands(client, state, CommandSource::Ethernet);
  }
}