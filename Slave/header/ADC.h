#ifndef ADC_H_INCLUDED
#define ADC_H_INCLUDED
#include "/home/herczig/Embedded/MSP/msp430-gcc/include/msp430g2553.h"

#define ADC10 1024
#define BYTE 8

void ConfigureAdc(void);

void ADCWorking(char*);

#endif
