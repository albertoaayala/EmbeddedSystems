//
// CS 431 - Lab 5 Server Skeleton
// PC/Linux (Provided)
//
// Change Log:
//   07/15/04  Created.  -Greg Dhuse
//   08/09/04  Skeleton. -Greg Dhuse
//

#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "pc_crc16.h"
#include "../common/lab03.h"

int main(int argc, char* argv[])
{
	double troll_pct=0;		// Perturbation % for the troll (if needed)
	int ifd,ofd,i,N,troll=0;	// Input and Output file descriptors (serial/troll)
	char str[MSG_BYTES_MSG],opt;	// String input
	struct termios oldtio, tio;	// Serial configuration parameters
	int VERBOSE = 0;		// Verbose output - can be overriden with -v

	// Command line options
	while ((opt = getopt(argc, argv, "t:v")) != -1) {
		switch (opt) {
			case 't':	troll = 1; 
					troll_pct = atof(optarg);
					break;
			case 'v':	VERBOSE = 1; break;
			default: 	break;
		}
	}

	printf("CS431 - Lab 3 Server\n(Enter a message to send.  Type \"quit\" to exit)\n");


	ifd = open("/dev/ttyS0", O_RDWR);
    if(ifd == -1) exit(-2);
	int logfile = open("./logfile.txt", O_WRONLY | O_CREAT);

	
	// Start the troll if necessary
	if (troll)
	{
		// Open troll process (lab5_troll) for output only
		FILE * pfile;		// Process FILE for troll (used locally only)
		char cmd[128];		// Shell command

		snprintf(cmd, 128, "./lab03_troll -p%f %s", troll_pct, (VERBOSE) ? "-v" : "");

		pfile = popen(cmd, "w");
		if (!pfile) { perror("lab5_troll"); exit(-1); }
		ofd = fileno(pfile);
	}
	else ofd = ifd;		// Use the serial port for both input and output
	
	tcgetattr(ifd, &oldtio);
	tio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	tio.c_iflag = 0;
	tio.c_oflag = 0;
	tio.c_lflag = 0;
	tcflush(ifd, TCIFLUSH);
	tcsetattr(ifd, TCSANOW, &tio);

	while(1)
	{

		char c = '\0';
        i = 0;
        while(c!='\n'){ 
            c = fgetc(stdin);
            str[i] = c;
            i++;
        }
        str[i-1] = 0;

        if (strcmp(str, "quit") == 0) break;

        int ack = 0;
        int attempts = 0;
        N = htons(pc_crc16(str, i));
        
		while (!ack)
		{
			printf("Sending (attempt %d)...\n", ++attempts);
			printf("crc: %d\n", N);
			int a = 0;
			write(ofd, &a, 1);
			write(ofd, &N, 2);
			write(ofd, &i, 1);
			write(ofd, str, i);
			write(logfile, &a, 1);
			write(logfile, &N, 2);
			write(logfile, &i, 1);
			write(logfile, str, i);

			printf("Message sent, waiting for ack... ");

            int b = read(ifd, &ack, 1);
			write(logfile, (void *)&ack, 1);

			printf("%s\n", ack ? "ACK" : "NACK, resending");
		}
		printf("\n");
	}

    tcflush(ifd, TCIFLUSH);
   	tcsetattr(ifd, TCSANOW, &oldtio);

	// Close the serial port
	close(ifd);
	close(logfile);
	return 0;
}
