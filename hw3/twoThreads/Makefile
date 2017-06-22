DEPS=linuxanalog.h
C=hw3.c linuxanalog.c
CC=gcc
OBJS = hw3.o linuxanalog.o

hw3:  $(DEPS)
	$(CC) $(DEPS) $(C) -o hw3 -lrt -lpthread
all:
	hw3
clean:
	rm -f hw3 linuxanalog.h.gch output.txt
