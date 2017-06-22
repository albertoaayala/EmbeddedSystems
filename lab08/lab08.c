/*
 * File:   lab05.c
 * Author: aaayala3
 *
 * Created on October 6, 2016, 5:16 PM
 */

#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
//#include <stdlib.h>
#include <libpic30.h>
#include<time.h>
#include "led.h"
#include "lcd.h"
#include "types.h"
#include "flextouch.h"
#include "flexmotor.h"
#include "joystick.h"
#include "build.h"


/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);
/*
 *
 */

uint16_t xMeasured = 0, yMeasured = 0;
volatile uint8_t timerTriggered = 0;

extrema_t joystick;
extrema_t touchscreen;

void __attribute__ ((__interrupt__)) _T3Interrupt(void) {
    timerTriggered = 1;
    IFS0bits.T3IF = 0;
}

uint16_t joystick_to_touch(const extrema_t* joystick, const extrema_t* touch, uint8_t dim){
    uint16_t p = 0, touch_p = 0;
    if(dim){
        p = readJoystickY();
        touch_p = ((p-joystick->y_min)*(touch->y_max-touch->y_min)/(joystick->y_max-joystick->y_min))+touch->y_min;
        //if(touch_p > touch->y_max) touch_p = touch->y_max;
        //else if(touch_p < touch->y_min) touch_p = touch->y_min;
        //else touch_p = ((p-joystick->y_min)*(touch->y_max-touch->y_min)/(joystick->y_max-joystick->y_min))+touch->y_min;
    } else{
        p = readJoystickX();
        touch_p = ((p-joystick->x_min)*(touch->x_max-touch->x_min)/(joystick->x_max-joystick->x_min))+touch->x_min;
        //if(touch_p > touch->x_max) touch_p = touch->x_max;
        //else if(touch_p < touch->x_min) touch_p = touch->x_min;
        //else touch_p = ((p-joystick-x_min)*(touch->x_max-touch->x_min)/(joystick->x_max-joystick->x_min))+touch->x_min;
    }
    return touch_p;
}

void timer_init(){
    CLEARBIT(T3CONbits.TON);     // Disable Timer
    CLEARBIT(T3CONbits.TCS);    // Select internal instruction cycle clock
    CLEARBIT(T3CONbits.TGATE);  // Disable Gated Timer mode
    T3CONbits.TCKPS = 0b11;     // Select 1:256 Prescaler
    TMR3 = 0x00;                // Clear timer register
    PR3 = 2500;                 // Set timer period 1ms:
    // 2500 = 50*10^-3 * 12.8*10^6 * 1/256
    CLEARBIT(IFS0bits.T3IF);    // Clear Timer3 interrupt status flag
    SETBIT(IEC0bits.T3IE);    // Enable Timer3 interrupt enable control bit
    SETBIT(T3CONbits.TON); /* Turn Timer 3 on */
}

int main(int argc, char** argv) {
    __C30_UART=1;
    lcd_initialize();
    setupADC2();
    joystick_init();
    motor_init(8);
    motor_init(7);
    touch_init();
    pid_control_t xpid, ypid;
    build_pid_control(&xpid, 0, 0, 0, 0, 0, 1, 0.015, 0.5);
    build_pid_control(&ypid, 0, 0, 0, 0, 0, 0.6, 0.006, 1.20);
    uint8_t update_lcd = 0;
    uint16_t nextSetX, nextSetY;
    uint16_t setpointX = (touchscreen.x_max + touchscreen.x_min)/2;
    uint16_t setpointY = (touchscreen.y_max + touchscreen.y_min)/2;
    lcd_clear();
    lcd_locate(0,0);
    lcd_printf(" ------LAB 08------\n");
    timer_init();
    while(1){
        if(trigger_debounce()){
            setpointX = joystick_to_touch(&joystick, &touchscreen, 0);
            setpointY = joystick_to_touch(&joystick, &touchscreen, 1);
        }
        while(trigger_debounce());
        if(timerTriggered){
            timerTriggered = 0;
            nextSetX = joystick_to_touch(&joystick, &touchscreen, 0);
            nextSetY = joystick_to_touch(&joystick, &touchscreen, 1);
            xMeasured = readTouchX(1);
            yMeasured = readTouchY(1);
            update_pid_control(&xpid, setpointX, xMeasured, 0.05);
            double newXPosition = setpointX+xpid.output;
            uint16_t x_duty = touch_to_duty(&touchscreen, newXPosition, 0);
            //Set X motor
            motor_set_duty(8, x_duty);
            update_pid_control(&ypid, setpointY, yMeasured, 0.05);
            double newYPosition = setpointY+ypid.output;
            uint16_t y_duty = touch_to_duty(&touchscreen, newYPosition, 1);
            //Set Y
            motor_set_duty(7, y_duty);
            if(update_lcd >= 5){
                lcd_locate(0,1);
                lcd_printf("SetX:%4d, SetY:%4d\n", setpointX, setpointY);
                lcd_printf("X:%4d, Y:%4d\n", xMeasured, yMeasured);
                lcd_printf("JX=%4d, JY=%4d\n", nextSetX, nextSetY);
                lcd_printf("Kp_x=%.3f,Kp_y=%.3f\n", xpid.kp, ypid.kp);
                lcd_printf("Ki_x=%.3f,Ki_y=%.3f\n", xpid.ki, ypid.ki);
                lcd_printf("Kd_x=%.3f,Kd_y=%.3f\n", xpid.kd, ypid.kd);
                update_lcd = 0;
            }
            update_lcd++;
        }
        if(thumb_debounce()){
             joystick_init();
             motor_init(8);
             motor_init(7);
             touch_init();
             setpointX = (touchscreen.x_max + touchscreen.x_min)/2;
             setpointY = (touchscreen.y_max + touchscreen.y_min)/2;
        }
    }
    return 0;
}
