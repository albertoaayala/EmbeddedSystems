#include <stdlib.h>
#include "../common/types.h"
#include "../common/lab03.h"

#ifndef FLEXSERIAL_H
#define	FLEXSERIAL_H

void uart2_init(uint16_t baud);

int uart2_putc(uint8_t c);

uint8_t uart2_getc();

#endif	/* FLEXSERIAL_H */

