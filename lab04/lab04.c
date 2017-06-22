#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "linuxanalog.h"

uint16_t minDigital = 0;
uint16_t maxDigital = 0;
char isHigh = 0;

int getDigitalValue(float voltage) {
	return (int)(((voltage+5)*(powf(2,12)-1))/10);
}

static void timer1Handler(int signum) {
    isHigh ? dac(maxDigital) : dac(minDigital);
    isHigh ^= 1;
}

void setup_signal_handler() {
 struct sigaction action;
	// Ensure that the entire structure is zeroed out.
	memset(&action, 0, sizeof(action));
	// Set the sa_handler member to point to the desired handler function.
	action.sa_handler = timer1Handler;
	// Call sigaction to change the action taken upon receipt of the SIGALRM signal.
	if (sigaction(SIGALRM, &action, NULL) != 0)
	{	
		// If there is an error, print out a message and exit.
		perror("sigaction");
		exit(1);
	}
}

int main(int argc, char** argv) {
	/* intitialize variables */
	long counter = 0;
	float minVoltage = 0, maxVoltage = 0, freq = 0, maxFreq = 0;
	char c = 0;


	printf("Enter voltage #1 (-5 to +5 volts, other to quit):");
	scanf("%f", &minVoltage);

	//Get the digital value of the voltage given
	minDigital = getDigitalValue(minVoltage);
	printf("Min voltage: %f, Min digital: %d\n", minVoltage, minDigital);

	printf("\nEnter voltage #2 (-5 to +5 volts, other to quit):");
	scanf("%f", &maxVoltage);

	//get the digital value of the voltage given
	maxDigital = getDigitalValue(maxVoltage);
	printf("Max voltage: %f, Max digital: %d\n", maxVoltage, maxDigital);

	/*Find Max frequency*/	
	struct timespec res;
	if(clock_getres(CLOCK_REALTIME, &res) != 0){
		perror("clock_getres");
		exit(1);
	}
	maxFreq = 1/(res.tv_sec + (res.tv_nsec*pow(10,-9)));

	printf("Enter frequency (0 to %f Hz, other to quit)", maxFreq);
	scanf("%f", &freq);

	das1602_initialize();
	setup_signal_handler();

	timer_t timer1;
	if(timer_create(CLOCK_REALTIME, NULL, &timer1) != 0){
		perror("timer_create");
		exit(1);
	}
	
	//set up the time til first interrupt and the period between interrupts
	struct itimerspec timer1Time;
	timer1Time.it_value.tv_sec = 0;
	timer1Time.it_value.tv_nsec = 1000000;
	timer1Time.it_interval.tv_sec = 0;
	timer1Time.it_interval.tv_nsec = (long)((1000000000/(freq*2)));

	//set the timer to the times specified by timer1Time
	if(timer_settime(timer1, 0, &timer1Time, NULL) != 0) {
		perror("timer_settime");
		exit(1);
	}

	while(1) {
		// Output the count value after every 100 million loops. 
		if (counter % 100000000 == 0) printf("Counter: %u\n", counter); 
		counter++;
	}
}
