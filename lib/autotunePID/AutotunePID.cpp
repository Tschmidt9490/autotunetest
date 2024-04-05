#include "AutoTunePID.h"

AutoTunePID::AutoTunePID(float (*inputFunction)(), void (*outputFunction)(float), float setPoint) 
: _inputFunction(inputFunction), _outputFunction(outputFunction), _setPoint(setPoint) {
}

void AutoTunePID::startAutoTuneProcess() {
    warmUpToSetPoint(); // Opvarm systemet til nær setpunktet
    AutoTune(); // Start autotune efter opvarmningen
}

void AutoTunePID::warmUpToSetPoint() {
    float currentTemp = _inputFunction();
    const float threshold = 0.5; // Tolerance for temperatur nær setpunktet
    float output = 0.0;
    const float maxOutput = 255.0;
    const float stepUp = 1.0;
    const float stepDown = 0.5;
    const float zero = 0.0;
    const unsigned long interval = 1000; // Interval mellem justeringer
    unsigned long stabilizationStartTime = 0; // Til at måle stabiliseringstiden
    const unsigned long stabilizationDuration = 5000; // Krævet tid for at betragte systemet som stabiliseret

    Serial.println("Warming up to setpoint...");

    while(true) {
        unsigned long currentTime = millis();
        static unsigned long lastUpdateTime = 0;

        if (currentTime - lastUpdateTime > interval) {
            // Justering af output baseret på aktuel temperatur
            if (currentTemp < _setPoint - threshold) {
                output = min(output + stepUp, maxOutput);
            } else if (currentTemp > _setPoint + threshold) {
                output = max(output - stepDown, zero);
            }
            _outputFunction(output);
            lastUpdateTime = currentTime;
            Serial.print("Current Temp: "); Serial.print(currentTemp);
            Serial.print(" | Output: "); Serial.println(output);
        }

        // Opdater den nuværende temperatur
        currentTemp = _inputFunction();

        // Tjek om vi er indenfor threshold af setpunktet og start stabiliseringstid
        if(abs(currentTemp - _setPoint) <= threshold) {
            if(stabilizationStartTime == 0) {
                stabilizationStartTime = currentTime; // Start stabiliseringstimeren
                Serial.println("Beginning stabilization phase...");
            } else if(currentTime - stabilizationStartTime > stabilizationDuration) {
                // Vi har været indenfor threshold i tilstrækkelig lang tid, antag at systemet er stabiliseret
                Serial.println("System stabilized at setpoint.");
                break; // Bryd loopet og fortsæt til AutoTune
            }
        } else {
            // Hvis vi bevæger os væk fra setpunktet, nulstil stabiliseringstid
            stabilizationStartTime = 0;
        }

        delay(100); // Mindre pause for at undgå at overbelaste CPU'en
    }
}




void AutoTunePID::AutoTune() {
    const float relayAmplitude = 5.0; // Ændringens amplitude i grader Celsius
    bool outputHigh = false; // Styrer, om output er i høj eller lav tilstand
    unsigned long lastSwitchTime = millis(); // Tidspunktet for sidste skifte
    float maxInput = -10000.0, minInput = 10000.0; // Initialiserer til ekstreme værdier
    unsigned long oscillationStart = 0; // Tidspunktet hvor første oscillation starter
    int oscillationCount = 0; // Antal fuldførte oscillationer
    const int desiredOscillations = 5; // Ønsket antal oscillationer før afslutning
    const unsigned long autoTuneTimeout = 120000; // Timeout for autotuning i millisekunder
    unsigned long autoTuneStartTime = millis(); // Starttidspunkt for autotuning

    Serial.println("Starting AutoTune process...");

    while (millis() - autoTuneStartTime < autoTuneTimeout && oscillationCount < desiredOscillations) {
        float input = _inputFunction(); // Læs den aktuelle procesværdi

        if ((outputHigh && input > _setPoint + relayAmplitude) || (!outputHigh && input < _setPoint - relayAmplitude)) {
            outputHigh = !outputHigh;
            _outputFunction(outputHigh ? 255.0 : 0.0);
            lastSwitchTime = millis();

            if (oscillationStart > 0) { // En fuld oscillation er observeret
                oscillationCount++;
                if (oscillationCount >= desiredOscillations) {
                    // Beregn PID-parametre efter det ønskede antal oscillationer
                    _Tu = (millis() - oscillationStart) / (oscillationCount * 1000.0);
                    _Ku = 4.0 * relayAmplitude / (PI * (maxInput - minInput));
                    calculatePID();
                    Serial.println("Autotune completed. Calculating PID parameters...");
                    break; // Afslut autotuningsprocessen
                }
            } else {
                oscillationStart = lastSwitchTime;
            }
        }

        // Opdaterer max og min inputs
        maxInput = max(maxInput, input);
        minInput = min(minInput, input);
    }

    if (oscillationCount < desiredOscillations) {
        Serial.println("Autotune did not complete within the time limit or did not achieve enough oscillations.");
    }
}




void AutoTunePID::GetParam(float &Kp, float &Ki, float &Kd, float &Ku, float &Tu) {
    Kp = _Kp;
    Ki = _Ki;
    Kd = _Kd;
    Ku = _Ku;
    Tu = _Tu;
}

void AutoTunePID::calculatePID() {
    _Kp = 0.6 * _Ku;
    _Ki = 2 * _Kp / _Tu;
    _Kd = _Kp * _Tu / 8;

    // Print beregnede værdier til seriel monitor
    Serial.println("PID Parameters Calculated:");
    Serial.print("Ku: "); Serial.println(_Ku, 6); // 6 decimaler for præcision
    Serial.print("Tu: "); Serial.println(_Tu, 6);
    Serial.print("Kp: "); Serial.println(_Kp, 6);
    Serial.print("Ki: "); Serial.println(_Ki, 6);
    Serial.print("Kd: "); Serial.println(_Kd, 6);
}