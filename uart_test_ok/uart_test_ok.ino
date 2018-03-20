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
    WDTCTL=WDTPW + WDTHOLD;   //Watchdog STOP
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
    
    //Revolt Kft. Bank: 10700457-43660009-51100005, Közlemény rovatba: Rendelésszám
}

void loop() {
 // put your main code here, to run repeatedly: 
  if(UCA0RXBUF==0x55)
    P1OUT |= BIT0;    
  else 
    P1OUT &= 0x00;  


      
}
/*#pragma vector=USCI0RX_VECTOR 
__interrupt void USCI0RX_ISR(void)
{
 P1OUT |= BIT0; 
   /* if (UCA0RXBUF == 0x55) // 0x55 received?
    { 
       i = 0; 
     //  UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt 
     //P1OUT 
     
       Serial.println(UCA0RXBUF);
      //UCA0TXBUF = myAddress; 
    } 
    //P1OUT &= ~RXLED;  
}
#pragma vector=USCI0TX_VECTOR 
__interrupt void USCI0TX_ISR(void)
{
   P1OUT |= TXLED; 
     UCA0TXBUF = 0xff; // TX next character 
    //if (i == sizeof string - 1) // TX over? 
       UC0IE &= ~UCA0TXIE; // Disable USCI_A0 TX interrupt 
    P1OUT &= ~TXLED;
  
}*/
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

/*
#pragma vector=USCIAB0RX_VECTOR 
__interrupt void USCI0RX_ISR(void) 
{ 
   
{
    P1OUT |= RXLED;
    if (UCA0RXBUF == 0x55)
    {
        QueueData *receivingData=NULL;
        int i=1;
        int dataIndex;
        uint16_t packetCrc,calculateCrc;
        calculateCrc=packetCrc=0;
        packetState State=EmptyState;
        unsigned char len1,len2,crc1,crc2;
        int loop=1;
        while(loop)
        {
            switch (State)
            {
            case EmptyState:
                if (UCA0RXBUF == 0x55)
                {
                    State=moto55;
                    continue;
                }
                else if (UCA0RXBUF == 0xFF)
                        {
                            State=moto1;
                            continue;
                        }
                else
                    break;

            case moto55:
                if (UCA0RXBUF == 0x55)
                    {
                        i++;
                        if(i==5);
                            break;
                        continue;
                    }
                else if(UCA0RXBUF == 0xFF)
                        {
                            State=moto1;
                            continue;
                        }
                else
                    break;

            case moto1:
                if(UCA0RXBUF==0x01)
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

                    calculateCrc = addCRC(calculateCrc, UCA0RXBUF);
                    receivingData=reserve(UCA0RXBUF);
                    if (!receivingData)
                        break;

                    State = command;
                    continue;
                }
                else
                    break;

            case command :
                calculateCrc = addCRC(calculateCrc,UCA0RXBUF);
                receivingData->cmd = UCA0RXBUF;
                State = DLenLow;
                continue;
            case DLenLow :
                calculateCrc = addCRC(calculateCrc, UCA0RXBUF);
                receivingData->dlen = (UCA0RXBUF & 0xff);
                State = DLenHigh;
                continue;
            case DLenHigh :
                calculateCrc = addCRC(calculateCrc, UCA0RXBUF);
                receivingData->dlen |= (UCA0RXBUF & 0xff) << BYTE ;
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
                    //break ahead of full if-function
                    State =  CrcLow;
                    continue;
                }
            case Data :
                calculateCrc = addCRC(calculateCrc, UCA0RXBUF);
                *((receivingData->data)+dataIndex) = UCA0RXBUF;
                if(++dataIndex>=receivingData->dlen)
                    State = CrcLow;
                else
                    State = UCA0RXBUF;
                continue;
            case CrcLow :
                packetCrc = (UCA0RXBUF & 0xff);
                State = CrcHigh;
                continue;
            case CrcHigh:
                packetCrc |= ( UCA0RXBUF & 0xff)<< BYTE;
                if (compareCRC(packetCrc, calculateCrc))
                {
                    if(receivingData->cmd==0x01 && receivingData->data && receivingData->address==myAddress)           //cmdTerm =1, not polling
                    {
                        free(receivingData);
                        UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
                        UCA0TXBUF = string[i++];
                        State=EmptyState;
                        loop=0;
                    }
                    else if (receivingData->cmd==0x69 && receivingData->address=='A')
                    {
                        i = 0;
                        UC0IE |= UCA0TXIE; // Enable USCI_A0 TX interrupt
                        UCA0TXBUF = string[i++];
                    }
                    P1OUT &= ~RXLED;
                }
            }
        }



    }




}*/
