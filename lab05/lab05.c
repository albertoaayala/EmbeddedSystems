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
#include <libpic30.h>

#include "lcd.h"
#include "joystick.h"
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

int main(int argc, char** argv) {
    __C30_UART=1;
    lcd_initialize();
    setupADC2();
    lcd_clear();
    joystick_init();
    while(1){
        if(thumb_debounce()) {
            lcd_clear();
            joystick_init();
        }
    }
    return 0;
}