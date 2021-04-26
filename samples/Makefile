# Makefile for samples
# Copyright (C) Koichi OZAKI <ozaki@cc.utsunomiya-u.ac.jp>, 2003-11-05
 
SHELL = /bin/sh

TARGET = $(SAMPLES)

INC_DIR = include
LIB_DIR = lib

SRC = sample1.c sample2.c
OBJ = $(SRC:.c=.o)
SAMPLES = $(SRC:.c= )

CC = gcc
CFLAGS = -c -g -Wall -I$(HOME)/include
.c.o:
	$(CC) $(CFLAGS) $<

all:$(SAMPLES)

clean:
	rm -f $(OBJ) $(TARGET) *~ 

$(SAMPLES): $(OBJ)
	$(CC) -o $@ $@.o -lshm -L$(HOME)/lib
