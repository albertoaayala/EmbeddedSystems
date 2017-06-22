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

#include "lcd.h"
#include "types.h"
#include "flextouch.h"
#include "flexmotor.h"


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
uint16_t setpointX = 1715, setpointY = 1570;
uint16_t x_min = 300, x_max = 3000;
uint16_t y_min = 430, y_max = 2720;
volatile uint8_t valueChanged = 0;


void delay(unsigned int mseconds)
{
    clock_t goal = mseconds + clock();
    while (goal > clock());
}

uint16_t getMedian(uint16_t Samp[10]){
  uint16_t temp, k, pass;
  for (pass = 1; pass < 10; pass++)
  {
    for (k = 0; k < 10 - pass ; k++)
    {
      if (Samp[k] > Samp[k+1])
      {
        temp = Samp[k];
        Samp[k] = Samp[k+1];
        Samp[k+1] = temp;
      }
    }
  }
  return Samp[2];
}

 void __attribute__ ((__interrupt__)) _T3Interrupt(void) {
    valueChanged = 1;
    //TMR2 = 0x00;
    IFS0bits.T3IF = 0;
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

typedef struct pidValues pidValues;
struct pidValues{
    int16_t error;
    int16_t integral;
    int16_t derivative;
    int16_t output;
    int16_t previous_error;
};


int main(int argc, char** argv) {
    __C30_UART=1;
    lcd_initialize();
    touch_init();
    motor_init(8);
    motor_init(7);
    timer_init();
    pidValues xpid, ypid;
    xpid.error = 0;
    xpid.integral = 0;
    xpid.derivative = 0;
    xpid.output = 0;
    xpid.previous_error = 0;
    ypid.error = 0;
    ypid.integral = 0;
    ypid.derivative = 0;
    ypid.output = 0;
    ypid.previous_error = 0;
    double kp = 1, ki = 0.015, kd = 0.25;
    lcd_clear();
    lcd_locate(0,0);
    lcd_printf(" ------LAB 07------\n");
    while(1){
        if(valueChanged == 1){
            valueChanged = 0;
            uint16_t x = 0;
            uint16_t xSamples[10] = {0,0,0,0,0,0,0,0,0,0};
            uint16_t ySamples[10] = {0,0,0,0,0,0,0,0,0,0};
            touch_select_dim(0);
            delay(10);
            for(x = 0; x < 10; x++){
                xSamples[x] = touch_adc();
            }
            xMeasured = getMedian(xSamples);
            /*touch_select_dim(1);
            delay(10);
            for(x = 0; x < 10; x++){
                ySamples[x] = touch_adc();
            }
            yMeasured = getMedian(ySamples);*/
            
            xpid.error = setpointX - xMeasured;
            xpid.integral = xpid.integral + xpid.error*0.05;
            xpid.derivative = (xpid.error - xpid.previous_error)/0.05;
            xpid.output = kp*xpid.error + ki*xpid.integral + kd*xpid.derivative;
            xpid.previous_error = xpid.error;
            /* Set X motor*/
            double newXPosition = setpointX+xpid.output;
            if (newXPosition < x_min) newXPosition = x_min;
            if (newXPosition > x_max) newXPosition = x_max;
            uint16_t x_duty = (uint16_t)(1000*((((newXPosition-x_min)*(2.1-.9))/(x_max-x_min))+.9));
            motor_set_duty(8, x_duty);

            /*ypid.error = setpointY - yMeasured;
            ypid.integral = ypid.integral + ypid.error*0.05;
            ypid.derivative = (ypid.error - ypid.previous_error)/0.05;
            ypid.output = kp*ypid.error + ki*ypid.integral + kd*ypid.derivative;
            ypid.previous_error = ypid.error;
            //Set Y
            double newYPosition = setpointY+xpid.output;
            if (newYPosition < y_min) newYPosition = y_min;
            if (newYPosition > y_max) newYPosition = y_max;
            uint16_t y_duty = (uint16_t)(1000*((((newYPosition-y_min)*(2.1-.9))/(y_max-y_min))+.9));
            motor_set_duty(7, y_duty);*/
            lcd_locate(0,1);
            lcd_printf("P=%.2f,I=%.2f\n,D=%.3f\n", kp, ki, kd);
            lcd_printf("Set Position=1715\n")
            lcd_printf("Current X=%4d\n", xMeasured);
            lcd_printf("Derivative=%4d\n", xpid.derivative);
            lcd_printf("Integral=%4d\n", xpid.integral);
            lcd_printf("Feedback=%4d\n", xpid.output);
        }
    }
    return 0;
}
