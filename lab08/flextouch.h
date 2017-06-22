
/*
 * File:   flexmotor.h
 * Author: aaayala3
 *
 * Created on October 6, 2016, 5:15 PM
 */

#ifndef FLEXTOUCH_H
#define	FLEXTOUCH_H

#include "types.h"

uint16_t getMedian(uint16_t Samp[10]);
uint16_t readTouch();
uint16_t readTouchX(uint8_t reset_dim);
uint16_t readTouchY(uint8_t reset_dim);
void touch_init();
uint16_t touch_adc();
uint16_t touch_to_duty(const extrema_t* touch, uint16_t p, uint8_t dim);

#endif	/* FLEXTOUCH_H */
