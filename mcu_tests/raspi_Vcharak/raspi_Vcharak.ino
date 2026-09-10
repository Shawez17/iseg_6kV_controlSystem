#include <Arduino.h>
#include "pico/unique_id.h"
#include "hardware/flash.h"
#include "hardware/structs/sysinfo.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(2000); // give serial monitor time to connect

  Serial.println("========================================");
  Serial.println("      RP2040 BOARD INFORMATION");
  Serial.println("========================================");

  // Chip family
  Serial.print("Chip: ");
#if defined(PICO_RP2350)
  Serial.println("RP2350");
#elif defined(PICO_RP2040)
  Serial.println("RP2040");
#else
  Serial.println("Unknown");
#endif

  // Board name as defined by the core
#ifdef ARDUINO_BOARD
  Serial.print("Arduino Board: ");
  Serial.println(ARDUINO_BOARD);
#endif

#ifdef PICO_BOARD
  Serial.print("Pico SDK Board: ");
  Serial.println(PICO_BOARD);
#endif

  // Flash size
  Serial.print("Flash size: ");
  Serial.print(PICO_FLASH_SIZE_BYTES / (1024 * 1024));
  Serial.println(" MB");

  // Unique board/flash ID (like a serial number)
  Serial.print("Unique Board ID: ");
  pico_unique_board_id_t id;
  pico_get_unique_board_id(&id);
  for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++) {
    if (id.id[i] < 0x10) Serial.print("0");
    Serial.print(id.id[i], HEX);
  }
  Serial.println();

  // SDK version
  Serial.print("Pico SDK Version: ");
  Serial.println(PICO_SDK_VERSION_STRING);

  // CPU frequency
  Serial.print("CPU Frequency: ");
  Serial.print(F_CPU / 1000000);
  Serial.println(" MHz");

  // Number of cores (RP2040 = 2)
  Serial.print("CPU Cores: ");
  Serial.println(2);

  // Chip version / revision from sysinfo
  uint32_t chip_id = sysinfo_hw->chip_id;
  Serial.print("Chip ID register: 0x");
  Serial.println(chip_id, HEX);

  Serial.println("========================================");
}

void loop() {
  // nothing
}
