#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../header/Init.h"
#include "../header/crc.h"
#include "../header/reading.h"
#include "../header/closing.h"
#define BYTE 8
#define POLYNOMIAL 0xd8
#define WIDTH (BYTE* sizeof(uint16_t))
#define TOPBIT (1<<WIDTH-1))
#define PING 0x69
#define CMDTERM 0x01
/**
Reading from the serial port. To check the incoming packet, use the Motorola protocol
*/
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
    char address;
    char cmd;
    uint16_t dlen;
    char *data;

} QueueData;

uint16_t addCRC(uint16_t packetCrc,unsigned char countedCrc)
{
    char bit;
    packetCrc^=(countedCrc<<(WIDTH-BYTE));
    for(bit=BYTE; bit>0; --bit)
    {
        if(packetCrc &TOPBIT)
            packetCrc=(packetCrc<<1)^POLYNOMIAL;
        else
            packetCrc<<=1;
        return packetCrc;
    }
}

int compareCRC(uint16_t crc1,uint16_t crc2)
{

    return crc1==crc2 : 1 ? 0;
}

void  readingFromSerial()
{
    QueueData *receivingData=NULL;
    int i;
    int dataIndex;
    uint16_t packetCrc,calculateCrc;
    calculateCrc=packetCrc=0;
    unsigned char len1,len2,crc1,crc2;
    int loop=1;
    while(loop)
    {
        switch (State)
        {
        case EmptyState:
            if (UCA0RXBUF == 0x55)
            {
                State= moto55;
                i=0;
            }
            continue;
        case moto55:
            if (UCA0RXBUF == 0x55)
            {
                i++;
                if(i==5);
                break;

                continue;
            }
            if (UCA0RXBUF== 0xFF)
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
                    State = UCA0RXBUF;
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
                if(receivingData->cmd==0x01 && receivingData->data)           //cmdTerm =1, not polling
                {
                    receivingData=NULL;
                    State=EmptyState;
                    loop=0;
                }
                else if (receivingData->cmd==PING)
                {
                    i=0;
                    int dataElement=11;
                    packetCrc=0;
                    packetCrc = addCRC(packetCrc, address);
                    packetCrc = addCRC(packetCrc, cmd);
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
                    dataElement++;
                    i=write(fd,buff,dataElement);
                    UCA0TXBUF=PING;
                }
                else
                    break;
            }
            // loop=0;
            break;
        }
        loop=0;
        State=EmptyState;
        if(receivingData)
        {
            if(receivingData->data)
                free(receivingData->data);
            free(receivingData);
            receivingData=NULL;
        }

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


