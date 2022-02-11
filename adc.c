/*
 * File:   adc.c
 * Author: Soren
 *
 * Created on March 24, 2020, 7:42 PM
 */


#include "xc.h"
void Delay_us (unsigned int);
void initadc1(void){
    
    //set port config
    ANSELA = ANSELB = ANSELC = ANSELE = ANSELG = 0x0000;
    ANSELGbits.ANSG6 = 1;   //assigns RG6 as analog pin
    
    //initialize module for manual sampling manual convert
    AD1CON1 = 0x0000;
    AD1CON2 = 0x0000;
    AD1CON3 = 0x000F;
    AD1CON4 = 0x0000;
   // AD1CHS0 = 0x0013;
    AD1CHS0bits.CH0SB = 0x13; //this selects the bit, direct binary to ANx
    //AD1CHS0bits.CH0NA = 0b0;
    //AD1CHS0bits.CH0SA = 0b010011;
    AD1CHS123 = 0x0000;
    AD1CSSH = 0x0000;
    AD1CSSL = 0x0000;
    AD1CON1bits.ADON = 1;
    //Delay_us(20);

}