/**
 * HV Supply  
 * author: Vijay Mocherla 
 * email: vijaysai.mocherla@gmail.com
 * 
 * Readout and display system for iseg DPR3040512 HV supply. Please iseg's manual and datasheet for
 * more details about the supply and the pin assignments.
 * 
 * The following voltages are monitored from the module (all in range of 0-5V):
 * - V_vset0: reference voltage for setpoint, used for both current and voltage output
 * - V_imon: monitor voltage for current output
 * - V_vmon: monitor voltage for voltage output
 * 
 **/
#include <Wire.h>
// #include <LiquidCrystal_I2C.h>

// LiquidCrystal_I2C lcd(0x27, 16, 2); // Change to 0x3F if needed

// HV Supply 01
// const float FACTOR_HV=663.5191;   // Conversion factor for HV_Out
// const float FACTOR_I=0.757;       // Conversion factor for I_Out (mA/V)

// HV Supply 02
const float FACTOR_HV=-654.1922;   // Conversion factor for HV_Out
const float FACTOR_I=0.757;       // Conversion factor for I_Out (mA/V)

// HV Supply 03
// const float FACTOR_HV=651.519139;   // Conversion factor for HV_Out
// const float FACTOR_I=0.757;       // Conversion factor for I_Out (mA/V)


const int navg = 50;
float read_delay = 20;
// readout array 
float v_readout[3] = {0.0, 0.0, 0.0};
// variables for voltages
float v_vmon;
float v_imon;
float v_vset0;
float hv_out;
float i_out;

int debug_mode = 0; // Debug mode flag

void setup() {
    Serial.begin(9600);
    Wire.begin();
    
    // lcd.begin(16, 2);
    // lcd.backlight();

    // Show caution message before enabling HV

    // lcd.setCursor(0, 0);
    // lcd.print("CAUTION: HIGH");
    // lcd.setCursor(0, 1);
    // lcd.print("VOLTAGE SYSTEM!");
    Serial.println("CAUTION: HIGH VOLTAGE SYSTEM!");
    
    delay(3000); // Wait 3 seconds before proceeding
}

// Display values on LCD
// void flush_output_to_LCD(){
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print("HV Out: ");
//     lcd.print(hv_out, 0);
//     lcd.print(" V");

//     lcd.setCursor(0, 1);
//     lcd.print("I_Out: ");
//     lcd.print(i_out, 3);
//     lcd.print(" mA");
// }

// Read voltages from analog pins (0-1023 mapped to 0-5V)
void read_voltages(){
    for (int i=0; i<navg; i++){
        v_readout[0] += analogRead(A0); 
        v_readout[1] += analogRead(A1);
        v_readout[2] += analogRead(A2);
        delay(read_delay);
    };
    // average and covert to voltage
    v_vset0 = v_readout[0]/navg * (5.0 / 1023.0);
    v_imon =  v_readout[1]/navg * (5.0 / 1023.0);
    v_vmon =  v_readout[2]/navg * (5.0 / 1023.0);
    v_readout[0] = 0.0;
    v_readout[1] = 0.0;
    v_readout[2] = 0.0;
    // Convert to HV_Out and I_Out
    hv_out = (v_vmon - v_vset0) * FACTOR_HV;
    i_out =  (v_imon - v_vset0) * FACTOR_I;  
}

// Print to Serial Monitor for debugging
void flush_to_serial_monitor(){
    Serial.print("V_vmon: ");
    Serial.print(v_vmon, 3);
    Serial.print(" V, V_imon: ");
    Serial.print(v_imon, 3);
    Serial.print(" V, V_vset0: ");
    Serial.print(v_vset0, 3);
    Serial.print(" V, HV_Out: ");
    Serial.print(hv_out, 1);
    Serial.print(" V, I_Out: ");
    Serial.print(i_out, 6);
    Serial.println(" mA");
}

// Check for serial commands to enter/exit debug mode
void check_serial_command(){
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil("\r");
        command.trim();
        Serial.println(command);
        if (command == "DEBUG"){
          Serial.println("Entering debug mode...\n");
          debug_mode = 1;
        }
        else if (command == "STOP"){
          Serial.println("Exiting debug mode...\n");
          debug_mode = 0;
        }
        else{
            Serial.println("ERROR\n");
        }
    }
    if (debug_mode){
      flush_to_serial_monitor();
    }
}


void loop() {
    read_voltages();
    // flush_output_to_LCD();
    check_serial_command();
    delay(500);
}
