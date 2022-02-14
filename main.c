
#define FCY 40000000UL
/**
  Section: Included Files
*/

#include <xc.h>
#include <dsp.h>
#include <libpic30.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcc_generated_files/system.h"
#include "uart_init.h"
#include "uart_funcs.h"
#include "adc.h"
#include "SPI_setup.h"



/*
                         Main application
 */

unsigned int recv_position = 0;  //stores what SPI reads NOW
unsigned int ADCvalue = 0;
void __attribute__ ((interrupt,no_auto_psv)) _T1Interrupt(void){
    
    _LATE15 = ~_LATE15;
    IFS0bits.T1IF = 0;
}

void __attribute__ ((interrupt,no_auto_psv)) _T2Interrupt(void){
    
    
    IFS0bits.T2IF = 0;
}

void __attribute__ ((interrupt,no_auto_psv)) _T3Interrupt(void){
    
    IFS0bits.T3IF = 0;
}

void __attribute__ ((interrupt,no_auto_psv)) _T4Interrupt(void){
  
    IFS1bits.T4IF = 0;
}

void __attribute__ ((interrupt,no_auto_psv)) _T5Interrupt(void){
    
   
    IFS1bits.T5IF = 0;
}

void __attribute__ ((interrupt,no_auto_psv)) _U2TXInterrupt(void){

    IFS1bits.U2TXIF = 0;    //reset interrupt flag
}

void timer1setup(){
    T1CON = 0x0000;
    TMR1 = 0x0000;
    PR1 = 2000;
    T1CONbits.TCKPS = 0x0000;
    
    //interrupt
    IPC0bits.T1IP = 7;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    
    //turn on
    T1CONbits.TON = 1;
}

void timer2setup(){
    T2CON = 0x0000;
    TMR2 = 0x0000;
    PR2 = 15625;
    T2CONbits.TCKPS = 0b0011;
    
    //interrupts
    IPC1bits.T2IP = 7;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    
    //turn on
    T2CONbits.TON;
}

void timer3setup(){
    T3CON = 0x0000;
    TMR3 = 0x0000;
    PR3 = 15625;
    T3CONbits.TCKPS = 0b0011;
    
    //interrupts
    IPC2bits.T3IP = 7;
    IFS0bits.T3IF = 0;
    IEC0bits.T2IE = 1;
    
    //turn on
    T3CONbits.TON;
}

void timer4setup(){
    T4CON = 0x0000;
    TMR4 = 0x0000;
    PR4 = 15625;
    T4CONbits.TCKPS = 0b0011;
    
    //interrupts
    IPC6bits.T4IP = 7;
    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 1;
    
    //turn on
    T4CONbits.TON;
}

void timer5setup(){
    T5CON = 0x0000;
    TMR5 = 0x0000;
    PR5 = 15625;
    T5CONbits.TCKPS = 0b0011;
    
    //interrupts
    IPC7bits.T5IP = 7;
    IFS1bits.T5IF = 0;
    IEC1bits.T5IE = 1;
    
    //turn on
    T5CONbits.TON;
}


void Delay_us(unsigned int delay){
    int i =0;
    for ( i = 0; i < delay; i++){
        __asm__ volatile ("repeat #39");
        __asm__ volatile ("nop");
    }
}

//value of 476 at __delay_us with LAT = ~LAT produces 9.998 kHz
//that is at 4000000UL fcy, but at 40,000,000 UL, 1.008 kHz

unsigned int sampling1(void){
    AD1CON1bits.SAMP = 1;
    __delay_us(50);
    AD1CON1bits.SAMP = 0;
    while (AD1CON1bits.DONE == 0){};
    return ADC1BUF0;
}

unsigned int readSPI(void);
int main(void)
{
    __C30_UART = 2;
    // initialize the device
    SYSTEM_Initialize();
    UART_initing();
    UART2_start();
   
   // SPI_init();
    unsigned int rotorpos = 0;
    
    timer1setup();
    
    _TRISC13 = 0;   //output for Csn for position sensor
    _TRISE14 = 0;   //output for C lower switch
    _TRISE15 = 0;   //output for C upper switch
    _TRISG6 = 1;    //input for ADC
    _TRISG8 = 0;    //output for devboard pot source
    
    _LATC13 = 1;    //initially on
    _LATE14 = 1;    //on initially
    _LATE15 = 0;    //off initially
    _LATG6 = 0;
    _LATG8 = 1;     //power supply for pot
     initadc1();
     SPI_init();
    printf("ABCDEDF!\n");
    while (1)
    {
      //  _LATE14 = ~_LATE14;
      //  __delay_us(476);
       
       // AD1CON1bits.SAMP = 1;
      //  __delay_us(50);
      //  AD1CON1bits.SAMP = 0;
     //   while (AD1CON1bits.DONE == 0){};
        //ADCvalue = sampling1();
        //Delay_us(1000);
       // printf("ADC:%u \n", ADCvalue);
        rotorpos = readSPI();  //just testing
        printf("%u\n", rotorpos);
        __delay_us(5000);
      __delay_us(5000);
    }
    return 1; 
}

unsigned int readSPI(){
    //need to add Csn pulses before reading. I think this will trigger the sensor to release data
    //i think it also needs to clear the recv bit after or before reading so dspic can work
    //i think at FCY of 4M, since each cycle is 250 ns, i dont have to put a delay. The instruction cycles will cover the needed delay by the encoder
    
   //Csn pin for encoder has to be active for at least 500ns
    _LATC13 = 1;
    __delay_us(1); //greater than 500 ns?
    _LATC13 = 0;
     __delay_us(1);       
    //then activate clock signal after the CSnpulse but there must be some delay of 500ns
    //i dont think the delay is needed because writing to the SPI1BUF register may take at least 1 cycle...
    SPI1BUF = 0x0000;   //starts the clock signal
    //wait while the receive buffer is filled
    while(SPI1STATbits.SPIRBF == 0b0){}
    __delay_us(1);  
   // _LATC13 = 1;
    recv_position =  SPI1BUF;    //copy only the lower twelve bits
    recv_position =  0x0FFF & recv_position;
   // SPI1STATbits.SPIRBF = 0b0;
   
    return  recv_position;
   
}
