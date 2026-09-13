/**
 * state.h
 * Authors: Vijay Mocherla & Shawez Shaik
 * 
 * This file contains the state management for the HV Supply.
 * 
*/

#pragma once
#include <Arduino.h>

struct systemState {
    float ch1Voltage; // Channel 1 voltage
    float ch2Voltage; // Channel 2 voltage
    float ch1Current; // Channel 1 current
    float ch2Current; // Channel 2 current
    float ch1SetVoltage; // Channel 1 set voltage
    float ch2SetVoltage; // Channel 2 set voltage
};

struct systemConfig {
    bool debugMode; // Debug mode flag
    bool remoteMode; // Remote mode flag
    uint8_t ipAddress[4]; // IP address
    uint8_t subnetMask[4]; // Subnet mask
    uint8_t gateway[4]; // Gateway address
    uint16_t port; // Port number
    bool dacEnabled; // DAC enabled flag
    bool adcPosEnabled; // Positive ADC enabled flag
    bool adcNegEnabled; // Negative ADC enabled flag
};
