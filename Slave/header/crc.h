#ifndef CRC_H_INCLUDED
#define CRC_H_INCLUDED

#define POLYNOMIAL 0xd8
#define BYTE 8
#define WIDTH (BYTE * sizeof(int))
#define TOPBIT (1 << (WIDTH-1))


uint16_t addCRC(uint16_t packetCrc, unsigned char countedCrc);

uint8_t compareCRC(uint16_t crc1, uint16_t crc2);



#endif // CRC_H_INCLUDED
