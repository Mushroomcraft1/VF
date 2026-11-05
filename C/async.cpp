#include <Arduino.h>
#include <math.h>

float Sine1 = 0;
float Sine2 = 0;
float Sine3 = 0;
unsigned long percentageTime1 = 0;
unsigned long percentageTime2 = 0;
unsigned long percentageTime3 = 0;

unsigned int deadTime = 100;

int PWMfreq = 400; 
float sineFreq = 0.2;
float power = 0.1;
float sineFreqTime;
unsigned long sineFreqTimeInt;

float tau;
float offset120deg;
float offset240deg;

float repeatInterval;
unsigned long repeatIntervalInt;
int potVal;

float PWMfreqTime;
unsigned long PWMfreqTimeInt;

int high1  = 0b00000001;
int low1   = 0b00000010;
int high2  = 0b00000100;
int low2   = 0b00001000;
int high3  = 0b00010000;
int low3   = 0b00100000;

int enable = 0b00000010;

int highMask;
int lowMask;
int fullMask;

void setup() {
    tau = PI * 2;
    offset120deg = tau * 1/3;
    offset240deg = tau * 2/3;

    highMask = high1 | high2 | high3;
    lowMask = low1 | low2 | low3;

    fullMask = highMask | lowMask;

    R_PORT1->PDR  |= fullMask; // set all pins as output pins
    R_PORT1->PORR |= fullMask; // set all pins to off
    R_PORT2->PDR  |= enable; // set all pins as output pins
    R_PORT2->PORR |= enable; // set enable to off
    
    sineFreqTime = 1000.0f / sineFreq;
    sineFreqTimeInt = (unsigned long)sineFreqTime;
    PWMfreqTime = 1000000.0f / PWMfreq;
    PWMfreqTimeInt = (unsigned long)PWMfreqTime;

    repeatInterval = 5000.0f;
    repeatIntervalInt = (unsigned long)(repeatInterval * tau);

    delay(1000);

    R_PORT2->POSR |= enable; // pin for relay connecting mains

    delay(1000);
}

void loop() {
    if (sineFreq < 0.1f && sineFreq > -0.1f) {
        R_PORT1->PORR |= fullMask; // set all pins to off
        delay(1);
    } else {
        float curTime = (float)(micros() % PWMfreqTimeInt);
        unsigned long milsec = millis() % sineFreqTimeInt;
        float completion = (float)milsec / sineFreqTime;


        // Swap one of the phases to change direction
        if (sineFreq > 0.0f) {
            Sine1 = sin(completion * tau);
            Sine2 = sin(completion * tau + offset120deg);
            Sine3 = sin(completion * tau + offset240deg);
        } else {
            Sine1 = sin(completion * tau + offset120deg) ;
            Sine2 = sin(completion * tau);
            Sine3 = sin(completion * tau + offset240deg);
        }

        percentageTime1 = abs(Sine1 * power) * PWMfreqTime;
        percentageTime2 = abs(Sine2 * power) * PWMfreqTime;
        percentageTime3 = abs(Sine3 * power) * PWMfreqTime;

        int mask1 = (Sine1 > 0 ? high1 : low1);
        int mask2 = (Sine2 > 0 ? high2 : low2);
        int mask3 = (Sine3 > 0 ? high3 : low3);
        
        int mask = ((curTime < percentageTime1) ? mask1 : 0)
                | ((curTime < percentageTime2) ? mask2 : 0)
                | ((curTime < percentageTime3) ? mask3 : 0);

        
        R_PORT1->PORR = (~mask) & fullMask; // turn off reverse
        delayMicroseconds(deadTime);
        R_PORT1->POSR = mask;
    }
}
