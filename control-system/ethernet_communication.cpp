#include "ethernet_communication.h"

#include <Ethernet.h>

#include "config.h"
#include "error_handling.h"
#include "serial_communication.h"

namespace {

constexpr uint8_t kEthernetMac[6] = {0x02, 0x1A, 0x6B, 0x51, 0x00, 0x01};
#ifdef WIZNET_CS_PIN
constexpr int kWiznetCsPin = WIZNET_CS_PIN;
#else
constexpr int kWiznetCsPin = 17;
#endif

EthernetServer server(TCP_PORT);
EthernetClient client;
bool ethernetStarted = false;

void startEthernet() {
  if (ethernetStarted) {
    return;
  }

  if (Ethernet.linkStatus() != LinkON) {
    return;
  }

  Ethernet.init(kWiznetCsPin);
  Ethernet.begin(const_cast<uint8_t*>(kEthernetMac),
                 const_cast<uint8_t*>(DEFAULT_IP),
                 const_cast<uint8_t*>(DEFAULT_GATEWAY),
                 const_cast<uint8_t*>(DEFAULT_GATEWAY),
                 const_cast<uint8_t*>(DEFAULT_SUBNET));
  server.begin();
  ethernetStarted = true;
}

void stopClient() {
  if (client) {
    client.stop();
  }
  client = EthernetClient();
}

}  // namespace

void initializeEthernetCommunication() {
  if (Ethernet.linkStatus() != LinkON) {
    raiseError(Serial, ErrorCode::Transport, "TRANSPORT",
               "Ethernet cable disconnected; USB remains active");
    return;
  }

  startEthernet();
}

bool ethernetClientConnected() {
  return client && client.connected();
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