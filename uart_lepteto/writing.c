#include <msp430g2553.h>
#include <stdio.h>
#include <stdlib.h>
#include "crc.h"
void sending(unsigned char address, unsigned char cmd,char *data, int  dLen)
{
    if ( !data || dLen <=0 )
            {
              P1OUT |= BIT0 + BIT6;
              return;
              }
            
    char *buff=(char*)malloc((dLen+13)*sizeof(char));
    if(!buff)
    {
      P1OUT |= BIT0 + BIT6;
      delay(10);
      P1OUT &=0x00;
      delay(10);
      P1OUT |= BIT0 + BIT6;
      delay(10);
      return;
    }
    
    int i=0;
    int dataElement=11;
    unsigned int crc=0;
    unsigned char len1,len2,crc1,crc2;
    crc = addCRC(crc, address);
    crc = addCRC(crc, cmd);
    len1= dLen & 0xff;
    crc = addCRC(crc,len1);
    len2 = (dLen >> BYTE) & 0xff;
    crc = addCRC(crc, len2);
    if(dLen>0)
        {
            int j;
            for (j=0; j<dLen; j++,data++)
                {
                    buff[dataElement]=*data;
                    crc = addCRC(crc, *data);
                    dataElement++;
                }
        }
    crc1=crc & 0xff;
    crc2=(crc>>BYTE) & 0xff;
    while(i!=5)
        {
            buff[i]=0x55;
            i++;
        }
    buff[5]=0xFF;
    buff[6]=0x01;
    buff[7]=address;
    buff[8]=cmd;
    buff[9]=len1;
    buff[10]=len2;
    buff[dataElement]=crc1;
    dataElement++;
    buff[dataElement]=crc2;
    //dataElement++;??
    i=0;
    while(i<dataElement)
    {
      P1OUT |=BIT0;
      UCA0TXBUF=buff[i];
      i++;
      P1OUT &= ~BIT0;
    }
    free(buff);
}


