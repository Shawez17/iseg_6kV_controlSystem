#include "hardware_i2c.h"

namespace {
bool positiveAdcReady = false;
bool negativeAdcReady = false;
}

void setupI2cHardware(TwoWire& bus, int sdaPin, int sclPin) {
  bus.setSDA(sdaPin);
  bus.setSCL(sclPin);
  bus.begin();
}

void scanI2cDevices(TwoWire& bus) {
  byte error;
  byte address;
  int nDevices = 0;

  Serial.println("Scanning Wire1 (I2C1)...");

  for (address = 0x01; address < 0x7F; address++) {
    bus.beginTransmission(address);
    error = bus.endTransmission();

    if (error == 0) {
      Serial.printf(
        "I2C1: device found at address 0x%02X\n",
        address
      );
      nDevices++;
    }
    else if (error != 2) {
      Serial.printf(
        "I2C1: error %u at address 0x%02X\n",
        error,
        address
      );
    }
  }

  if (nDevices == 0) {
    Serial.println("I2C1: No devices found");
  }
}

void setupAdcModule(Adafruit_ADS1115& positive, Adafruit_ADS1115& negative) {
  positiveAdcReady = positive.begin(0x48, &Wire1);
  if (!positiveAdcReady) {
    Serial.println("Positive ADS FAILED");
  } else {
    positive.setGain(GAIN_TWOTHIRDS);
    Serial.println("Positive ADC initialized");
  }

  negativeAdcReady = negative.begin(0x49, &Wire1);
  if (!negativeAdcReady) {
    Serial.println("Negative ADS FAILED");
  } else {
    negative.setGain(GAIN_TWOTHIRDS);
    Serial.println("Negative ADC initialized");
  }
}

void updateAdcReadings(Adafruit_ADS1115& positive,
                       Adafruit_ADS1115& negative,
                       SystemState& state) {
  Serial.println("adc started!");
  const uint8_t channelCount = 4;
  long posSum[channelCount] = {0, 0, 0, 0};
  long negSum[channelCount] = {0, 0, 0, 0};
  Serial.println("adc going into loop!");

  if (!positiveAdcReady || !negativeAdcReady) {
    Serial.println("ADC read skipped: check ADS1115 power, wiring, addresses, and I2C pins");
    return;
  }

  for (uint16_t sample = 0; sample < NUM_SAMPLES; ++sample) {

  Serial.println("POS CH0");
  posSum[0] += positive.readADC_SingleEnded(0);

  Serial.println("POS CH1");
  posSum[1] += positive.readADC_SingleEnded(1);

  Serial.println("POS CH2");
  posSum[2] += positive.readADC_SingleEnded(2);

  Serial.println("POS CH3");
  posSum[3] += positive.readADC_SingleEnded(3);

  Serial.println("NEG CH0");
  negSum[0] += negative.readADC_SingleEnded(0);

  Serial.println("NEG CH1");
  negSum[1] += negative.readADC_SingleEnded(1);

  Serial.println("NEG CH2");
  negSum[2] += negative.readADC_SingleEnded(2);

  Serial.println("NEG CH3");
  negSum[3] += negative.readADC_SingleEnded(3);

  Serial.println("ALL ADC READS DONE");

  if (SAMPLE_DELAY_MS > 0) {
    delay(SAMPLE_DELAY_MS);
  }
}

  const float positiveScale = 1.0f / static_cast<float>(NUM_SAMPLES);
  const float negativeScale = 1.0f / static_cast<float>(NUM_SAMPLES);

  float posAvg[channelCount];
  float negAvg[channelCount];

  for (uint8_t i = 0; i < channelCount; ++i) {
    posAvg[i] = static_cast<float>(posSum[i]) * positiveScale;
    negAvg[i] = static_cast<float>(negSum[i]) * negativeScale;
  }


  Serial.println("adc computing voltage!");
  state.vref_pos = positive.computeVolts(posAvg[0]);
  state.vmon_pos = positive.computeVolts(posAvg[1]);
  state.imon_pos = positive.computeVolts(posAvg[2]);
  state.vset_pos = positive.computeVolts(posAvg[3]);
  state.hv_pos = HV_FACTOR_POS * state.vset_pos;

  state.vref_neg = negative.computeVolts(negAvg[0]);
  state.vmon_neg = negative.computeVolts(negAvg[1]);
  state.imon_neg = negative.computeVolts(negAvg[2]);
  state.vset_neg = negative.computeVolts(negAvg[3]);
  state.hv_neg = HV_FACTOR_NEG * state.vset_neg;

  state.trend_hv_pos[state.trend_head] = state.hv_pos;
  state.trend_hv_neg[state.trend_head] = state.hv_neg;
  state.trend_imon_pos[state.trend_head] = state.imon_pos;
  state.trend_imon_neg[state.trend_head] = state.imon_neg;
  state.trend_head = static_cast<uint8_t>((state.trend_head + 1) % TREND_SAMPLES);
  if (state.trend_head == 0) {
    state.trend_full = true;
  }
}

void flush_to_serial(const SystemState& state) {
  Serial.println("========== Serial monitor- ADC READINGS ==========");

  Serial.printf("POS: Vref = %.4f V, Vmon = %.4f V, Imon = %.4f V, Vset = %.4f V, HV = %.4f V\n",
                state.vref_pos,
                state.vmon_pos,
                state.imon_pos,
                state.vset_pos,
                state.hv_pos);

  Serial.printf("NEG: Vref = %.4f V, Vmon = %.4f V, Imon = %.4f V, Vset = %.4f V, HV = %.4f V\n",
                state.vref_neg,
                state.vmon_neg,
                state.imon_neg,
                state.vset_neg,
                state.hv_neg);

  Serial.println("==================================");
}

void setupDacModule(DFRobot_GP8XXX_IIC& dac, SystemState& state) {
  //state.dac_code_0 = 32767; // Set DAC channel 0 to mid-scale (0V)

    if ((dac.begin()), &Wire1) {
        Serial.println("GP8413 has initialised");
    }
    else{
        Serial.println("GP8413 has NOT initialized.");
    }
  

  dac.setDACOutRange(dac.eOutputRange5V);
  state.dac_code_0 = 32767; // Set DAC channel 0 to mid-scale (0V)
  state.dac_code_1 = 16383; // Set DAC channel 1 to quarter-scale
  dac.setDACOutVoltage(state.dac_code_0, 0);
  Serial.println("dac channel 0 set to 5V");
  dac.setDACOutVoltage(state.dac_code_1, 1);
  Serial.println("dac channel 1 set to 2.5V");
  delay(SETTLE_DELAY_MS);
}

void updateDacOutputs(DFRobot_GP8XXX_IIC& dac, const SystemState& state) {
  dac.setDACOutVoltage(state.dac_code_0, 0);
  Serial.println("dac channel 0 updates..");
  dac.setDACOutVoltage(state.dac_code_1, 1);
  Serial.println("dac channel 1 updates..");
  delay(SETTLE_DELAY_MS);
}
