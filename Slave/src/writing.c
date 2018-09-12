#include <stdlib.h>
#include "../header/reading.h"
#include "../header/crc.h"
#include "../header/init.h"
#include "/home/herczig/Embedded/MSP/msp430-gcc/include/msp430g2553.h"

static volatile char *txbuffer;
static volatile int i=0;
void sendPacket(unsigned char address, unsigned char cmd,char *data, int dLen)
{
    if ( !data || dLen <0 )
            return;

    char *buff=(char*)malloc((dLen+13)*sizeof(char));
    if(!buff)
            return;


    int dataElement=7;
    int crc=0;
    unsigned char len1,len2,crc1,crc2;
    crc = addCRC(crc, address);
    crc = addCRC(crc, cmd);
    len1= dLen & 0xff;
    crc = addCRC(crc,len1);
    len2 = (dLen >> BYTE) & 0xff;
    crc = addCRC(crc, len2);
    if(dLen>=0)
        {
            int j=0;
            do
            {
                 buff[dataElement]=*data;
                    crc = addCRC(crc, *data);
                    dataElement++;
            }
            while(j<dLen)
            ;
        }

    crc1=crc & 0xff;
    crc2=(crc>>BYTE) & 0xff;

    buff[0]=0x55;
    buff[1]=0xFF;
    buff[2]=0x01;
    buff[3]=address;
    buff[4]=cmd;
    buff[5]=len1;
    buff[6]=len2;
    buff[dataElement]=crc1;
    dataElement++;
    buff[dataElement]=crc2;
    dataElement++;
    txbuffer=buff;
    UC0IE |= UCA0TXIE;
    UCA0TXBUF=txbuffer[i];
    free(buff);

}
/*#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)*/
void __attribute__((interrupt(USCIAB0TX_VECTOR))) USCI0TX_ISR(void)
{

    P1OUT |= TXLED;
    UCA0TXBUF = txbuffer[i++]; // TX next character
    if (i == sizeof(txbuffer)-1) // TX over?
        UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt
    P1OUT &= ~TXLED;

}
