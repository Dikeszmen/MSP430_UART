#ifndef CRC_UART_H_INCLUDED
#define CRC_H_INCLUDED

#define POLYNOMIAL 0xd8
#define BYTE 8
#define WIDTH (BYTE * sizeof(int))
#define TOPBIT (1 << (WIDTH-1))

int addCRC(int packetCrc, unsigned char countedCrc);
int compareCRC(int crc1, int crc2);

#endif
