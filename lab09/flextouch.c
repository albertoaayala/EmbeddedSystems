#include <p33Fxxxx.h>
#define FCY 12800000UL
#include <libpic30.h>
#include <stdlib.h>
#include "types.h"
#include "lcd.h"
#include "led.h"
#include "joystick.h"
#include "flexmotor.h"
#include "flextouch.h"
#include "build.h"
#include <time.h>

extern extrema_t joystick;
extern extrema_t touchscreen;

uint16_t getMedian(uint16_t Samp[5])
{
  uint16_t temp, k, pass;
  for (pass = 1; pass < 5; pass++)
  {
    for (k = 0; k < 5 - pass ; k++)
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

uint16_t readTouch(){
    uint16_t x, touch_samples[5] = {0,0,0,0,0};
    for(x = 0; x < 5; x++){
        touch_samples[x] = touch_adc();
    }
    return getMedian(touch_samples);
}

uint16_t readTouchX(uint8_t reset_dim){
    if(reset_dim){
        // Connect to ADC
        CLEARBIT(PORTEbits.RE1);        //E1 = 0
        Nop();
        SETBIT(PORTEbits.RE2);          //E2 = 1
        Nop();
        SETBIT(PORTEbits.RE3);          //E3 = 1
        __delay_ms(10);
    }
    AD2CHS0bits.CH0SA = 0b01111;    //set ADC to Sample AN15 pin
    return readTouch();
}
uint16_t readTouchY(uint8_t reset_dim){
    if(reset_dim){
        // Connect to ADC
        SETBIT(PORTEbits.RE1);          //E1 = 1
        Nop();
        CLEARBIT(PORTEbits.RE2);        //E2 = 0
        Nop();
        CLEARBIT(PORTEbits.RE3);        //E3 = 0
        __delay_ms(10);
    }
    AD2CHS0bits.CH0SA = 0b01001;    //set ADC to Sample AN09 pin
    return readTouch();
}

void touch_init(){
        //disable ADC
    CLEARBIT(AD2CON1bits.ADON);

    SETBIT(TRISBbits.TRISB9); //set TRISB RB4 to input
    CLEARBIT(AD1PCFGLbits.PCFG9);
    CLEARBIT(AD2PCFGLbits.PCFG9); //set AD2 AN9 input pin as analog
    SETBIT(TRISBbits.TRISB15); //set TRISB RB5 to input
    CLEARBIT(AD2PCFGLbits.PCFG15); //set AD2 AN15 input pin as analog
    CLEARBIT(AD1PCFGLbits.PCFG15);

    // I/O pin set to output for E1, E2, E3
    CLEARBIT(TRISEbits.TRISE1);
    CLEARBIT(TRISEbits.TRISE2);
    CLEARBIT(TRISEbits.TRISE3);

    //Configure AD1CON1
    SETBIT(AD2CON1bits.AD12B); // set 12b Operation Mode
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
    lcd_locate(0,4);
    lcd_printf("Touch Xmax =\n");
    while(!trigger_debounce()){
        motor_set_duty(8, joystick_to_duty(&joystick, readJoystickX(), 0));
    }
    touchscreen.x_max = readTouchX(1);
    lcd_locate(13, 4);
    lcd_printf("%4d", touchscreen.x_max);
    while(trigger_debounce());
    lcd_locate(0,5);
    lcd_printf("Touch Xmin =\n");
    while(!trigger_debounce()){
        motor_set_duty(8, joystick_to_duty(&joystick, readJoystickX(), 0));
    }
    touchscreen.x_min = readTouchX(1);
    lcd_locate(13, 5);
    lcd_printf("%4d", touchscreen.x_min);
    while(trigger_debounce());
    lcd_locate(0,6);
    lcd_printf("Touch Ymax =\n");
    while(!trigger_debounce()){
        motor_set_duty(7, joystick_to_duty(&joystick, readJoystickY(), 1));
    }
    touchscreen.y_max = readTouchY(1);
    lcd_locate(13, 6);
    lcd_printf("%4d", touchscreen.y_max);
    while(trigger_debounce());
    lcd_locate(0,7);
    lcd_printf("Touch Ymin =\n");
    while(!trigger_debounce()){
        motor_set_duty(7, joystick_to_duty(&joystick, readJoystickY(), 1));
    }
    touchscreen.y_min = readTouchY(1);
    lcd_locate(13, 7);
    lcd_printf("%4d", touchscreen.y_min);
    while(trigger_debounce());
}

uint16_t touch_adc(){
    SETBIT(AD2CON1bits.SAMP);       //start to sample
    while(!AD2CON1bits.DONE);       //wait for conversion to finish
    CLEARBIT(AD2CON1bits.DONE);     //MUST HAVE! clear conversion done bit
    return ADC2BUF0;                //return sample
}

uint16_t touch_to_duty(const extrema_t* touch, uint16_t p, uint8_t dim){
    uint16_t duty = 900;
    if(dim){
        if(p < touch->y_min) p = touch->y_min;
        if(p > touch->y_max) p = touch->y_max;
        duty = 1000*((p-touch->y_min)*(2.1-0.9)/(touch->y_max-touch->y_min)+0.9);
    } else {
        if(p < touch->x_min) p = touch->x_min;
        if(p > touch->x_max) p = touch->x_max;
        duty = 1000*((p-touch->x_min)*(2.1-0.9)/(touch->x_max-touch->x_min)+0.9);
    }
    return duty; 
}

void touch_select_dim(uint8_t dim){
    if(dim  == 1) {
        // Connect to ADC
        SETBIT(PORTEbits.RE1);          //E1 = 1
        Nop();
        CLEARBIT(PORTEbits.RE2);        //E2 = 0
        Nop();
        CLEARBIT(PORTEbits.RE3);        //E3 = 0
        AD2CHS0bits.CH0SA = 0b01001;
    }
    else if(dim == 0) {
       // Connect to ADC
        CLEARBIT(PORTEbits.RE1);        //E1 = 0
        Nop();
        SETBIT(PORTEbits.RE2);          //E2 = 1
        Nop();
        SETBIT(PORTEbits.RE3);          //E3 = 1
        AD2CHS0bits.CH0SA = 0b01111;    //set ADC to Sample AN15 pin
    }
}