#include <stdio.h>
#include <msp430g2553.h>
#define POLYNOMIAL 0xd8
#define BYTE 8
#define WIDTH (BYTE * sizeof(int))
#define TOPBIT (1 << (WIDTH-1))
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




