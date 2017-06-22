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

uint16_t minDigital = 0;
uint16_t maxDigital = 0;
char isHigh = 0;
sigset_t signal_set;

int* create_shmem(const char *name){
	int shmDesc = shm_open(name, O_CREAT|O_RDWR, S_IRWXU);
	if(ftruncate(shmDesc, MYSHMSIZE)){
		perror("ftruncate\n");
	}
	int *shmAre = mmap(0, MYSHMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmDesc, (long) 0);
	return shmAre;
}

int *init_shmem(int mem){
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

static void timer1Handler() {
    isHigh ? dac(maxDigital) : dac(minDigital);
    isHigh ^= 1;
}

int main(int argc, char** argv) {
	das1602_initialize();
	//FILE *fp = fopen("output.txt", "w+");
	timer_t timer1;
	float newFreq = 1.0;

	sem_t *sem = sem_open( SEMAPHORE_NAME,
		O_CREAT     |   // create the semaphore if it does not already exist
		O_CLOEXEC   ,   // close on execute
		S_IRWXU     |   // permissions:  user
		S_IRWXG     |   // permissions:  group
		S_IRWXO     ,   // permissions:  other
		1           );  // initial value of the semaphore

	int *maxDig = init_shmem(0);
	int *minDig = init_shmem(1);
	int *waveFreq = init_shmem(2);
	int *change = init_shmem(3);
	sem_wait(sem);
	maxDigital = *maxDig;
	minDigital = *minDig;
	newFreq = *waveFreq;
	sem_post(sem);

	/* Testing out of the lab
	if(1){
		fprintf(fp, "Max Digital: %2d\nMin Digital: %2d\n", maxDigital, minDigital);
	}
	*/

  //set the signals that need to be blocked for the timer
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGALRM);
  sigprocmask(SIG_BLOCK, &signal_set, 0);

	if(timer_create(CLOCK_REALTIME, NULL, &timer1) != 0){
		perror("timer_create");
		shm_unlink(SHM_MIN_AREA_NAME);
		shm_unlink(SHM_MAX_AREA_NAME);
		shm_unlink(SHM_WAVE_AREA_NAME);
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
		shm_unlink(SHM_MIN_AREA_NAME);
		shm_unlink(SHM_MAX_AREA_NAME);
		shm_unlink(SHM_WAVE_AREA_NAME);
		exit(1);
	}

	while(1){
		if(sigwaitinfo(&signal_set, NULL)==SIGALRM){
			if(*change == 1){
				sem_wait(sem);
				newFreq = *waveFreq;
				*change = 0;
				if(maxDigital != *maxDig || minDigital!=*minDig){
					maxDigital = *maxDig;
					minDigital = *minDig;
					/*Testing when out of lab
					fprintf(fp, "New Max: %2d\nNew Min: %2d\n", maxDigital, minDigital);
					*/
				}
				sem_post(sem);
				if(newFreq < 0){
					shm_unlink(SHM_MIN_AREA_NAME);
					shm_unlink(SHM_MAX_AREA_NAME);
					shm_unlink(SHM_WAVE_AREA_NAME);
					exit(1);
				}
				timer1Time.it_interval.tv_nsec = (long)((1000000000/(newFreq*2)));
				//set the timer to the times specified by timer1Time
				if(timer_settime(timer1, 0, &timer1Time, NULL) != 0) {
					perror("timer_settime");
					shm_unlink(SHM_MIN_AREA_NAME);
					shm_unlink(SHM_MAX_AREA_NAME);
					shm_unlink(SHM_WAVE_AREA_NAME);
					exit(1);
				}
			}
			timer1Handler();
			/* Testing out of lab
			isHigh ? fprintf(fp, " %f", newFreq) : fprintf(fp, "0 ");
			*/
		}
	}
}
