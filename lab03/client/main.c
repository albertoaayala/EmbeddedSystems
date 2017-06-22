#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/types.h"
#include "../common/lcd.h"
#include "../common/lab03.h"
#include "flexserial.h"
#include "crc16.h"

volatile int fails = 0;
volatile uint8_t running = 1;
volatile char data[MSG_BYTES_MSG];

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

void reset_timer1() {
    CLEARBIT(T1CONbits.TON);
    TMR1 = 0;
}

void msg_failed(){
    uart2_putc(MSG_NACK);
    //reset_timer1();
    fails++;
    memset(&data[0], 0, sizeof(data));
}

void __attribute__((__interrupt__)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0; // clear the interrupt flag
    running = 0;
    msg_failed();
}

/*
 * 
 */
int main(int argc, char** argv) {
    __C30_UART=1;
    lcd_initialize();
    lcd_clear();
    lcd_locate(0,0);
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
    uart2_init(9600);



    // Print group number and member names for Objective 1 of lab01
    lcd_printf("Group 3\nAlberto, Juan, Andrew\r");
    uint16_t server_crc = 0, crc = 0;
    uint8_t c;
    uint8_t i = 0;
    fails = 0;
    while(1){
        lcd_locate(0, 0);
        lcd_printf("Fails: %d\n", fails);
        c = uart2_getc();
        if (c != 0) {
            msg_failed();
            continue;
        }
        server_crc = 0 | uart2_getc() << 8;
        server_crc |= uart2_getc();
        uint8_t data_length = uart2_getc();
        i = 0;
        crc = 0;
        while(running && i < data_length) {
            SETBIT(T1CONbits.TON);// Start Timer
            c = uart2_getc();
            reset_timer1();
            data[i] = c;
            crc = crc_update(crc, data[i]);
            i++;
        }
        if(i != data_length){
            msg_failed();
            continue;
        }
        if(server_crc != crc){
            msg_failed();
            continue;
        }
        uart2_putc(MSG_ACK);
        lcd_printf("server_crc:\n %d", crc);
        lcd_printf("crc: %d\n", crc);
        lcd_printf("msg: %s\n", data);

    }

    return (EXIT_SUCCESS);
}

