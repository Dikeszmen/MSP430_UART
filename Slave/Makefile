
MCU = -mmcu=msp430g2553
INCLUDE_DIR = $$MSP_HOME/msp430-gcc/include

IFLAGS = -I$(INCLUDE_DIR)
GCC_DIR= /home/herczig/Embedded/MSP/msp430-gcc/bin/
LFLAGS = -L /usr/msp430/lib/ldscripts/msp430g2553 -T msp430g2553.ld
CC = $(GCC_DIR)msp430-elf-gcc
CFLAGS = -Wall -g -Os $(MCU) -Iheader

SRC:=src/%.c
SRC_ALL=$(wildcard src/*.c)
OBJ:=obj/%.o
OBJDIR=obj/
ALL_OBJ:=$(patsubst src/%.c,obj/%.o,$(SRC_ALL) main.c)
RESULT=app

$(RESULT): $(ALL_OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^ 
#$(MCU) $? volt a felso sorba

$(OBJDIR)main.o:main.c
	$(CC) -c -o $@ $< $(CFLAGS) $(LFLAGS)

$(OBJ):$(SRC)
	$(CC) -c -o $@ $< $(CFLAGS) $(LFLAGS)
	
.PHONY: clean
	
clean:
	rm -fr  obj/*.o  $(RESULT)

install:
	mspdebug rf2500 "prog blink run"

#GCC_DIR = ../../../bin

#SUPPORT_FILE_DIRECTORY = ../../../include

# Please set your device here

#DEVICE = msp430X

#CC = $(GCC_DIR)/msp430-elf-gcc

#GDB = $(GCC_DIR)/msp430-elf-gdb

#CFLAGS = -I $(SUPPORT_FILE_DIRECTORY) -mmcu=$(DEVICE) -O2 -g

#LFLAGS = -L $(SUPPORT_FILE_DIRECTORY) -T $(DEVICE).ld

#all: ${OBJECTS}

#$(CC) $(CFLAGS) $(LFLAGS) $? -o $(DEVICE).out

#debug: all

#$(GDB) $(DEVICE).out
