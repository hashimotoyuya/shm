# Makefile 
# Copyright (C) Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp>, 2004-10-07
 
SHELL = /bin/sh

TARGET = shm_show_info

INC_DIR = include
LIB_DIR = lib

SRC = main.c
OBJ = $(SRC:.c=.o)

CC = gcc
CFLAGS = -c -O2 -Wall -I$(HOME)/include

.c.o:
	$(CC) $(CFLAGS) $<

all:$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) *~ 

$(TARGET): $(OBJ)
	$(CC) -o $@ $(OBJ) -lshm -L$(HOME)/lib

install: $(TARGET)
	@if ! test -d $(HOME)/bin; then mkdir $(HOME)/bin; fi
	install -m 744 $(TARGET) $(HOME)/bin

uninstall:
	rm -f $(HOME)/bin/$(TARGET)
