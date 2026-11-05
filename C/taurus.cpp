#include <Arduino.h>
#include <math.h>

enum modes {
    OFF,
    ASYNC,
    SYNC,
    SHEPWM
};

modes switchingMode = ASYNC;

float Sine1 = 0;
float Sine2 = 0;
float Sine3 = 0;

unsigned int deadTime = 4;

float PWMfreq = 400; 
float sineFreq = 0;
float power = 0;
float throttle = 0;
float sineFreqTime;
unsigned long sineFreqTimeInt;

float noOfSwitches = 12;

float tau;
float offset120deg;
float offset240deg;

float repeatInterval;
unsigned long repeatIntervalInt;
int potVal;

float PWMfreqTime;
unsigned long PWMfreqTimeInt;

float completion = 0;
unsigned long prevTime = 0;
unsigned long microsec = 0;

int high1;
int high2;
int high3;
int low1;
int low2;
int low3;

int analogueStick = 18;
int button = 17;
float ADCSteps = (1 << 10) - 1;
float analogueStickZero;

int highMask;
int lowMask;
int fullMask;
int otherBits;

int itteration = 0;

void setup() {
    tau = PI * 2;
    offset120deg = tau * 1/3;
    offset240deg = tau * 2/3;

    otherBits = 0xFF & ~fullMask;

    high1  = 1 << digitalPinToBspPin(2); // Digital 2
    high2  = 1 << digitalPinToBspPin(3); // Digital 3
    high3  = 1 << digitalPinToBspPin(4); // Digital 4
    low1   = 1 << digitalPinToBspPin(5); // Digital 5
    low2   = 1 << digitalPinToBspPin(6); // Digital 6
    low3   = 1 << digitalPinToBspPin(7); // Digital 7
    
    int enable = 1 << digitalPinToBspPin(19); // Analogue 5

    highMask = high1 | high2 | high3;
    lowMask = low1 | low2 | low3;

    fullMask = highMask | lowMask;

    R_PORT1->PDR  |= fullMask | enable; // set all pins as output pins
    R_PORT1->PORR |= fullMask | enable; // set all pins to off
    
    sineFreqTime = 1000000.0f / sineFreq;
    sineFreqTimeInt = (unsigned long)sineFreqTime;
    PWMfreqTime = 1000000.0f / PWMfreq;
    PWMfreqTimeInt = (unsigned long)PWMfreqTime;

    repeatInterval = 5000.0f;
    repeatIntervalInt = (unsigned long)(repeatInterval * tau);

    analogueStickZero = (float)analogRead(analogueStick) / ADCSteps ;

    delay(1000);

    R_PORT1->POSR |= enable; // pin for relay connecting mains

    delay(1000);
}

float frequencies[] = {
    73.42, 82.41, 87.31, 98, 110, 123.47, 130.81, 146.83, 164.81, 174.61, 196, 220, 246.94, 261.63, 293.66
};

void loop() {
    unsigned long rawMicroSec = micros();
    microsec += rawMicroSec - prevTime;
    if (microsec > sineFreqTimeInt) microsec = 0;
    float curTime = (float)(microsec % PWMfreqTimeInt);
    completion = (float)(microsec) / sineFreqTime;
    prevTime = rawMicroSec;
    

    // Swap one of the phases to change direction
    if (sineFreq > 0.01f) {
        Sine1 = sin(completion * tau);
        Sine2 = sin(completion * tau + offset120deg);
        Sine3 = sin(completion * tau + offset240deg);
    } else if (sineFreq < -0.01f) {
        Sine1 = sin(completion * tau + offset120deg) ;
        Sine2 = sin(completion * tau);
        Sine3 = sin(completion * tau + offset240deg);
    } else {
        Sine1 = 0.0f;
        Sine2 = 0.0f;
        Sine3 = 0.0f;
    }

    int mask;

    switch (switchingMode)
    {
        case SYNC:
        {
            float triangleSync = abs(fmod(noOfSwitches * completion, 2.0f) - 1);
            float percentageTime1 = triangleSync * PWMfreqTime;

            int pattern1 = Sine1 > 0 ? high1 : low1;
            int pattern2 = Sine2 > 0 ? high2 : low2;
            int pattern3 = Sine3 > 0 ? high3 : low3;

            int mask1 = abs(Sine1 * power) > triangleSync ? pattern1 : 0;
            int mask2 = abs(Sine2 * power) > triangleSync ? pattern2 : 0;
            int mask3 = abs(Sine3 * power) > triangleSync ? pattern3 : 0;

            mask = mask1 | mask2 | mask3;
            break;
        }
        case ASYNC:
        {
            float percentageTime1 = abs(Sine1 * power) * PWMfreqTime;
            float percentageTime2 = abs(Sine2 * power) * PWMfreqTime;
            float percentageTime3 = abs(Sine3 * power) * PWMfreqTime;

            int mask1 = (Sine1 > 0 ? high1 : low1);
            int mask2 = (Sine2 > 0 ? high2 : low2);
            int mask3 = (Sine3 > 0 ? high3 : low3);
            
            mask = ((curTime < percentageTime1) ? mask1 : 0)
                    | ((curTime < percentageTime2) ? mask2 : 0)
                    | ((curTime < percentageTime3) ? mask3 : 0);
            break;
        }
        default:
        {
            delayMicroseconds(deadTime);
            mask = 0;
            break;
        }
    }
    
    R_PORT1->PORR |= ~mask & fullMask; // turn off reverse
    delayMicroseconds(deadTime);
    R_PORT1->POSR |= mask;

    if ((itteration % 101) == 0) {
        sineFreq = sineFreq + throttle;
        if (throttle < 0.0f && sineFreq > 1.0f && sineFreq < 5.0f) sineFreq = 0.0f, throttle = 0.0f;
        if (throttle > 0.0f && sineFreq < -1.0f && sineFreq > -5.0f) sineFreq = 0.0f, throttle = 0.0f;
        float absSineFreq = abs(sineFreq);
        if (absSineFreq < 8.0f) {
            switchingMode = ASYNC;
            PWMfreq = frequencies[min(14, (int)(absSineFreq * 2))];
        } else if (absSineFreq < 15.0f) {
            switchingMode = ASYNC;
            PWMfreq = 300;
        } else if (absSineFreq < 25.0f) {
            switchingMode = SYNC;
            noOfSwitches = 9 * 4;
        } else if (absSineFreq < 35.0f) {
            switchingMode = SYNC;
            noOfSwitches = 7 * 4;
        } else {
            switchingMode = SYNC;
            noOfSwitches = 5 * 4;
        }
        power = min(0.5f, absSineFreq / 25.0f);
        sineFreqTime = 1000000.0f / absSineFreq;
        sineFreqTimeInt = (unsigned long)sineFreqTime;
        PWMfreqTime = 1000000.0f / PWMfreq;
        PWMfreqTimeInt = (unsigned long)PWMfreqTime;
    }
    // if ((itteration % 771) == 0) {
    //     if ((float)analogRead(button) / ADCSteps > 0.5f) {
    //         sineFreq = 0;
    //         power = 0;
    //         sineFreqTime = 1;
    //         sineFreqTimeInt = (unsigned long)sineFreqTime;
    //     }
    // }

    if (++itteration > 1000) {
        itteration = 0;
        float analogueVal = (float)analogRead(analogueStick) / ADCSteps - analogueStickZero;
        if (abs(analogueVal) > 0.05f) {
            throttle = analogueVal / 30.0f; 
        } else {
            throttle = 0;
        }
    }
}