#include <p33Fxxxx.h>
#include <stdio.h>
#include <libpic30.h>
#include<time.h>
#include "led.h"
#include "lcd.h"
#include "types.h"
#include "flextouch.h"
#include "flexmotor.h"
#include "joystick.h"

void build_extrema(extrema_t* e, uint16_t x_min, uint16_t x_max, uint16_t y_min, uint16_t y_max){
    e->x_min = x_min;
    e->x_max = x_max;
    e->y_min = y_min;
    e->y_max = y_max;
}

void build_pid_control(pid_control_t* pid_c, int16_t e, int16_t i, int16_t d, int16_t o, int16_t p_e, double kp, double ki, double kd){
    pid_c->error = e;
    pid_c->integral = i;
    pid_c->derivative = d;
    pid_c->output = o;
    pid_c->previous_error = p_e;
    pid_c->kp = kp;
    pid_c->ki = ki;
    pid_c->kd = kd;
}

void update_pid_control(pid_control_t* pid_c, int16_t setpoint, int16_t measured, double dt){
    pid_c->error = setpoint - measured;
    pid_c->integral += pid_c->error*dt;
    pid_c->derivative = (pid_c->error - pid_c->previous_error)/dt;
    pid_c->output = pid_c->kp*pid_c->error + pid_c->ki*pid_c->integral + pid_c->kd*pid_c->derivative;
    pid_c->previous_error = pid_c->error;
}
