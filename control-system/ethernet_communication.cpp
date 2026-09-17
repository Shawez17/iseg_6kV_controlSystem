/**
 * ethernet_communication.cpp
 *
 * DHCP-only Ethernet transport for the on-board WIZnet W5100S.
 * Modeled after WIZnet's official W5100S-EVB-Pico Arduino examples
 * (DHCP_IP_Address / TCP_Server): Ethernet.init(csPin) followed by
 * Ethernet.begin(mac) to obtain a DHCP lease, then a plain EthernetServer
 * for the SCPI-style command parser. There is no static-IP fallback; if a
 * lease cannot be obtained, acquisition is retried periodically.
 */
#include "hardware/watchdog.h"

#include "ethernet_communication.h"

#include <SPI.h>
#include <Ethernet.h>

#include "config.h"
#include "error_handling.h"
#include "serial_communication.h"

namespace {

EthernetServer server(TCP_PORT);
EthernetClient client;
bool dhcpAcquired = false;
uint32_t lastDhcpAttemptMs = 0;

void printStatus(const String& message) {
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

bool acquireDhcpLease() {
  printStatus("Requesting DHCP lease...");

  // Ethernet.begin() blocks internally for up to
  // ETHERNET_DHCP_TIMEOUT_MS + ETHERNET_DHCP_RESPONSE_TIMEOUT_MS with no
  // opportunity to pet the watchdog mid-call, so widen the watchdog window
  // just for the duration of this call and restore the tight steady-state
  // timeout immediately after.
  watchdog_enable(WATCHDOG_STARTUP_TIMEOUT_MS, true);
  const bool leaseObtained = Ethernet.begin(const_cast<uint8_t*>(ETHERNET_MAC_ADDRESS),
                                            ETHERNET_DHCP_TIMEOUT_MS,
                                            ETHERNET_DHCP_RESPONSE_TIMEOUT_MS) != 0;
  watchdog_enable(I2C_WATCHDOG_TIMEOUT_MS, true);

  if (!leaseObtained) {
    return false;
  }

  printStatus("DHCP lease acquired: " + Ethernet.localIP().toString());
  return true;
}

}  // namespace

void initializeEthernetCommunication() {
  Ethernet.init(WNET_CS_PIN);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    raiseError(Serial, ErrorCode::Transport, "TRANSPORT", "W5100S hardware not detected");
    return;
  }

  lastDhcpAttemptMs = millis();
  dhcpAcquired = acquireDhcpLease();

  if (!dhcpAcquired) {
    raiseError(Serial, ErrorCode::Transport, "TRANSPORT",
               "DHCP lease not acquired; will keep retrying");
    return;
  }

  server.begin();
}

bool ethernetInterfaceReady() {
  return dhcpAcquired && (Ethernet.localIP() != IPAddress(0, 0, 0, 0));
}

IPAddress ethernetLocalIP() {
  return Ethernet.localIP();
}

void pollEthernetCommunication(SystemState& state) {
  if (state.transport_mode != TransportMode::Ethernet) {
    stopClient();
    return;
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    stopClient();
    dhcpAcquired = false;
    return;
  }

  if (!dhcpAcquired) {
    if (millis() - lastDhcpAttemptMs >= ETHERNET_DHCP_RETRY_MS) {
      lastDhcpAttemptMs = millis();
      dhcpAcquired = acquireDhcpLease();
      if (dhcpAcquired) {
        server.begin();
      }
    }
    return;
  }

  // Ethernet.maintain() can internally re-run the same bounded-but-blocking
  // DHCP request/response exchange as acquireDhcpLease() when renewing a
  // lease, so it needs the same temporary watchdog widening.
  watchdog_enable(WATCHDOG_STARTUP_TIMEOUT_MS, true);
  Ethernet.maintain();
  watchdog_enable(I2C_WATCHDOG_TIMEOUT_MS, true);

  if (!client || !client.connected()) {
    stopClient();
    client = server.available();
  }

  if (client && client.connected()) {
    handleSerialCommands(client, state, CommandSource::Ethernet);
  }
}