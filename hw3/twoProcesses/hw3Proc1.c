#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "linuxanalog.h"

#define MYSHMSIZE 4
#define SHM_MAX_AREA_NAME "/maxDigital"
#define SHM_MIN_AREA_NAME "/minDigital"
#define SHM_WAVE_AREA_NAME "/waveFreq"
#define SHM_CHANGE_AREA_NAME "/change"
#define SEMAPHORE_NAME "/hw3"

int* create_shmem(const char* name){
	int shmDesc = shm_open(name, O_CREAT|O_RDWR, S_IRWXU);
	if(ftruncate(shmDesc, MYSHMSIZE)){
		perror("ftruncate\n");
	}
	int *shmAre = mmap(0, MYSHMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmDesc, (long) 0);
	return shmAre;
}

int* init_shmem(int mem){
	switch (mem) {
		case 0:
	    	return create_shmem(SHM_MAX_AREA_NAME);
		case 1:
			return create_shmem(SHM_MIN_AREA_NAME);
		case 2:
			return create_shmem(SHM_WAVE_AREA_NAME);
		case 3:
			return create_shmem(SHM_CHANGE_AREA_NAME);
	}
}

int getDigitalValue(float voltage) {
	return (int)(((voltage+5)*(powf(2,12)-1))/10);
}

int main(int argc, char** argv) {
	/* intitialize variables */
    int *maxDigital = init_shmem(0);
	int *minDigital = init_shmem(1);
	int *waveFreq = init_shmem(2);
	int *change = init_shmem(3);
	float minVoltage = 0, maxVoltage = 0, freq = 0;

	sem_t *sem = sem_open( SEMAPHORE_NAME,
		O_CREAT     |   // create the semaphore if it does not already exist
		O_CLOEXEC   ,   // close on execute
		S_IRWXU     |   // permissions:  user
		S_IRWXG     |   // permissions:  group
		S_IRWXO     ,   // permissions:  other
		1           );  // initial value of the semaphore

	printf("Enter minimum voltage (-5 to +5 volts, other to quit):");
	sem_wait(sem);
	scanf("%f", &minVoltage);
	if(minVoltage>5 || minVoltage<-5){
		exit(1);
	}
	//Get the digital value of the voltage given
	*minDigital = getDigitalValue(minVoltage);

	printf("Min voltage: %f, Min digital: %d\n", minVoltage, *minDigital);

	printf("\nEnter maximum voltage (-5 to +5 volts, other to quit):");
	scanf("%f", &maxVoltage);
	if(maxVoltage>5 || maxVoltage<-5){
		exit(1);
	}

	//get the digital value of the voltage given
	*maxDigital = getDigitalValue(maxVoltage);
	//sem_post(sem);
	printf("Max voltage: %f, Max digital: %d\n", maxVoltage, *maxDigital);

	/*Find Max frequency*/
	struct timespec res;
	if(clock_getres(CLOCK_REALTIME, &res) != 0){
		perror("clock_getres");
		exit(1);
	}
	float maxFreq = 1/((res.tv_sec + (res.tv_nsec*pow(10,-9))*2));
	printf("Enter frequency (0 to %f Hz)", maxFreq);
	scanf("%f", &freq);
	*waveFreq = freq;
	*change = 1;
	sem_post(sem);

	while(1) {
		// Output the count value after every 100 million loops.
		//if (counter % 100000000 == 0) printf("Counter: %u\n", counter);
		//counter++;
		printf("Enter frequency (0 to %f Hz, -1 to quit, -2 to change V)", maxFreq);
		scanf("%f", &freq);
		if(freq==-2){
                        exit(1);
                }
	        sem_wait(sem);
		*change = 1;
		*waveFreq = freq;
        	sem_post(sem);
		if(freq==-1){
                        shm_unlink(SHM_MIN_AREA_NAME);
                        shm_unlink(SHM_MAX_AREA_NAME);
                        shm_unlink(SHM_WAVE_AREA_NAME);
                        exit(1);
                }
	}
}
