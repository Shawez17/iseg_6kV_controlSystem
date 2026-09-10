#include "dac_control.h"

void setupDacModule(DFRobot_GP8XXX_IIC& dac) {
  dac.setDACOutVoltage(DAC_CODE_0, 0);
  dac.setDACOutVoltage(DAC_CODE_1, 1);
  delay(SETTLE_DELAY_MS);
}

void updateDacOutputs(DFRobot_GP8XXX_IIC& dac, const SystemState& state) {
  (void)state;
  dac.setDACOutVoltage(DAC_CODE_0, 0);
  dac.setDACOutVoltage(DAC_CODE_1, 1);
  delay(SETTLE_DELAY_MS);
}
