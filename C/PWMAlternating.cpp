#include <Arduino.h>
#include <math.h>

int count = 0;
int PWMPercentage = 5;
unsigned long percentageTime = 0;

unsigned int deadTime = 10;

int PWMfreq = 350; 
float sineFreq = 0.1;
unsigned long sineFreqTime;
unsigned long PWMfreqTime;

int forw = 0b00000010;
int rev = 0b00000100;
int mask;

void setup() {
    mask = forw | rev;

    R_PORT3->PDR |= mask; 
    R_PORT3->PORR = mask;
    
    PWMfreqTime = (unsigned long)(1000000 / PWMfreq);
    percentageTime = PWMPercentage * PWMfreqTime / 100;
    sineFreqTime = 1000 / sineFreq;
    
    delay(1000);
    Serial.println("Working");
}

void loop() {
    unsigned long curTime = micros() % PWMfreqTime;
    unsigned long milsec = millis() % sineFreqTime;
    
    if (curTime < percentageTime) {
        if (PWMPercentage > 0) {
            R_PORT3->PORR = rev; // turn off reverse
            delayMicroseconds(deadTime);
            R_PORT3->POSR = forw; 
        } else {
            R_PORT3->PORR = forw; // turn off forwards
            delayMicroseconds(deadTime);
            R_PORT3->POSR = rev; 
        }
    } else {
        R_PORT3->PORR = mask; // off 
    }

    PWMPercentage = sin(((float)milsec / (float)sineFreqTime) * PI * 2) * 20;
    percentageTime = abs(PWMPercentage) * PWMfreqTime / 100;
}