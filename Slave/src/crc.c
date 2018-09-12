/*!

Source from Zidarics Zoltan a.k.a Zamek
\version $Id: crc.c v 1.4 2009-05-08 06:16:09 zamek Exp $

*/
#include "../header/reading.h"
#include "../header/crc.h"


int addCRC(int packetCrc, unsigned char countedCrc)
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

int compareCRC(int crc1, int crc2)
{
    return crc1==crc2 ? 1:0;
}

