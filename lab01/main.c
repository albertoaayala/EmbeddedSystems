#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL 
#include <stdio.h>
#include <libpic30.h>

#include "lcd.h"
#include "led.h"

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT); 

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);  

void print_count(uint8_t count) {
    //Display count in fixed position as part of Objective 6
    lcd_locate(0,6);
    //Display count in hex and decimal as part of Objective 6
    lcd_printf("Count = %d\nHex = 0x%X\r", count, count);
}

void main(){
    //Init LCD
    __C30_UART=1;
    lcd_initialize();
    lcd_clear();
    lcd_locate(0,0);
    // Print group number and member names for Objective 1 of lab01
    lcd_printf("Group 3\nAlberto, Juan, Andrew\r");
    print_count(0);
    uint8_t triggerCount = 0;
    uint8_t prevCount = 0;
    uint16_t T = 0;
    uint8_t led_state=000; //state of led on startup is LED1 and LED2 off 
    //mask off bits witih BV(i) where i is led number

    //Set up LED 1,2,3 and 4 for output
    CLEARBIT(TRISAbits.TRISA4);
    CLEARBIT(TRISAbits.TRISA5);
    CLEARBIT(TRISAbits.TRISA9);
    CLEARBIT(TRISAbits.TRISA10);
    // Set LED4 to lit for Objective 2 
    SETLED(PORTAbits.RA10);
    // Set up trigger & thumb button for input
    SETBIT(AD1PCFGHbits.PCFG20);
    SETBIT(TRISEbits.TRISE8);
    SETBIT(TRISDbits.TRISD10);

    while(1){
        while((~PORTEbits.RE8 & ~led_state) & BV(0))
        {
            T++;
            if (T == 2500) {
                triggerCount ++;
                led_state |= BV(0);
                //Turn on LED1 when trigger is pressed as part of Objective 3
                SETLED(PORTAbits.RA4);
            }
        }
        T=0; 
        
        while((PORTEbits.RE8 & led_state) & BV(0))
        {
            T++;
            if(T == 2500) {
                led_state &= ~BV(0);
                //Turn off LED1 when trigger is not pressed as part of Objective 3
                CLEARLED(PORTAbits.RA4);
            }
        }
        T=0;

        while((~PORTDbits.RD10 & ~led_state >> 1) & BV(0))
        {
            T++;
            if(T == 2500) {
                led_state |= BV(1);
                //Turn on LED2 when thumb button is pressed as part of Objective 4
                SETLED(PORTAbits.RA5);
            }
        }
        T=0;

        while((PORTDbits.RD10 & led_state >> 1) & BV(0))
        {
            T++;
            if(T == 2500) {
                led_state &= ~BV(1);
                //Turn off LED2 when thumb button is not pressed as part of Objective 4
                CLEARLED(PORTAbits.RA5);
            }
        }
        T=0;
        //Turn on LED3 when LED1 and LED2 have same state for Objective 5
        PORTAbits.RA9 = ~(led_state ^ (led_state >>1));
        if(triggerCount != prevCount){
            print_count(triggerCount);
        }
        prevCount = triggerCount;
    }
}

