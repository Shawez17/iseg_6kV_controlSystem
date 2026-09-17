#include "hardware/watchdog.h" 
#include "hardware_i2c.h"

void setupI2cHardware(TwoWire& bus, int sdaPin, int sclPin) {
  bus.setSDA(sdaPin);
  bus.setSCL(sclPin);
  bus.begin();
}

void scanI2cDevices(TwoWire& bus) {
  byte error;
  byte address;
  int nDevices = 0;

  Serial.println("Scanning I2C bus...");
  for (address = 1; address < 127; address++) {
    bus.beginTransmission(address);
    error = bus.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.println("Done");
  }
}

void setupAdcModule(Adafruit_ADS1115& positive, Adafruit_ADS1115& negative) {
  // ADS1115 GAIN set to 2/3x gain (+/- 6.144V, 1 bit = 0.1875mV).
  positive.setGain(GAIN_TWOTHIRDS);
  negative.setGain(GAIN_TWOTHIRDS);

  if (!positive.begin(ADDR_ADS1115_CH1, &Wire1)) {
    Serial.println("Failed to initialize positive-channel ADS1115. Please check the connection.");
  } else {
    Serial.println("Positive-channel ADS1115 initialized successfully.");
  }

  if (!negative.begin(ADDR_ADS1115_CH2, &Wire1)) {
    Serial.println("Failed to initialize negative-channel ADS1115. Please check the connection.");
  } else {
    Serial.println("Negative-channel ADS1115 initialized successfully.");
  }
}

AdcChannelReadout readAdcChannels(Adafruit_ADS1115& adc) {
  long sum1 = 0;
  long sum2 = 0;
  long sum3 = 0;

  for (uint16_t sample = 0; sample < NUM_SAMPLES; ++sample) {
    watchdog_update();
    sum1 += adc.readADC_SingleEnded(1);

    watchdog_update();
    sum2 += adc.readADC_SingleEnded(2);

    watchdog_update();
    sum3 += adc.readADC_SingleEnded(3);

    if (SAMPLE_DELAY_MS > 0) {
      delay(SAMPLE_DELAY_MS);
    }
  }

  AdcChannelReadout readout;
  const float scale = 1.0f / static_cast<float>(NUM_SAMPLES);
  readout.ch1 = static_cast<float>(sum1) * scale;
  readout.ch2 = static_cast<float>(sum2) * scale;
  readout.ch3 = static_cast<float>(sum3) * scale;
  return readout;
}

namespace {

// Converts one device's averaged ADC counts into Vset/Vmon/Imon/HV and
// writes the result directly into the matching SystemState fields.
void applyChannelReadout(Adafruit_ADS1115& adc, const AdcChannelReadout& readout, float hvFactor,
                         float& vset, float& vmon, float& imon, float& hv) {
  vset = adc.computeVolts(readout.ch1);
  vmon = adc.computeVolts(readout.ch2);
  imon = adc.computeVolts(readout.ch3);
  hv = hvFactor * vmon;
}

}  // namespace

void updateAdcReadings(Adafruit_ADS1115& positive,
                       Adafruit_ADS1115& negative,
                       SystemState& state) {
  const AdcChannelReadout posReadout = readAdcChannels(positive);
  applyChannelReadout(positive, posReadout, HV_FACTOR_POS,
                      state.vset_pos, state.vmon_pos, state.imon_pos, state.hv_pos);

  const AdcChannelReadout negReadout = readAdcChannels(negative);
  applyChannelReadout(negative, negReadout, HV_FACTOR_NEG,
                      state.vset_neg, state.vmon_neg, state.imon_neg, state.hv_neg);

  state.trend_hv_pos[state.trend_head] = state.hv_pos;
  state.trend_hv_neg[state.trend_head] = state.hv_neg;
  state.trend_imon_pos[state.trend_head] = state.imon_pos;
  state.trend_imon_neg[state.trend_head] = state.imon_neg;
  state.trend_head = static_cast<uint8_t>((state.trend_head + 1) % TREND_SAMPLES);
  if (state.trend_head == 0) {
    state.trend_full = true;
  }
}

void setupDacModule(DFRobot_GP8XXX_IIC& dac, SystemState& state) {
  if (dac.begin() != 0) {
    Serial.println("GP8413 DAC initialization failed. Please check the connection.");
  } else {
    Serial.println("GP8413 DAC initialized successfully.");
  }

  dac.setDACOutRange(dac.eOutputRange5V);
  state.dac_code_0 = DAC_BOOT_CODE; // Set DAC channel 0 to boot voltage (0V)
  state.dac_code_1 = DAC_BOOT_CODE; // Set DAC channel 1 to boot voltage (0V)
  dac.setDACOutVoltage(state.dac_code_0, 0);
  dac.setDACOutVoltage(state.dac_code_1, 1);
  delay(SETTLE_DELAY_MS);
}

void updateDacOutputs(DFRobot_GP8XXX_IIC& dac, const SystemState& state) {
  dac.setDACOutVoltage(state.dac_code_0, 0);
  dac.setDACOutVoltage(state.dac_code_1, 1);
  delay(SETTLE_DELAY_MS);
}
