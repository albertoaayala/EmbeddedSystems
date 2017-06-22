#include <p33Fxxxx.h>
#include <libpic30.h>
#include <stdlib.h>
#include "flexmotor.h"

void motor_init(uint8_t chan){
    CLEARBIT(T2CONbits.TON);
    CLEARBIT(T2CONbits.TCS);
    CLEARBIT(T2CONbits.TGATE);
    TMR2 = 0x00;
    T2CONbits.TCKPS = 0b10;
    CLEARBIT(IFS0bits.T2IF);
    CLEARBIT(IEC0bits.T2IE);
    PR2 = 4000;                 //4000 = 20*10E-3*12.7E6*(1/64)
    if(chan == 8){
        CLEARBIT(TRISDbits.TRISD7);
        OC8R = 0;
        OC8RS = 3700;
        OC8CON =0x0006;
    }
    else {
        CLEARBIT(TRISDbits.TRISD5);
        OC7R = 0;
        OC7RS = 3700;
        OC7CON = 0x0006;
    }
    SETBIT(T2CONbits.TON);
}

void motor_set_duty(uint8_t chan, uint16_t t_duty_us){
    if(chan == 8){
        OC8RS = 4000 - ((4*t_duty_us)/20);
    }
    else {
        OC7RS = 4000 - ((4*t_duty_us)/20);
    }
}