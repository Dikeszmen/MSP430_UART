#include <msp430g2553.h>
#define TX BIT2
#define RX BIT1
#define TXLED BIT0
#define RXLED BIT6
#define LIMIT 128
char myAddress='A';
unsigned int i;
typedef enum PacketState

{
    /*! Default condition*/
    EmptyState,

    /*! min. one 0x55 received*/
    moto55,

    /*! after 0xFF, 0x01 received*/
    moto1,

    /*! Address */
    address,

    /*! Command*/
    command,

    /*! Low byte of datalength*/
    DLenLow,

    /*! High byte of datalength*/
    DLenHigh,

    /*! Databyte */
    Data,

    /*! low byte of crc  Packet*/
    CrcLow,

    /*! high byte of crc  Packet*/
    CrcHigh

} packetState;

typedef struct queueData
{
    /** packet item data */
    char address;
    char cmd;
    uint16_t dlen;
    char *data;
} QueueData;

QueueData *reserve(char data);

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
    //IE2 |= UCA0RXIE;    //UC0IE
    //__bis_SR_register(CPUOFF + GIE);
    
    
}

void loop() {
 // put your main code here, to run repeatedly: 
  if(myAddress!=UCA0RXBUF)
      myAddress=UCA0RXBUF;
  else
    {
     if(UCA0RXBUF>='0' && UCA0RXBUF<='9' )
      {
        myAddress=UCA0RXBUF;
        P1OUT |= BIT0;    
      }
      else if((UCA0RXBUF>='A' && UCA0RXBUF<='Z') ||(UCA0RXBUF>='a' && UCA0RXBUF<='z') )
      {
        myAddress=UCA0RXBUF;
        P1OUT &= ~BIT0;
        P1OUT |= BIT6;  
        delay(50);
        P1OUT |= BIT0;
        P1OUT &= ~BIT6;
        delay(50);  
      }
      else 
      {
        myAddress=UCA0RXBUF;
        P1OUT &= 0x00;  
      } 
   }
       
  

}
