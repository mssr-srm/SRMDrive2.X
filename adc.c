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
    ANSELAbits.ANSA0 = 1;
    ANSELAbits.ANSA4 = 1;
    ANSELGbits.ANSG9 = 1;  //assigns RG9  ? Phase C?
    //initialize module for manual sampling manual convert
    AD1CON1 = 0x0000;
    AD1CON1bits.AD12B = 1;  //12 bit mode
    AD1CON2 = 0x0000;
    AD1CON3 = 0x0000;   //nov922 000F
    AD1CON4 = 0x0000;
    AD1CHS0bits.CH0SA = 0x13;   //corresponds to AN19, pot on devboard
   // AD1CHS0bits.CH0SA = 0x13;   //corresponds to AN19, pot on devboard
    //AD1CHS0bits.CH0SA = 0x10;   //corresponds to AN19, pot on devboard
    AD1CHS123 = 0x0000;
    AD1CSSH = 0x0000;
    AD1CSSL = 0x0000;
    AD1CON1bits.ADON = 1;
    Delay_us(2);

}