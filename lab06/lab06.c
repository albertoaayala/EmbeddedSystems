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

#include "lcd.h"
#include "types.h"
#include "flextouch.h"

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

uint16_t getMedian(uint16_t Samp[5]){
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
  return Samp[3];
}

int main(int argc, char** argv) {
    __C30_UART=1;
    lcd_initialize();
    touch_init();
    lcd_clear();
    uint16_t x = 0, y = 0;
    uint16_t xMedian = 0, yMedian = 0;//, update_lcd = 0;
    lcd_locate(0,0);
    lcd_printf(" ------LAB 06------\n");
    while(1){
        uint16_t xSamples[5] = {0,0,0,0,0};
        uint16_t ySamples[5] = {0,0,0,0,0};
        touch_select_dim(0);
        while(y < 65535){
            y++;
        }
        y = 0;
        for(x = 0; x < 5; x++){
            xSamples[x] = touch_adc();
            //for(y = 0; y < 1000; y++);
            //touch_select_dim(1);
            //ySamples[x] = touch_adc();
        }
        xMedian = getMedian(xSamples);
        touch_select_dim(1);
        while(y < 32767){
            y++;
        }
        y = 0;
        for(x = 0; x < 5; x++){
            //touch_select_dim(0);
            //xSamples[x] = touch_adc();
            //for(y = 0; y < 1000; y++);
            ySamples[x] = touch_adc();
        }
        yMedian = getMedian(ySamples);
        //if(update_lcd >= 1000){
            lcd_locate(0, 1);
            lcd_printf("Ball X Position=%4d", xMedian);
            lcd_locate(0,2);
            lcd_printf("Ball Y Position=%4d", yMedian)
            //update_lcd = 0;
        //}
        //update_lcd++;
    }
    return 0;
}