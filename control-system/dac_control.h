#pragma once

#include <Arduino.h>
#include <DFRobot_GP8XXX.h>

#include "config.h"
#include "state.h"

void setupDacModule(DFRobot_GP8XXX_IIC& dac);
void updateDacOutputs(DFRobot_GP8XXX_IIC& dac, const SystemState& state);
