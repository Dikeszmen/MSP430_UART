#include "../header/ADC.h"



/*
MSP430g2553 has 12 Bits ADC and can use for inner tempsensor (diode)
    This for ADC10CTL0:
        - REFBURST - STOP INNER VOLTAGE GENERATOR IF REFOUT=1
        When REFOUT = 1, the REFBURST bit controls the operation of the internal reference buffer.
        When REFBURST = 0, the buffer is on continuously, allowing the reference voltage to be
        present outside the device continuously. When REFBURST = 1, the buffer is automatically
        disabled when the ADC10 is not actively converting and is automatically re-enabled when needed.
        - ADC10SC - START ADC
        - ENC
        - AD10SSELx

    ADC10CTl1:
        - ADC10BUSY
        - ADC10MEM
        - ADC10DF format
        - SHS_0  For ADC10SC not for any timer

        Resetting ENC in single-channel single-conversion mode stops a conversion immediately and the
        results are unpredictable. For correct results, poll the ADC10BUSY bit until reset
        before clearing ENC.
        584 pages for calibrate tempsensor

*/


void ConfigureAdc(void)
{


   ADC10CTL1 = INCH_10 | SHS_0 | ADC10DIV_3 | ADC10DF | ADC10SSEL_2 ; // Channel 5, ADC10CLK/4

   ADC10CTL0 =  ADC10SHT_1 |REFON | REF2_5V | ADC10SHT_3 | ADC10ON /*| ADC10IE */; //Vcc & Vss as reference   | ADC10IF?

   ADC10CTL0 &= ~(ENC + ADC10SC);

}



void ADCWorking(char* result)
{


    int degC,temp;
    ADC10CTL0 |= ENC+ADC10SC;

    while(BUSY)
        ;
    ADC10CTL0 &= ~(ENC + ADC10SC);
    temp=ADC10MEM;

    degC=((temp-673)*423)/ADC10;
    result[0] = degC & 0xff;
    result[1] |= (degC>>BYTE) & 0xff;

}
