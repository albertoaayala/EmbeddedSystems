#include <stdlib.h>
#include <sys/io.h>
#include <inttypes.h>
#include "linuxanalog.h"

void das1602_initialize() {
	outw(0xF0A7, BADR1+8);
}

void dac(uint16_t value) {
	uint16_t send = value & 0x0fff;
	outw(send, BADR4+0);
}
