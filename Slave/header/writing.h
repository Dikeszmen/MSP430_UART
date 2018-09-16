#ifndef WRITING_H_INCLUDED
#define WRITING_H_INCLUDED
#include <stdint.h>

void sendPacket(unsigned char address, unsigned char cmd,char *data, uint8_t dLen);

#endif // WRITING_H_INCLUDED
