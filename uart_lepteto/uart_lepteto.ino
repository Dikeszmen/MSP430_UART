#include <msp430g2553.h>
#include <stdint.h>
#define TX BIT2
#define RX BIT1
#define TXLED BIT0
#define RXLED BIT6
#define POLYNOMIAL 0xd8
#define BYTE 8
#define WIDTH (BYTE * sizeof(uint16_t))
#define TOPBIT (1 << (WIDTH-1))
#define LIMIT 128


char myAddress[4];
volatile char *paddress=NULL;

typedef struct FIFO
{
  char data;
  struct FIFO *next;
} List;



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
  P1OUT |= BIT0;
  //__bic_SR_register_on_exit (LPM3_bits);
  
  P1OUT &= ~BIT0;
  
}

void reading(List *actual)
{
    /*QueueData *receivingData=NULL,
               *toQueueuPacket=NULL;*/
    unsigned char data;
    int i=0;
    int dataIndex;
    Crc packetCrc,calculateCrc;
    calculateCrc=packetCrc=0;
    packetState State=EmptyState;

    while(actual->next)
        {
            switch (State)
                {
                case EmptyState:
                    if (data == 0x55)
                        {
                            State= moto55;
                            i=0;
                        }
                    continue;
                case moto55:
                    if (data == 0x55)
                        {
                            i++;
                            if(i==5)
                                break;
                                
                            continue;
                        }
                    if (data== FF)
                        {
                            State=moto1;
                            continue;
                        }
                    else
                        break;

                case moto1:
                    if(data==1)
                        {
                            calculateCrc=0;
                            State= address;
                            continue;
                        }
                    else
                        break;
                case address:
                    if (!receivingData)
                        {
                            calculateCrc = addCRC(calculateCrc, data);
                            receivingData=reserve(data);
                            if (!receivingData)
                                    break;
                            State = command;
                            continue;
                        }
                    else
                        break;

                case command :
                    calculateCrc = addCRC(calculateCrc,data);
                    receivingData->cmd = data;
                    State = DLenLow;
                    continue;
                case DLenLow :
                    calculateCrc = addCRC(calculateCrc, data);
                    receivingData->dlen = (data & 0xff);
                    State = DLenHigh;
                    continue;
                case DLenHigh :
                    calculateCrc = addCRC(calculateCrc, data);
                    receivingData->dlen |= (data& 0xff) << BYTE ;
                    dataIndex=0;
                    if (receivingData->dlen > 0)
                        {
                            if (receivingData->dlen <= LIMIT)
                                {
                                    receivingData->data =(char*)malloc((receivingData->dlen)*sizeof(char));
                                    if(!receivingData->data)
                                            break;
                                    State = Data;
                                    continue;
                                }
                            else
                                    break;
                        }
                    else
                        {
                            State =  CrcLow;
                            continue;
                        }
                case Data :
                    calculateCrc = addCRC(calculateCrc, data);
                    *((receivingData->data)+dataIndex) = data;
                    if(++dataIndex>=receivingData->dlen)
                        State = CrcLow;
                    else
                        State = Data;
                    continue;
                case CrcLow :
                    packetCrc = (data & 0xff);
                    State = CrcHigh;
                    continue;
                case CrcHigh:
                    packetCrc |= ( data & 0xff)<< BYTE;
                    if (compareCRC(packetCrc, calculateCrc))
                        {
                            if(receivingData->cmd==1 && receivingData->data)           //cmdTerm =1, not polling
                                {
                                    toQueueuPacket=receivingData;
                                    receivingData=NULL;
                                }
                            else if (receivingData->cmd==PING)
                                {

                                }
                            else
                                syslog(LOG_ERR,"ERROR Packet");
                        }
                    break;
                }
            State=EmptyState;
            if(receivingData)
                {
                    if(receivingData->data)
                        free(receivingData->data);
                    free(receivingData);
                    receivingData=NULL;
                }
        }
    if(receivingData)
        {
            if(receivingData->data)
                free(receivingData->data);
            free(receivingData);
            receivingData=NULL;
        }
}

QueueData *reserve(char data)
{
    QueueData *temp;
    temp=(QueueData *)malloc(sizeof(QueueData));
    if (!temp)
        return NULL;
    temp->address=data;
    temp->cmd=0;
    temp->dlen = 0;
    temp->data = NULL;
    return temp;
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
    List *elso=(List*)malloc(sizeof(List));
    if(!elso)
      {

        
      }
    else
        elso->next=NULL;
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

void addToList(List *actual,char data)
{
 List *uj=NULL;
 uj=(List*)malloc(sizeof(List));
 if(!uj)
  //vmi
  else
  {
    actual->next=uj;
    uj->data=data;
    uj->next=NULL;    
  }
 
}

