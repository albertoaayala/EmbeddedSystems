/* 
 * File:   build.h
 * Author: aaayala3
 *
 * Created on November 11, 2016, 11:35 AM
 */

#ifndef BUILD_H
#define	BUILD_H

#include "types.h"

    void build_extrema(extrema_t* e, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max);
    void build_pid_control(pid_control_t* pid_c, int16_t e, int16_t i, int16_t d, int16_t o, int16_t p_e, double kp, double ki, double kd);
    void update_pid_control(pid_control_t* pid_c, uint16_t setpoint, uint16_t measured, double dt);

#endif	/* BUILD_H */

