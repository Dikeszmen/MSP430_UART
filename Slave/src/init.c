#include "../header/init.h"
#include "../header/ADC.h"

 void init(void)
 {
    WDTCTL=WDTPW + WDTHOLD;
    DCOCTL=0;
    BCSCTL1=CALBC1_1MHZ;
    DCOCTL=CALDCO_1MHZ;

    /****************************/

    P1DIR |= RXLED+TXLED;
    P1OUT &= 0x00;
    P2DIR |= 0xFF;
    P2OUT &= 0x00;

    P1DIR |= BIT0;
    P1SEL |= RX+TX ;   // P1.1 UCA0RXD input
    P1SEL2 |=  RX +TX;  // P1.2 UCA0TXD output
    P1DIR |= RXLED + TXLED;
    UCA0CTL0 &= 0x00;
    UCA0CTL1 |= UCSSEL_2;
    UCA0BR0 = 0x6D;   //9600-> 109|d=0x6D BAUD
    UCA0BR1 = 0x00;
    UCA0MCTL=UCBRS0+UCA0BR1 ;
    UCA0CTL1 &= ~UCSWRST;
    IE2 |= UCA0RXIE;    //UC0IE
    __enable_interrupt();
    UCA0CTL1 &= ~UCSWRST;
    ConfigureAdc();

 }
