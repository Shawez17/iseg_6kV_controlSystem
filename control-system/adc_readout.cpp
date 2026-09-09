#include "adc_readout.h"

void setupAdcModule(Adafruit_ADS1115& positive, Adafruit_ADS1115& negative) {
  if (!positive.begin()) {
    Serial.println("Positive ADC failed to initialize.");
  }

  if (!negative.begin()) {
    Serial.println("Negative ADC failed to initialize.");
  }

  positive.setGain(GAIN_ONE);
  negative.setGain(GAIN_ONE);
}

void updateAdcReadings(Adafruit_ADS1115& positive,
                       Adafruit_ADS1115& negative,
                       SystemState& state) {
  const uint8_t channelCount = 4;
  long posSum[channelCount] = {0, 0, 0, 0};
  long negSum[channelCount] = {0, 0, 0, 0};

  for (uint16_t sample = 0; sample < NUM_SAMPLES; ++sample) {
    posSum[0] += positive.readADC_SingleEnded(0);
    posSum[1] += positive.readADC_SingleEnded(1);
    posSum[2] += positive.readADC_SingleEnded(2);
    posSum[3] += positive.readADC_SingleEnded(3);

    negSum[0] += negative.readADC_SingleEnded(0);
    negSum[1] += negative.readADC_SingleEnded(1);
    negSum[2] += negative.readADC_SingleEnded(2);
    negSum[3] += negative.readADC_SingleEnded(3);

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
}
