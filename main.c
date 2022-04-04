
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
uint16_t rotorpos = 0;

int start_read_pos = 0;

unsigned int readSPI(void);
unsigned int sampling1(void);
void read_rotorpos(void);

//according to measurements taken 04/04/2022, the trendlines are:
//curr vs voltage 1: y = 0.1626x + 2.4696   R2 = 0.9995
//curr vs voltage 2: y = 0.1647x + 2.4708   R2 = 1
//curr vs bits 1:    y = 132.37x + 1997.3   R2 = 0.9995
//curr vs bits 2:    y = 134.96x + 1995.8   R2  =0.9997
//voltage vs bits 1: y = 813.93x - 12.71    R2 = 0.9992
//voltage vs bits 2: y = 819.43x - 28.813   R2 = 0.9996
//ideally, at 0 A, Vcc/2 = 2.5 V
void __attribute__ ((interrupt,no_auto_psv)) _T1Interrupt(void){
    //Csn pin for encoder has to be active for at least 500ns
    //with _delay32, smaller delay is possible. 173 at ~lat = lat produces 100 kHz (10000ns)
    //so 173 equals 5000 ns (5us). According to datasheet, Tclkfe (time after Csn before clock starts)
    //is 500ns, so to be safe, __delay(32) 18? But experiments say it works even without!
    //_LATC13 = 1;
    //__delay32(1);
    
    //_LATC13 = 0;
    //__delay32(1);
    //SPI1BUF = 0x0000;   //starts the clock signal
     ADCvalue = sampling1();
    start_read_pos =1;
    _LATE14 = 1; //soft switching upper switch works
    _LATE15 = 1;        //soft switching lower switch always on
    _LATC4 = ~_LATC4;
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
    PR1 = 1000; //1000
    T1CONbits.TCKPS = 0x00; //0x0000;
    
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

//value of 173 at __delay32 with LAT_~LAT (and nothing else) produces 100.0 kHz
unsigned int sampling1(void){
    AD1CON1bits.SAMP = 1;
    __delay_us(2);
    AD1CON1bits.SAMP = 0;
    while (AD1CON1bits.DONE == 0){};
    return ADC1BUF0;

}

void read_rotorpos(void){
    rotorpos = readSPI();
}

/*
 The entire thing that commutates the motor *should be in a single* ISR
 * if possible, as it possibly simplifies coding and has economic use of dspic 
 * resources. However this means that the ISR time should be budgeted i.e.
 * SPI and ADC readings should fit inside the chosen frequency.
 * The logic should be: 
 * First, setup a timer and its interrupt that is twice as fast as the desired
 * switching rate of the coils. 
 * Second, determine position with that timer
 * Then, in this same interrupt routine, with that position data we can 
 * determine which phase should be active in the first place. 
 * FOr now I believe there are 6 possible choices:
 * A, AB, B, BC, C, CA and cycle repeats. Those with two letters are considers
 * the overlap given rotor position and the so-called Advance angle of switching
 * which considers the finite inductor rise time.
 * 
 * Once we know the position, we know the active phase combos, then with a
 * basic If else we can now measure the currents in those phases with the ADC.
 * Note that the ADC reading can be a bit slow, and this is not even considering
 * the timing requirements accdg to dsPIC datasheets
 * after we know where in the hysteresis band we are, we can send the swtiching 
 * info to the three active transistors (assume soft switching)
 * 
 * pseudo code
 * Timer 2 ISR {
 *  determine position
 *  ah okay phases B and C should be active given rotorpos
 * 
 *  if A...
 *  else if AB..
 * ...
 * else if BC{
 *  turn off A no matter current (inactive phase)
 *  if currB < lowerlimitB
 *      turn on B
 *  else if currB > upperlimitB
 *      turn off B
 *  else B
 *      ignore (keep status quo)
 *  [then same checks for C]
 *  
 * }
 *
 * }
 * 
 * ^all that should fit within timer set. FOr now, we choose 20 kHz timing so that
 * the frequency of coils are 10 kHz. Which means entire code should be executed
 * within 50,000 ns. SPI needs 16 clock cycles to send full data, and its clock
 * speed is 1.25 MHz. Therefor rotorpos readout with SPI takes 800 ns x 16 bits =
 * 12,800 ns. Lets say 14,000 ns with allowance.
 * 
 * So there remains 36,000 ns to determine with if-else which are the active phases
 * and to read at most 2 ADC modules (AB, BC, CA). So ideally, IF ONLY 1 of the 
 * above possible cases are chosen (no reason for otherwise, since this is only
 * a very basic control method), we have 36,000 ns to determine which, then use
 * ADC at most twice. THe "problem" here is that ADC can be a slow process. It
 * has two parts: sampling and conversion. Right now (16-02-22) conversion is about
 * 16 * Tcy = 16 * 25 = 400 ns. But currently sampling is set with DELAY of 50 us
 * waaay longer than 36000 ns budget. Need to reconsider.
 */
int main(void)
{
    __C30_UART = 2;
    // initialize the device
    SYSTEM_Initialize();
    UART_initing();
    UART2_start();
   
    
    
    timer1setup();
    
    _TRISC4 = 0;    //LED output
    _TRISC13 = 0;   //output for Csn for position sensor
    _TRISE14 = 0;   //output for C lower switch
    _TRISE15 = 0;   //output for C upper switch
    _TRISG6 = 1;    //input for ADC
    _TRISG8 = 0;    //output for devboard pot source
    
    _LATC4 = 0;
    _LATC13 = 1;    //initially on
    
    //FOR SAFETY turn both switches off at start up.
    _LATE14 = 0;    //off initially
    _LATE15 = 0;    //off initially
    _LATG6 = 0;
    _LATG8 = 1;     //power supply for pot
    
    int rot_max = 2047;    //this is the maximum value of the sensor 12 bits
    int rot_offset = 425; //so that the unaligned position is correct
    int rot_adj = 0;       //adjust zero rotor position
    float angle_scale = 360.0/rot_max;  //scaling factor for 12bit position sensor
    uint16_t rp = 0x0000;
    
    INTCON1bits.NSTDIS = 0;
     initadc1();
     SPI_init();
    printf("ABCDEDF!\n");
    //can use Lat flags befor and after the motor run loop to test if timing is
    //correct!
    
    //LATE14 is the C lower switch! so keep it on for softswitching
    //FOR SAFETY turn both switches off at start up.
    _LATE14 = 0;    //off initially
    _LATE15 = 0;    //off initially
    while (1)
    {
        //_LATE14 = ~_LATE14;
       //__delay32(173);
       
      
        //Delay_us(1000);
        
        /*According to oscilloscope measurements, with simple t1 trigger to if 
         * below and nothing else, from the moment it is triggered, it takes
         * (DS00032.png) 3us. THen after the alt falling edge, it is turned off
         * after 9.19us. Chec: Pulsewidth of LATE14 is 24.8us,24.8-3-9.0 = 12.8us
         * SPI freq is 1.25 MHz, period 800ns. x16 = 12800 ns = 12.8us. Close enough.
         * DS0032 to DS0034
         */
       
        if(start_read_pos == 1){
            
           /* rotorpos = readSPI();  //just testing
            rp = rotorpos;
            rp = (rp & 0xFF00) >> 8 | (rp & 0x00FF) << 8;
            rp = (rp & 0xF0F0) >> 4 | (rp & 0x0F0F) << 4;
            rp = (rp & 0xCCCC) >> 2 | (rp & 0x3333) << 2;
            rp = (rp & 0xAAAA) >> 1 | (rp & 0x5555) << 1;
            rp = rp >>2;
          
            //the next if else just moves the 0 deg position somewhere else while
            //maintaining "circularity" i.e. the new 4095 is the previous 999 when the offset is 1000.
            if ((unsigned int) rotorpos >= rot_offset){
                rot_adj = (int)rotorpos - rot_offset;
            }
            else{
                rot_adj = rot_max + 1 + ((int)rotorpos - rot_offset);
            }*/
            _LATE14 = 1;
           // ADCvalue = sampling1();
            //start_read_pos = 0;
            //__delay_us(100);
            _LATE14 = 0;
        }
       printf("ADC:%u \n", ADCvalue);
     //printf("%f\n", rot_adj*angle_scale); //apparently this line takes 5ms to send, interesting
        //printf("%u\n",rotorpos);
        //printf("%u\n",rp);
       //printf("%u\n", rot_adj);
        //__delay_us(20);
    }
    return 1; 
}

uint16_t readSPI(){
    //need to add Csn pulses before reading. I think this will trigger the sensor to release data
    //i think it also needs to clear the recv bit after or before reading so dspic can work
    //i think at FCY of 4M, since each cycle is 250 ns, i dont have to put a delay. The instruction cycles will cover the needed delay by the encoder
    
   //Csn pin for encoder has to be active for at least 500ns
    //with _delay32, smaller delay is possible. 173 at ~lat = lat produces 100 kHz (10000ns)
    //so 173 equals 5000 ns (5us). According to datasheet, Tclkfe (time after Csn before clock starts)
    //is 500ns, so to be safe, __delay(32) 18? But experiments say it works even without!
    _LATC13 = 1;
    __delay32(1);
    
    _LATC13 = 0;
    __delay32(1);
    //then activate clock signal after the CSnpulse but there must be some delay of 500ns
    //i dont think the delay is needed because writing to the SPI1BUF register may take at least 1 cycle...
    SPI1BUF = 0x0000;   //starts the clock signal
    //wait while the receive buffer is filled
    
    while(SPI1STATbits.SPIRBF == 0b0){}
    
    //__delay_us(1);  
   // _LATC13 = 1;
    recv_position =  SPI1BUF;    //copy only the lower twelve bits
 
    //recv_position =  0x03FF & recv_position;
   // SPI1STATbits.SPIRBF = 0b0;
    return  recv_position >> 4; //this bit shifting seems to work for the 10 bit sensor somehow
   
}
