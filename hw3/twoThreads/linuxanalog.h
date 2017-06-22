#ifndef LINUXANALOG_H
#define	LINUXANALOG_H
#include <inttypes.h>

#define BADR0 0xd000
#define	BADR1 0xd0a0
#define BADR2	0xd080
#define BADR3	0xd060
#define BADR4	0xd040

void das1602_initialize();

void dac(uint16_t value);


#endif	/* LINUXANALOG_H */
