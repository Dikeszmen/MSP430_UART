#include <msp430g2553.h>
#define TX BIT2
#define RX BIT1
#define TXLED BIT0
#define RXLED BIT6
#define LIMIT 128
char myAddress[4];
volatile char *paddress=NULL;
int i=0;
typedef struct FIFO
{
  char data;
  struct FIFO *next;
} List;



void setup() {
  // put your setup code here, to run once:
    //WDTCTL=WDTPW + WDTHOLD;   //Watchdog STOP
    /*Setting inner clock to 1MHz*/
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
    UCA0BR0 = 0x6D;   //9600-> 109
    UCA0BR1 = 0x00;
    UCA0MCTL=UCBRS0+UCA0BR1 ;
    UCA0CTL1 &= ~UCSWRST;
    IE2 |= UCA0RXIE;    //UC0IE
    //__bis_SR_register(LPM3_bits + GIE);
    memset(myAddress,0,5);
     P1OUT |=0x41;
}

void loop() {


 switch (i)
 {
  case 0:
    P1OUT &= ~BIT0;
    P1OUT &= ~BIT6;
    break;
  case 1:
    P1OUT |=BIT0;
    P1OUT &= ~BIT6;
    break;
  case 2:
    P1OUT &= ~BIT0;
    P1OUT |= BIT6;
    break;
  case 3:
    P1OUT |= BIT0;
    P1OUT |= BIT6;
    break;
 }
/*
  if(i)
    {
   
      /*
      //P1OUT |=BIT0;
      //delay(10);
      P1OUT &=~BIT0;
      
      P1OUT |=BIT0;
      delay(50);
      paddress=NULL;
      __bis_SR_register(LPM3_bits + GIE);
    }
 // else
   //P1OUT &=0x00;
   
       */
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  if(i<3)
    myAddress[i++]=UCA0RXBUF;
  else
    {
      i=0;
      myAddress[i]=UCA0RXBUF;
    }
    
   
   //P1IFG=0x00;
   //__bic_SR_register_on_exit (LPM3_bits);
}
