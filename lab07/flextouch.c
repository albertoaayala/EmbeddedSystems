#include <p33Fxxxx.h>
#include <libpic30.h>
#include <stdlib.h>
#include "types.h"

void readX(){
    // Connect to ADC
    CLEARBIT(PORTEbits.RE1);        //E1 = 0
    Nop();
    SETBIT(PORTEbits.RE2);          //E2 = 1
    Nop();
    SETBIT(PORTEbits.RE3);          //E3 = 1
    AD2CHS0bits.CH0SA = 0b01111;    //set ADC to Sample AN15 pin
}
void readY(){
    // Connect to ADC
    SETBIT(PORTEbits.RE1);          //E1 = 1
    Nop();
    CLEARBIT(PORTEbits.RE2);        //E2 = 0
    Nop();
    CLEARBIT(PORTEbits.RE3);        //E3 = 0
    AD2CHS0bits.CH0SA = 0b01001;    //set ADC to Sample AN09 pin
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
}

void touch_select_dim(uint8_t dim){
    if(dim  == 1) {
        readY();
    }
    else if(dim == 0) {
        readX();
    }
}

uint16_t touch_adc(){
        SETBIT(AD2CON1bits.SAMP);       //start to sample
        while(!AD2CON1bits.DONE);       //wait for conversion to finish
        CLEARBIT(AD2CON1bits.DONE);     //MUST HAVE! clear conversion done bit
        return ADC2BUF0;                //return sample
}