#include <p33Fxxxx.h>

#include <stdlib.h>
#include "flexserial.h"

extern volatile uint8_t running;

void uart2_init(uint16_t baud){
    /* Stop UART port */
    CLEARBIT(U2MODEbits.UARTEN);  //Disable UART for configuration
    /* Disable Interrupts */
    IEC1bits.U2RXIE = 0;
    IEC1bits.U2TXIE = 0;
    /* Clear Interrupt flag bits */
    IFS1bits.U2RXIF = 0;
    IFS1bits.U2TXIF = 0;
    /* Set IO pins */
    TRISFbits.TRISF2 = 1;        //set as input UART2 RX pin
    TRISFbits.TRISF3 = 0;        //set as output UART2 TX pin
    /* baud rate */
    // use the following equation to compute the proper
    // setting for a specific baud rate
    U2MODEbits.BRGH = 0;                    //Set low speed baud rate
    U2BRG  = (uint32_t) 800000/ baud - 1;
    /* Operation settings and start port */
    U2MODE = 0; // 8-bit, no parity and, 1 stop bit
    U2MODEbits.RTSMD = 0;   //select simplex mode
    U2MODEbits.UEN = 0;     //select simplex mode
    U2MODE |= 0x00;
    U2MODEbits.UARTEN = 1;  //enable UART
    U2STA = 0;
    U2STAbits.UTXEN = 1;    //enable UART TX
}

int uart2_putc(uint8_t c){
    while (U2STAbits.UTXBF);
    U2TXREG = c;
    while(!U2STAbits.TRMT);
    return 0;
}

uint8_t uart2_getc(){
    running = 1;
    while(running){
        if (U2STAbits.FERR){
            U2STAbits.FERR = 0;
            continue;
        }
        if (U2STAbits.OERR) {
            U2STAbits.OERR = 0;
            continue;
        }
        if (U2STAbits.URXDA) {
            return U2RXREG & 0x00FF;
        }
    }
}