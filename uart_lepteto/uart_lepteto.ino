#include <msp430g2553.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*#include "ADC.h"
#include "crc.h"
#include "init_uart.h"
#include "reading.h"
#include "writing.h"*/
#define POLYNOMIAL 0xd8
#define BYTE 8
#define WIDTH (BYTE * sizeof(int))
#define TOPBIT (1 << (WIDTH-1))
#define TX BIT2
#define RX BIT1
#define TXLED BIT0
#define RXLED BIT6
#define PING 0x69
#define TERM 0x01
#define LIMIT 2
#define TRUE 1
#define FALSE 0


typedef struct FIFO
{
  char data;
  struct FIFO *next;
}LIST;

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

const char myAddress=1;
unsigned char cmd=0x69;
char sendingData=1;
LIST *first,*actual,*newpiece;

int reading(LIST *act,unsigned char myAddress);
int addCRC(int packetCrc, unsigned char countedCrc);
int compareCRC(int crc1, int crc2);
void sending(unsigned char address, unsigned char cmd,char *data, int dLen);

void serialInit(LIST *first, LIST *actual, LIST *newpiece);


void setup() {
  // put your setup code here, to run once:

   serialInit(first,actual,newpiece);
   // __bis_SR_register(LPM3_bits + GIE);//LPM3;
}

void loop() 
{
      //if(reading(first)==1)
        //ADC
        if(reading(first,myAddress))
            sending(myAddress, cmd,&sendingData, strlen(&sendingData));
        //sleep
}


#pragma vector=USCIAB0RX_VECTOR 
__interrupt void USCI0RX_ISR(void)
{
  //__bic_SR_register_on_exit (LPM3_bits);
  P1OUT |= BIT6;
  LIST *newlist=(LIST*)malloc(sizeof(LIST));
  newlist->data=UCA0RXBUF;
  newlist->next=NULL;
  if(first)
      actual->next=newlist;
  else
      first=newlist;
  actual=newlist;
  P1OUT &= ~BIT6;
}

void serialInit(LIST *first, LIST *actual, LIST *newpiece)
{
    
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
    UCA0BR0 = 0x6D;   //9600-> 109|d=0x6D BAUD 
    UCA0BR1 = 0x00;
    UCA0MCTL=UCBRS0+UCA0BR1 ;
    UCA0CTL1 &= ~UCSWRST;
    IE2 |= UCA0RXIE;    //UC0IE
    first=actual=newpiece=NULL;
    //__bis_SR_register(LPM3_bits + GIE);
       
}


int reading(LIST *act,unsigned char myAddress){
    if(!act)
      return -1;

    LIST *temp;
    int i=0;
    char cmd=0;
    char *reqData=NULL;
    unsigned int dataIndex;
    unsigned int len=0,crc,calculateCrc;
    calculateCrc=crc=0;
    int LOOP=TRUE;
    packetState State=EmptyState;
  //  char P1OUT;
    while(LOOP)
        {
            switch (State)
                {
                case EmptyState:
                    if (act->data== 0x55)
                        {
                         
                            State= moto55;
                            i++;
                        }
                       
                    temp=act;
                    act=temp->next;
                    free(temp);
                    if(act->data==0xff)
                          {P1OUT &=0;
                            P1OUT |=BIT6+BIT0;}
                            else
                            P1OUT |=BIT6;

                    continue;
                case moto55:
                    if (act->data== 0x55)
                        {
                             P1OUT &=0;
                            P1OUT |=BIT6+BIT0;
                            if(i==5)
                            {
                              
                                temp=act;
                                act=temp->next;
                                free(temp);
                                break;
                            }
                            temp=act;
                            act=temp->next;
                            free(temp);
                            continue;
                        }
                    if (act->data== 0xFF)
                        {
                           P1OUT |=BIT6;
                            State=moto55;
                            temp=act;
                            act=temp->next;
                            free(temp);
                            continue;
                        }
                    else
                        {
                         
                            temp=act;
                            act=temp->next;
                            free(temp);
                            break;
                        }

                case moto1:
                    if(act->data==0x01)
                        {
                           
                            calculateCrc=0;
                            State= address;
                            temp=act;
                            act=temp->next;
                            free(temp);
                            continue;
                        }
                    else
                    {
                        temp=act;
                        act=temp->next;
                        free(temp);
                        break;
                    }
                case address:
                    if(act->data==myAddress)
                    {
                      calculateCrc = addCRC(calculateCrc, act->data);
                      State = command;
                      temp=act;
                      act=temp->next;
                      free(temp);
                      continue;
                    }
                    else
                    {
                      State=EmptyState;
                      LOOP=FALSE;
                      while(act->next)
                      {
                      temp=act;
                      act=temp->next;
                      free(temp);
                      }
                      break;
                    }

                case command :
                    calculateCrc = addCRC(calculateCrc,act->data);
                    cmd=act->data;
                    State = DLenLow;
                    temp=act;
                    act=temp->next;
                    free(temp);
                    continue;

                case DLenLow :
                    calculateCrc = addCRC(calculateCrc, act->data);
                    len=act->data & 0xFF;
                    State = DLenHigh;
                    temp=act;
                    act=temp->next;
                    free(temp);
                    continue;

                case DLenHigh :
                    calculateCrc = addCRC(calculateCrc, act->data);
                    len |= (act->data& 0xff) << BYTE ;
                    /*temp=act;
                    act=temp->next;
                    free(temp);*/
                    dataIndex=0;
                    if (len> 0)
                        {
                            if (len <= LIMIT)
                                {
                                    reqData =(char*)malloc((len)*sizeof(char));
                                    if(!reqData)
                                      {
                                        while(act->next)
                                       {
                                          P1OUT &= 0x00;
                                          temp=act;
                                          act=temp->next;
                                          free(temp);
                                          P1OUT = BIT0+BIT6;
                                       }
                                            break;
                                      }
                                    State = Data;
                                    temp=act;
                                    act=temp->next;
                                    free(temp);
                                    continue;
                                }
                            else
                            {
                              LOOP=FALSE;
                              while(act->next)
                                {
                                  temp=act;
                                  act=temp->next;
                                  free(temp);
                                }
                                break;
                            }

                        }
                    else
                        {
                            State =  CrcLow;
                            temp=act;
                            act=temp->next;
                            free(temp);
                            continue;
                        }
                case Data :
                    calculateCrc = addCRC(calculateCrc, act->data);
                    *((reqData)+dataIndex) = act->data;
                    if(++dataIndex>=len)
                        State = CrcLow;
                    else
                        State = Data;
                     temp=act;
                     act=temp->next;
                     free(temp);
                     continue;

                case CrcLow :
                    crc = (act->data & 0xff);
                    State = CrcHigh;
                    temp=act;
                    act=temp->next;
                    free(temp);
                    continue;

                case CrcHigh:
                    crc |= ( act->data & 0xff)<< BYTE;
                    if (compareCRC(crc, calculateCrc))
                        {
                            if(cmd==TERM && *reqData)           //cmdTerm =1, not polling
                                {
                                  while(act->next)
                                    {
                                      temp=act;
                                      act=temp->next;
                                      free(temp);
                                    }
                                   free(reqData);
                                    return 1;
                                }
                            else if (cmd==PING)
                               {
                                while(act->next)
                                {
                                  temp=act;
                                  act=temp->next;
                                  free(temp);
                                }
                                return 2;
                               }

                        }
                    if(reqData)
                      {
                        free(reqData);
                        LOOP=FALSE;
                        break;
                      }

                }

        }
        return 0;
}


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

int addCRC(int packetCrc, unsigned char countedCrc)
{
    char bits;
    packetCrc^= ( countedCrc<< (WIDTH-BYTE));
    for (bits=BYTE; bits >0; --bits)
        if (packetCrc& TOPBIT)
            packetCrc= (packetCrc<< 1) ^ POLYNOMIAL;
        else
            packetCrc<<= 1;
    return packetCrc;
}

int compareCRC(int crc1, int crc2)
{
    return crc1==crc2 ? 1:0;
}



