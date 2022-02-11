
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


void __attribute__ ((interrupt,no_auto_psv)) _U2TXInterrupt(void){

    IFS1bits.U2TXIF = 0;    //reset interrupt flag
}

void timer1setup(){
    T1CON = 0x0000;
    TMR1 = 0x0000;
    PR1 = 1000;
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
int main(void)
{
    __C30_UART = 2;
    // initialize the device
    SYSTEM_Initialize();
    UART_initing();
    UART2_start();
   
   // SPI_init();
    
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
     
    printf("ABCDEDF!\n");
    while (1)
    {
      //  _LATE14 = ~_LATE14;
      //  __delay_us(476);
       
       // AD1CON1bits.SAMP = 1;
      //  __delay_us(50);
      //  AD1CON1bits.SAMP = 0;
     //   while (AD1CON1bits.DONE == 0){};
        ADCvalue = sampling1();
        //Delay_us(1000);
        printf("ADC:%u \n", ADCvalue);
        
    }
    return 1; 
}

unsigned int readSPI(){
    //need to add Csn pulses before reading. I think this will trigger the sensor to release data
    //i think it also needs to clear the recv bit after or before reading so dspic can work
    //i think at FCY of 4M, since each cycle is 250 ns, i dont have to put a delay. The instruction cycles will cover the needed delay by the encoder
    
   //Csn pin for encoder has to be active for at least 500ns
    _LATC13 = 1;

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
