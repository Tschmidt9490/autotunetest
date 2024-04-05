#include <Arduino.h>
#include <AutotunePID.h>
#include <Adafruit_MAX31865.h>
#include "Wire.h"


/*****************************/
//Pins for the SPI 
/*****************************/
#define MOSI 23
#define MISO 19
#define CLK 18
#define EVAP_CS 21 //Chip select for the EVAP max chip
#define TRAY_CS 5 //Chip select for the TRAY max chip STRAPPING PIN!!!
/*****************************/
//IO signal pins
/*****************************/
#define FUEL 2 //ADC2_CH3, TOUCH_CH3, MTDO, PWM, STRAPPING PIN!!!! SHOULD BE HIGH OR FLOATING DURING BOOT OTHERWISE U0TXD Silent
#define SPILL 34 //INPUT ONLY, ADC
#define GLOW_AC 15 //ADC2_CH0, TOUCH_CH0, PWM (For AC control) SIG4
#define HEATER_AC 4 //ADC2_CH2, TOUCH_CH2, PWM (For AC control) SIG5, STRAPPING PIN!!!! SHOULD BE 0 FOR DOWNLOAD BOOT!
#define SIG1 36 //ADC, PWM, INPUT ONLY, S_VP
#define SIG2 39 //ADC, PWM, INPUT ONLY, S_VN
#define PWM_ATOMIZER 22 //DIGITAL ONLY, PWM, WIRE SCL, 
#define SIG3 35 //INPUT ONLY, ADC
#define FAN_PWM 12 //ADC, PWM, TOUCH, MTDI STRAPPING PIN!!!! SHOULD BE LOW ON BOOT. WHEN HIGH INTERNAL LDO GOES TO 1.8V
/*****************************/
//pins for display UART
/*****************************/
#define RX2 17
#define TX2 16
/*****************************/
//Pins for I2C
/*****************************/
#define SDA 14//ADC, PWM, TOUCH
#define SCL 13 // ADC2_CH4, TOUCH_CH4, MTCK, PWM
/*****************************/
//Power pins
/*****************************/
#define MOS2 26 //PWM enabled, connected to 5A mosfet
#define MOS3 25 //PWM enabled, connected to 5A mosfet
#define GLOW_DC 27 //PWM enabled, connected to 5A mosfet and 3A NTC
#define M2 33 //PWM enabled, motor control pin (OUT2)
#define M1 32 //PWM enabled, motor control pin (OUT1)
/*****************************/


Adafruit_MAX31865 EVAP_T = Adafruit_MAX31865(EVAP_CS, MISO, MOSI, CLK);
#define RREF 4200.0
#define RNOMINAL 1000.0

float getEvapTemp() {
    float temperature = EVAP_T.temperature(RNOMINAL, RREF); // Use resistance to calculate temperature
    return temperature; // Return the calculated temperature
}

void applyOutput(float output) {
       ledcWrite(1, output);
}

AutoTunePID autoTuner(getEvapTemp, applyOutput, 100.0); // Setpoint is 100.0





void setup() {
  Serial.begin(115200);
  EVAP_T.begin(MAX31865_2WIRE);
  pinMode(GLOW_AC, OUTPUT);
    ledcSetup(1, 4, 8);
    ledcAttachPin(HEATER_AC, 1);

}



void loop() {
    static bool autotuneRequested = false;

    // Check for en betingelse eller en brugeranmodning om at starte autotuning
    if (!autotuneRequested) {
      digitalWrite(GLOW_AC, HIGH);
        Serial.println("Starter autotuning...");
        autoTuner.startAutoTuneProcess();
        autotuneRequested = true; // Sæt til true for at forhindre gentagne kald
        Serial.println("Autotuning afsluttet.");
    }

    delay(1000);
    Serial.print("Current Temp: "); Serial.print(getEvapTemp());
    Serial.print(" | Output: "); Serial.println("sidste output værdi");
    //Serial.println(getEvapTemp());
}

