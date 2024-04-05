#ifndef AutoTunePID_h
#define AutoTunePID_h

#include "Arduino.h"

class AutoTunePID {
public:
    AutoTunePID(float (*inputFunction)(), void (*outputFunction)(float), float setPoint);
    void AutoTune();
    void GetParam(float &Kp, float &Ki, float &Kd, float &Ku, float &Tu);
    void startAutoTuneProcess();

private:
    float (*_inputFunction)();
    void (*_outputFunction)(float);
    float _setPoint;
    float _Ku, _Tu, _Kp, _Ki, _Kd;
    // Intern brug
    void calculatePID();
    void warmUpToSetPoint();
    
};

#endif
