/* 
 * File:   flexmotor.h
 * Author: aaayala3
 *
 * Created on October 6, 2016, 5:15 PM
 */

#ifndef FLEXMOTOR_H
#define	FLEXMOTOR_H

#include "types.h"

    void motor_init(uint8_t chan);
    void motor_set_duty(uint8_t chan, uint16_t t_duty_us);

#endif	/* FLEXMOTOR_H */

