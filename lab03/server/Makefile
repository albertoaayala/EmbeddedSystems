DEPS=../common/lab03.h pc_crc16.h 
C=lab03_server.c pc_crc16.c
CC=gcc
OBJS = lab03_server.o pc_crc16.o

lab03:  $(DEPS)
	$(CC) $(DEPS) $(C) -o lab03
all:
	lab03
clean:
	rm -f logfile.txt lab03 ../common/lab03.h.gch pc_crc16.h.gch
