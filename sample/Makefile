# include ../config.mk
CC = gcc
CXX = g++

SRC_FILES = ../src/ttstream.c \
			../src/ttfont.c \
			../src/ttcmap.c

CFLAGS = -I../include/

sample : sample.c $(SRC_FILES)
	$(CC) $(CFLAGS) -g -o ../build/sample sample.c $(SRC_FILES)

test : test.c
	$(CC) -o ../build/test test.c