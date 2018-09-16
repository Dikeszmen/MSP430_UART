#include "init.h"
#include "../header/reading.h"

/**
Reading from the serial port. To check the incoming packet, use the Motorola protocol
*/

uint8_t readingFromSerial(void)
{
    extern List *first;
    static volatile char cmd;
    int dlen=0;
    int i;
    packetState State=EmptyState;
    int calculateCrc,packetCrc;
    int retval=0;
    List *act,*temp;
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
                act=act->next;
                continue;
            }
            break;

        case moto55:
            if (act->data == 0x55)
            {
                i++;
                if(i==MAXU)
                    break;

                continue;
            }
            if (act->data== FF)
            {
                State=moto1;
                act=act->next;
                continue;
            }
            else
                break;

        case moto1:
            if(act->data==1)
            {
                calculateCrc=0;
                State= address;
                act=act->next;
                continue;
            }
            else
                break;
        case address:

            if(SLAVE==act->data)
            {
                calculateCrc = addCRC(calculateCrc, act->data);
                State = command;
                act=act->next;
                continue;

            }
            else
                break;

        case command :
            calculateCrc = addCRC(calculateCrc,act->data);
            cmd = act->data;
            State = DLenLow;
            act=act->next;
            continue;

        case DLenLow :
            calculateCrc = addCRC(calculateCrc, act->data);
            dlen = (act->data & 0xff);
            State = DLenHigh;
            act=act->next;
            continue;

        case DLenHigh :
            calculateCrc = addCRC(calculateCrc, act->data);
            dlen |= (act->data& 0xff) << BYTE ;
            if (dlen != ZERO)
                break;
            else
            {
                State=Data;
                act=act->next;
                continue;
            }

        case Data :
            calculateCrc = addCRC(calculateCrc, act->data);
            State = CrcLow;
            act=act->next;
            continue;

        case CrcLow :
            packetCrc = (act->data & 0xff);
            State = CrcHigh;
            act=act->next;
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
            act=act->next;
            break;

        default:
            act=act->next;
            break;
        }
        break;
    }
    while(first!=act)
    {
        temp=first;
        first=first->next;
        free(temp);
    }

    return retval;
}

