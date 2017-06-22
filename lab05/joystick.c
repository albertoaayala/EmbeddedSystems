#include <p33Fxxxx.h>
#include "types.h"
#include <stdio.h>
#include "lcd.h"
#include "joystick.h"
#include "flexmotor.h"


// Joystick x axis
// PIN: RB4 
// PORT: B
// ADC Input: ADC2CH4 

// Joystick y axis
// PIN: RB5
// PORT: B
// ADC Input: ADC2CH5

// Balance Board x axis
// PIN: RB15
// PORT: B
// ADC Input: ADC1CH15

// Balance Board y axis
// PIN: RB9
// PORT: B
// ADC Input: ADC1CH9

uint16_t x_min, x_max, y_min, y_max, x_width, y_width;

uint8_t trigger_debounce(){
    uint16_t T = 0;
    while(!PORTEbits.RE8){
        T++;
        if (T == 5000) {
            return 1;
        }
    }
    return 0;
}

uint8_t thumb_debounce(){
    uint16_t T = 0;
    while(!PORTDbits.RD10){
        T++;
        if (T == 5000) {
            return 1;
        }
    }
    return 0;
}

void setupADC2(){
    //disable ADC
    CLEARBIT(AD2CON1bits.ADON);
    //initialize PIN
    SETBIT(TRISBbits.TRISB4); //set TRISB RB4 to input
    CLEARBIT(AD2PCFGLbits.PCFG4); //set AD2 AN4 input pin as analog

    SETBIT(TRISBbits.TRISB5); //set TRISB RB5 to input
    CLEARBIT(AD2PCFGLbits.PCFG5); //set AD2 AN5 input pin as analog
    //Configure AD1CON1
    CLEARBIT(AD2CON1bits.AD12B); //set 10b Operation Mode
    AD2CON1bits.FORM = 0; //set integer output
    AD2CON1bits.SSRC = 0x7; //set automatic conversion
    //Configure AD1CON2
    AD2CON2 = 0; //not using scanning sampling
    //Configure AD1CON3
    CLEARBIT(AD2CON3bits.ADRC); //internal clock source
    AD2CON3bits.SAMC = 0x1F; //sample-to-conversion clock = 31Tad
    AD2CON3bits.ADCS = 0x2; //Tad = 3Tcy (Time cycles)
    //Leave AD1CON4 at its default value
    //enable ADC
    SETBIT(AD2CON1bits.ADON);
}

uint16_t readJoystickX(){
    AD2CHS0bits.CH0SA = 0b0100; //set ADC to Sample AN20 pin
    SETBIT(AD2CON1bits.SAMP); //start to sample
    while(!AD2CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD2CON1bits.DONE); //MUST HAVE! clear conversion done bit
    return ADC2BUF0; //return sample
}

uint16_t readJoystickY(){
    AD2CHS0bits.CH0SA = 0b0101; //set ADC to Sample AN20 pin
    SETBIT(AD2CON1bits.SAMP); //start to sample
    while(!AD2CON1bits.DONE); //wait for conversion to finish
    CLEARBIT(AD2CON1bits.DONE); //MUST HAVE! clear conversion done bit
    return ADC2BUF0; //return sample
}

void joystick_init(){
    SETBIT(AD1PCFGHbits.PCFG20);
    SETBIT(TRISEbits.TRISE8);
    SETBIT(TRISDbits.TRISD10);
    x_min = x_max = y_min = y_max = x_width = y_width = 0;
    uint16_t update_lcd = 0;
    uint16_t pulseWidthX = 0;
    uint16_t pulseWidthY = 0;
    lcd_locate(0,0);
    lcd_printf(" ------LAB 05------\n");
    lcd_locate(0, 1);
    lcd_printf("Joystick max X = \n");
    while(!trigger_debounce()){
        x_max = readJoystickX();
        if(update_lcd >= 1000){
            lcd_locate(17, 1);
            lcd_printf("%3d", x_max);
            update_lcd = 0;
        }
        update_lcd++;
    }
    update_lcd = 0;
    while(trigger_debounce());
    lcd_locate(0,2);
    lcd_printf("Joystick min X = \n");
    while(!trigger_debounce()){
        x_min = readJoystickX();
        if(update_lcd >= 1000){
            lcd_locate(17, 2);
            lcd_printf("%3d", x_min);
            update_lcd = 0;
        }
        update_lcd++;
    }
    while(trigger_debounce());
    lcd_locate(0,3);
    lcd_printf("Joystick max Y = \n");
    while(!trigger_debounce()){
        y_max = readJoystickY();
        if(update_lcd >= 1000){
            lcd_locate(17, 3);
            lcd_printf("%3d", y_max)
            update_lcd = 0;
        }
        update_lcd++;
    }
    while(trigger_debounce());
    lcd_locate(0,4);
    lcd_printf("Joystick min Y = \n");
    while(!trigger_debounce()){
        y_min = readJoystickY();
        if(update_lcd >= 1000){
            lcd_locate(17,4);
            lcd_printf("%3d", y_min);
            update_lcd = 0;
        }
        update_lcd++;
    }
    while(trigger_debounce());
    motor_init(8);
    motor_init(7);
    lcd_locate(0,5);
    lcd_printf("Pulse width X =\n");
    while(!trigger_debounce()){
        x_width = readJoystickX();
        if(x_width<x_min) pulseWidthX = 900;
        else pulseWidthX = 1000*((((x_width-x_min)*(2.1-.9))/(x_max-x_min))+.9);
        if(pulseWidthX > 2100) pulseWidthX = 2100;
        motor_set_duty(8, pulseWidthX);
        if(update_lcd >= 1000){
            lcd_locate(16, 5);
            lcd_printf("%4d", pulseWidthX);
            update_lcd = 0;
        }
        update_lcd++;
    }
    while(trigger_debounce());
    lcd_locate(0,6);
    lcd_printf("Pulse width Y =\n");
    while(!trigger_debounce()){
        y_width = readJoystickY();
        if(y_width<y_min) pulseWidthY = 900;
        else pulseWidthY = 1000*((((y_width-y_min)*(2.1-.9))/(y_max-y_min))+.9);
        if(pulseWidthY > 2100) pulseWidthY = 2100;
        motor_set_duty(7, pulseWidthY);
        if(update_lcd >= 1000){
            lcd_locate(16, 6);
            lcd_printf("%4d", pulseWidthY);
            update_lcd = 0;
        }
        update_lcd++;
    }
    while(trigger_debounce());
}