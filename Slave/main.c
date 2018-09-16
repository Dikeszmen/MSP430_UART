#include "/home/herczig/Embedded/MSP/msp430-gcc/include/msp430g2553.h"
#include "header/ADC.h"
#include "header/crc.h"
#include "header/init.h"
#include "header/writing.h"
#include "header/reading.h"
#include <stdlib.h>

List *first,*act;

int main(void)
{

    char adResult[2];
    first=NULL;
    init();
    while(TRUE)
    {
        if(readingFromSerial()==TERMCMD)
        {
            ADCWorking(adResult);
            sendPacket(SLAVE,TERMCMD,adResult,sizeof(adResult));
        }
        if(readingFromSerial()==PING)
            sendPacket(SLAVE,PING,0,0);
    }
    return 0;
}


//#pragma vector=USCIAB0RX_VECTOR
void __attribute__((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR(void)
{
// __bic_SR_register_on_exit (LPM3_bits);
    //_BIC_SR(LPM3_EXIT);
    P1OUT |=BIT0;
    List *newlist=(List*)malloc(sizeof(List));
    newlist->data=UCA0RXBUF;
    newlist->next=NULL;
    if(first)
        act->next=newlist;
    else
        first=newlist;
    act=newlist;
    P1OUT &= ~BIT0;
}

//__interrupt void USCI0RX_ISR(void)
//__attribute__((interrupt(USCIAB0RX_VECTOR))) void USCI0RX_ISR(void)




