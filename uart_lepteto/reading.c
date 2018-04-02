#include <stdio.h>
#include <stdlib.h>
#include "reading.h"
#include "init_uart.h"
#include "crc.h"
#include <msp430g2553.h>
#define PING 0x69
#define TERM 0x01
#define LIMIT 2
#define TRUE 1
#define FALSE 0


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
    char P1OUT;
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

                    continue;
                case moto55:
                    if (act->data== 0x55)
                        {

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
                            State=moto1;
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



