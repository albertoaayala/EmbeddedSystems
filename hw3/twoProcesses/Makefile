DEPS=linuxanalog.h
C=hw3Proc1.c hw3Proc2.c linuxanalog.c
CC=gcc
OBJS=hw3Proc1.o hw3Proc2.o

all: hw3

hw3Proc1.o: hw3Proc1.c $(DEPS)
	$(CC) $(DEPS) -c $< linuxanalog.c -lrt -lpthread

hw3Proc2.o: hw3Proc2.c $(DEPS)
	$(CC) $(DEPS) -c $< linuxanalog.c -lrt -lpthread

hw3: $(OBJS)
	$(CC) hw3Proc1.o linuxanalog.o -o hw3Proc1 -lrt -lpthread
	$(CC) hw3Proc2.o linuxanalog.o -o hw3Proc2 -lrt -lpthread




clean:
	rm -f hw3Proc1 hw3Proc2 linuxanalog.h.gch output.txt *.o
