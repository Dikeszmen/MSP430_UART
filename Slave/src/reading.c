#include <stdio.h>
#include <stdlib.h>
#include "init.h"
#include "../header/crc.h"
#include "../header/reading.h"


/**
Reading from the serial port. To check the incoming packet, use the Motorola protocol
*/
const unsigned char slaveAddress=1;
const unsigned char termCommand=TERMCMD;
const unsigned char pingCommand=PING;

int readingFromSerial(void)
{
    extern List *first;
    static volatile char cmd;
    int dlen=0;
    int i;
    packetState State=EmptyState;
    int calculateCrc,packetCrc;
    int retval=0;
    List /* *temp,*/*act;
    act=NULL;

    while(first)
    {
        act=first;
        switch (State)
        {
        case EmptyState:
            if (act->data== 0x55)
            {
                State= moto55;
                i=0;
                first=act->next;
                free(act);
            }
            continue;
        case moto55:
            if (act->data == 0x55)
            {
                i++;
                if(i==5)
                    break;

                continue;
            }
            if (act->data== FF)
            {
                State=moto1;
                first=act->next;
                free(act);
                continue;
            }
            else
                break;

        case moto1:
            if(act->data==1)
            {
                calculateCrc=0;
                State= address;
                first=act->next;
                free(act);
                continue;
            }
            else
                break;
        case address:

            if(slaveAddress==act->data)
            {
                calculateCrc = addCRC(calculateCrc, act->data);
                State = command;
                first=act->next;
                free(act);
                continue;

            }
            else
                break;

        case command :
            calculateCrc = addCRC(calculateCrc,act->data);
            cmd = act->data;
            State = DLenLow;
            first=act->next;
            free(act);
            continue;

        case DLenLow :
            calculateCrc = addCRC(calculateCrc, act->data);
            dlen = (act->data & 0xff);
            State = DLenHigh;
            first=act->next;
            free(act);
            continue;

        case DLenHigh :
            calculateCrc = addCRC(calculateCrc, act->data);
            dlen |= (act->data& 0xff) << BYTE ;
            if (dlen == 0)
            {
                State=Data;
                first=act->next;
                free(act);
                continue;
            }
            else
                break;

        case Data :
            calculateCrc = addCRC(calculateCrc, act->data);
            State = CrcLow;
            first=act->next;
            free(act);
            continue;

        case CrcLow :
            packetCrc = (act->data & 0xff);
            State = CrcHigh;
            first=act->next;
            free(act);
            continue;

        case CrcHigh:
            packetCrc |= ( act->data & 0xff)<< BYTE;
            if (compareCRC(packetCrc, calculateCrc))
            {
                if(cmd==TERMCMD)           //cmdTerm =1, not polling
                    retval=TERMCMD;

                else if (cmd==PING)
                    retval=PING;
            }
            first=act->next;
            free(act);
            break;

        default:
            first=act->next;
            free(act);
            break;
        }
        break;
    }
    return retval;
}

