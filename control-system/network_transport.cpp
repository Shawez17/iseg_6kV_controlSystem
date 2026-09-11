#include "network_transport.h"

#include "config.h"
#include "serial_commands.h"

#ifdef USE_WIZNET
#include <Ethernet.h>

static EthernetServer server(TCP_PORT);

void initNetworkTransport(SystemState& state) {
  byte mac[6] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
  IPAddress ip(DEFAULT_IP[0], DEFAULT_IP[1], DEFAULT_IP[2], DEFAULT_IP[3]);
  IPAddress subnet(DEFAULT_SUBNET[0], DEFAULT_SUBNET[1], DEFAULT_SUBNET[2], DEFAULT_SUBNET[3]);
  IPAddress gateway(DEFAULT_GATEWAY[0], DEFAULT_GATEWAY[1], DEFAULT_GATEWAY[2], DEFAULT_GATEWAY[3]);

  if (Ethernet.begin(mac) == 0) {
    Ethernet.begin(mac, ip);
  }

  server.begin();
}

void pollNetworkTransport(SystemState& state) {
  EthernetClient client = server.available();
  if (client) {
    handleSerialCommands(client, state, CommandSource::Ethernet);
  }
}

#else

void initNetworkTransport(SystemState& state) {
  // WIZnet not enabled — network transport is a no-op until enabled.
  (void)state;
}

void pollNetworkTransport(SystemState& state) {
  (void)state;
}

#endif
