#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "linuxanalog.h"

pthread_mutex_t waveSpecMutex;
uint16_t minDigital = 0;
uint16_t maxDigital = 0;
uint8_t change = 0, done = 0;
long counter = 0;
float newFreq = 0, maxFreq = 0;
char isHigh = 0;
sigset_t signal_set;

int getDigitalValue(float voltage) {
	return (int)(((voltage+5)*(powf(2,12)-1))/10);
}

static void timer1Handler(/*int signum*/) {
    isHigh ? dac(maxDigital) : dac(minDigital);
    isHigh ^= 1;
}

void *waveGenerator(void *args){
	das1602_initialize();
	/* Testing out of lab
	FILE *fp = fopen("output.txt", "w+");
	*/
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
	timer1Time.it_interval.tv_nsec = (long)((1000000000/(newFreq*2)));

	if(timer_settime(timer1, 0, &timer1Time, NULL) != 0) {
		perror("timer_settime");
		exit(1);
	}

	while(1){
		/* Test out of lab
		if(done){
			fclose(fp);
			exit(1);
		}
		*/
		if(change == 1){
			pthread_mutex_lock(&waveSpecMutex);
			/* Test out of lab
			if(done){
				fclose(fp);
				exit(1);
			}
			*/
			timer1Time.it_interval.tv_nsec = (long)((1000000000/(newFreq*2)));
			pthread_mutex_unlock(&waveSpecMutex);

			//set the timer to the times specified by timer1Time
			if(timer_settime(timer1, 0, &timer1Time, NULL) != 0) {
				perror("timer_settime");
				exit(1);
			}
			change = 0;
		}
		if(sigwaitinfo(&signal_set, NULL)!=-1){
			timer1Handler(/*14*/);
			/* Test out of lab
			counter++;
			isHigh ? fprintf(fp, " %f", freqPrint) : fprintf(fp, "0 ");
			*/
		}
	}
}

int main(int argc, char** argv) {
	/* intitialize variables */
	float minVoltage = 0, maxVoltage = 0, freq = 0;
	char c = 0;

	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGALRM);
	sigprocmask(SIG_BLOCK, &signal_set, NULL);
	pthread_t waveThreadID;

	printf("Enter Minimum Voltage (-5 to +5 volts, other to quit):");
	scanf("%f", &minVoltage);
	if(minVoltage>5 || minVoltage<-5){
		return 0;
	}
	//Get the digital value of the voltage given
	minDigital = getDigitalValue(minVoltage);
	printf("Min voltage: %f, Min digital: %d\n", minVoltage, minDigital);

	printf("\nEnter Maximum Voltage (-5 to +5 volts, other to quit):");
	scanf("%f", &maxVoltage);
	if(maxVoltage>5 || maxVoltage<-5){
		return 0;
	}

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
	scanf("%f", &newFreq);

	pthread_mutex_init(&waveSpecMutex, NULL);						//intitialize the mutex
	if(pthread_create(&waveThreadID, NULL, waveGenerator, NULL) != 0){
		perror("pthread_create");
		exit(1);
	}

	while(1) {
		printf("Enter frequency (0 to %f Hz, other to quit)"/* Counter: %li"*/, maxFreq);//, counter);
		scanf("%f", &freq);
		if(freq < 0 || freq>maxFreq){
			/* Test out of lab
			done = 1;
			*/
			exit(1);
		}
		pthread_mutex_lock(&waveSpecMutex);
		newFreq = freq;
		pthread_mutex_unlock(&waveSpecMutex);
		change = 1;
	}
}
