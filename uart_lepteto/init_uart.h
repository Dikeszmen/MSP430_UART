#ifndef INIT_UART_H_INCLUDED
#define INIT_UART_H_INCLUDED

typedef struct FIFO
{
  char data;
  struct FIFO *next;
}LIST;

void serialInit(LIST *first, LIST *actual, LIST *newpiece);

#endif

