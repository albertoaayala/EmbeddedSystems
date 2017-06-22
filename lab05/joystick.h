/* 
 * File:   joystick.h
 * Author: aaayala3
 *
 * Created on October 6, 2016, 6:50 PM
 */
#ifndef JOYSTICK_H
#define	JOYSTICK_H

#include "types.h"


uint8_t trigger_debounce();
uint8_t thumb_debounce();
void setupADC2();
uint16_t readJoystickX();
uint16_t readJoystickY();
void joystick_init();

#endif	/* JOYSTICK_H */

