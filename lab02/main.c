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

volatile uint16_t global_counter = 0;
volatile uint8_t toggle = 00;
volatile uint16_t milli = 0;
volatile uint8_t seconds = 0;
volatile uint16_t minutes = 0;

void print_time(uint16_t time) {
    //Display time, count, and period in a fixed position as part of Objective 6
    lcd_locate(0,5);
    //Display time, count, and period as part of Objective 6
    lcd_printf("Time: %02d:%02d:%04d\nCycles:%03d\nPeriod: %04f\r"
            , minutes, seconds, milli, time, (float)time/12800);
}

void __attribute__((__interrupt__)) _T1Interrupt(void)
{
    TOGGLEBIT(PORTAbits.RA5); //toggle LED2 every 1 second as per Objective 3
    if(seconds==60) {
        seconds = 0;
        minutes++;
    } else
        seconds++;
    IFS0bits.T1IF = 0; // clear the interrupt flag
}

void __attribute__ ((__interrupt__)) _T2Interrupt(void)
{
    if(toggle)
        TOGGLEBIT(PORTAbits.RA4); //toggle LED1 every 2ms as per Objective 2
    toggle ^= 0; //toggle boolean determining whether to toggle LED1
    if(milli==1000)
        milli = 0;
    else
        milli++;
    IFS0bits.T2IF = 0; // clear the interrupt flag
}

// no interrupt for timer 3 necessary as we only wish to read the time

void __attribute__ ((__interrupt__)) _INT1Interrupt(void)
{
    // resets timer as part of Objective 5
    milli = 0;
    seconds = 0;
    minutes = 0;
    IFS1bits.INT1IF = 0; // clear the interrupt flag
}

int main(){
    //Init LCD
    __C30_UART=1;
    lcd_initialize();
    lcd_clear();
    lcd_locate(0,0);
    // Print group number and group number for identification
    lcd_printf("Group 3\nAlberto, Juan, Andrew\r");
    print_time(0);

    //Set up LED 1,2, and 4 for output
    CLEARBIT(TRISAbits.TRISA4);
    CLEARBIT(TRISAbits.TRISA5);
    CLEARBIT(TRISAbits.TRISA10);
    // Set up trigger & thumb button for input
    SETBIT(AD1PCFGHbits.PCFG20);
    SETBIT(TRISEbits.TRISE8);

    //setup timer 1
    //enable LPOSCEN
    __builtin_write_OSCCONL(OSCCONL | 2);
    CLEARBIT(T1CONbits.TON); //Disable Timer
    SETBIT(T1CONbits.TCS); //Select external clock
    CLEARBIT(T1CONbits.TSYNC); //Disable Synchronization
    T1CONbits.TCKPS = 0b00; //Select 1:1 Prescaler
    TMR1 = 0x00; //Clear timer register
    PR1 = 32767; //Load the period value 1s
    IPC0bits.T1IP = 0x01; // Set Timer1 Interrupt Priority Level
    CLEARBIT(IFS0bits.T1IF); // Clear Timer1 Interrupt Flag
    SETBIT(IEC0bits.T1IE);// Enable Timer1 interrupt control bit
    SETBIT(T1CONbits.TON);// Start Timer

    //setup Timer 2
    CLEARBIT(T2CONbits.TON);     // Disable Timer
    CLEARBIT(T2CONbits.TCS);    // Select internal instruction cycle clock
    CLEARBIT(T2CONbits.TGATE);  // Disable Gated Timer mode
    T2CONbits.TCKPS = 0b11;     // Select 1:256 Prescaler
    TMR2 = 0x00;                // Clear timer register
    PR2 = 50;                 // Set timer period 1ms:
    // 50 = 1*10^-3 * 12.8*10^6 * 1/256
    CLEARBIT(IFS0bits.T2IF);    // Clear Timer2 interrupt status flag
    SETBIT(IEC0bits.T2IE);    // Enable Timer2 interrupt enable control bit
    SETBIT(T2CONbits.TON); /* Turn Timer 2 on */

    //setup Timer 3
    CLEARBIT(T3CONbits.TON);     // Disable Timer
    CLEARBIT(T3CONbits.TCS);    // Select internal instruction cycle clock
    CLEARBIT(T3CONbits.TGATE);  // Disable Gated Timer mode
    TMR3 = 0x00;                // Clear timer register
    T3CONbits.TCKPS = 0b00;     // Select 1:1 Prescaler
    CLEARBIT(IFS0bits.T3IF);    // Clear Timer3 interrupt status flag
    CLEARBIT(IEC0bits.T3IE);    // Disable Timer3 interrupt enable control bit
    SETBIT(T3CONbits.TON); /* Turn Timer 2 on */
    
    //setup Trigger button
    SETBIT(IEC1bits.INT1IE); //Interrupt Enable Control Register 1
    //External Interrupt 1 Enable bit
    IPC5bits.INT1IP = 01; //Interrupt Priority Control Register 5
    //External Interrupt 1 Priority bits
    CLEARBIT(IFS1bits.INT1IF); //Interrupt Flag Status Register 1

    while(1){
        global_counter++; //iterate global counter as per Objective 1
        if(global_counter==20000){ //every 20000th loop, update timer
            float timer3 = (float)TMR3;
            print_time(timer3);
            TMR3 = 0x0;
            global_counter = 0;
        }
    }
}

