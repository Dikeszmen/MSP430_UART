#include <msp430g2553.h>
#include "ADC.h"
#include "crc.h"
#include "init_uart.h"
#include "reading.h"
#include "writing.h"
#define MAXELEMENT 16
#define TX BIT2
#define RX BIT1
#define TXLED BIT0
#define RXLED BIT6


#define LIMIT 2
#define TRUE 1
#define FALSE 0

const char myAddress='A';
volatile int LOOP=TRUE;
LIST *first,*actual,*newpiece;

void setup() {
  // put your setup code here, to run once:

   serialInit(first,actual,newpiece);
    
}

void loop() 
{
      reading(first);
    __bis_SR_register(LPM3_bits + GIE);//LPM3;
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  //__bic_SR_register_on_exit (LPM3_bits);
  P1OUT |= BIT6;
  LIST *newlist=(LIST*)malloc(sizeof(List));
  newlist->data=UCA0RXBUF;
  newlist->next=NULL;
  if(first)
      actual->next=newlist;
  else
      first=newlist;
  actual=newlist;
  P1OUT &= ~BIT6;
}

