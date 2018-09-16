#ifndef READING_H_INCLUDED
#define READING_H_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../header/crc.h"
#include "../header/reading.h"
#define FF 0xFF
#define ONE 1
#define ZERO 0
#define MAXU 5
#define PING 0x69
#define TERMCMD 0x01
#define SLAVE 1

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


typedef struct FIFO
{
    char data;
    struct FIFO *next;

} List;

uint8_t readingFromSerial(void);

#endif // READING_H_INCLUDED
