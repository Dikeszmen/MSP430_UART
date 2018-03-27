#include <msp430g2553.h>
#include <stdint.h>
#define MAXELEMENT 16
#define TX BIT2
#define RX BIT1
#define TXLED BIT0
#define RXLED BIT6
#define PING 0x69
#define TERM 0x01
#define POLYNOMIAL 0xd8
#define BYTE 8
#define WIDTH (BYTE * sizeof(uint16_t))
#define TOPBIT (1 << (WIDTH-1))
#define LIMIT 64
#define TRUE 1
#define FALSE 0


char myAddress='A';

char incomingdata[MAXELEMENT];
int i=0,LOOP=TRUE;

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

void setup() {
  // put your setup code here, to run once:

   
   serialInit();
    
}

void loop() {


    __bis_SR_register(LPM3_bits + GIE);//LPM3;
  
      
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
  P1OUT |= BIT6;
  //__bic_SR_register_on_exit (LPM3_bits);
  incomingdata[i]=UCA0RXBUF;
  incomingdata[i++]='\0';
  P1OUT &= ~BIT6;
  
}

int reading(char *pdata)
{
    /*QueueData *receivingData=NULL,
               *toQueueuPacket=NULL;*/
    if(!pdata)
      return;
    
    char cmd=0;
    char *reqData=NULL;
    unsigned int i=0;
    unsigned int dataIndex;
    unsigned int len=0,crc;
    unsigned int dataIndex;
    calculateCrc=crc=0;
    packetState State=EmptyState;

    while(LOOP)
        {
            switch (State)
                {
                case EmptyState:
                    if (pdata[i]== 0x55)
                        {
                            State= moto55;
                            i++;
                        }
                    continue;
                case moto55:
                    if (pdata[i]== 0x55)
                        {
                          
                            if(i==5)
                                break;
                            i++;
                            continue;
                        }
                    if (pdata[i]== FF)
                        {
                            State=moto1;
                            i++;
                            continue;
                        }
                    else
                        break;

                case moto1:
                    if(pdata[i]==1)
                        {
                            calculateCrc=0;
                            State= address;
                            i++;
                            continue;
                        }
                    else
                        break;
                case address:
                    if(pdata[i]=='A')
                    {
                      calculateCrc = addCRC(calculateCrc, pdata[i]);
                      State = command;
                      i++;
                      continue;
                    }
                    else
                    {
                      memset(pdata,'\0',MAXELEMENT);
                      State=EmptyState;
                      LOOP=FALSE;
                      break;
                    }
                        
                case command :
                    calculateCrc = addCRC(calculateCrc,pdata[i]);
                    cmd=pdata[i];
                    State = DLenLow;
                    i++;
                    continue;
                case DLenLow :
                    calculateCrc = addCRC(calculateCrc, pdata[i]);
                    *plen=pdata[i] & 0xFF;
                    State = DLenHigh;
                    i++;
                    continue;
                case DLenHigh :
                    calculateCrc = addCRC(calculateCrc, pdata[i]);
                    *plen |= (pdata[i]& 0xff) << BYTE ;
                    i++;
                    dataIndex=0;
                    if (*plen> 0)
                        {
                            if (*plen <= LIMIT)
                                {
                                    reqData =(char*)malloc((*plen)*sizeof(char));
                                    if(!reqData)
                                            break;
                                    State = Data;
                                    i++;
                                    continue;
                                }
                            else
                                    break;
                        }
                    else
                        {
                            State =  CrcLow;
                            i++;
                            continue;
                        }
                case Data :
                    calculateCrc = addCRC(calculateCrc, pdata[i]);
                    *((reqData)+dataIndex) = data;
                    if(++dataIndex>=*plen)
                      {
                        State = CrcLow;
                        i++;
                      }
                    else
                      {
                        State = Data
                        i++;
                      }
                      continue;
                case CrcLow :
                    crc = (pdata[i] & 0xff);
                    State = CrcHigh;
                    i++;
                    continue;
                case CrcHigh:
                    crc |= ( pdata[i] & 0xff)<< BYTE;
                    if (compareCRC(crc, calculateCrc))
                        {
                            if(cmd==TERM && *reqData)           //cmdTerm =1, not polling
                                {
                                   free(reqData);
                                    return 1;
                                }
                            else if (cmd==PING)
                               return 2;
                           
                        }
                    if(reqData)
                      free(reqData);
                    LOOP=FALSE;
                    break;
                }
            
            if(reqData)
                free(reqData);
        }
}
void sending(unsigned char address, unsigned char cmd,char *data, uint16_t dLen)
{
    if ( !data || fd <0 || dLen <0 )
            {
              P1OUT |= BIT0 + BIT6;
              return;
              }
            
    char *buff=(char*)malloc((dLen+13)*sizeof(char));
    if(!buff)
    {
      P1OUT |= BIT0 + BIT6;
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
void serialInit()
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

    __bis_SR_register(LPM3_bits + GIE);
       
}
uint16_t addCRC(uint16_t packetCrc, unsigned char countedCrc)
{
    char bit;
    packetCrc^= ( countedCrc<< (WIDTH-BYTE));
    for (bit=BYTE; bit >0; --bit)
        if (packetCrc& TOPBIT)
            packetCrc= (packetCrc<< 1) ^ POLYNOMIAL;
        else
            packetCrc<<= 1;
    return packetCrc;
}

int compareCRC(uint16_t crc1, uint16_t crc2)
{
    return crc1==crc2 ? 1:0;
}


