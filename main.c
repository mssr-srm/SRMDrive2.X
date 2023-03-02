
#define FCY 40000000UL
/**
  Section: Included Files
*/

#include <xc.h>
#include <dsp.h>
#include <libpic30.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcc_generated_files/system.h"
#include "uart_init.h"
#include "uart_funcs.h"
#include "adc.h"
#include "SPI_setup.h"
#include "PWMcontrols.h"


unsigned int recv_position = 0;  //stores what SPI reads NOW
unsigned int ADCvalue = 0;
unsigned int ADCvalue2 = 0;
unsigned int ADCvalue3 = 0;

unsigned int PhA_Ihigh = 0;
unsigned int PhB_Ihigh = 0;
unsigned int PhC_Ihigh = 0;

unsigned int PhA_Ilow = 0;
unsigned int PhB_Ilow = 0;
unsigned int PhC_Ilow = 0;

uint16_t rotorpos = 0;

int start_read_pos = 0;

unsigned int readSPI(void);
void read_rotorpos(void);
unsigned int sampling1(void);
int PIDcontrollerA(int);
void PIDDefs(void);

int rot_max = 2047;    //this is the maximum value of the sensor 12 bits
int rot_offset = 1580;//425; //so that the unaligned position is correct
int rot_adj = 0;       //adjust zero rotor position
float angle_scale = 360.0/2047;  //scaling factor for 12bit position sensor
uint16_t rp = 0x0000;

int prev_pos = 0;
int curr_pos = 0;
float curr_spd = 0.0;
int init_pos = 0;   //should be the initial position of the rotor for speed purposes
unsigned int spd_counter = 0;
unsigned int spd_act = 0;
//signals to turn on coils
int sigA = 0;
int sigB = 0;
int sigC = 0;

int is_starting = 1;

int pwm_signal = 0;
float pid_out = 0.0;

//according to measurements taken 04/04/2022, the trendlines are:
//curr vs voltage 1: y = 0.1626x + 2.4696   R2 = 0.9995
//curr vs voltage 2: y = 0.1647x + 2.4708   R2 = 1
//curr vs bits 1:    y = 132.37x + 1997.3   R2 = 0.9995
//curr vs bits 2:    y = 134.96x + 1995.8   R2  =0.9997
//voltage vs bits 1: y = 813.93x - 12.71    R2 = 0.9992
//voltage vs bits 2: y = 819.43x - 28.813   R2 = 0.9996
//ideally, at 0 A, Vcc/2 = 2.5 V

//300 duty cycle rotates motor to:
//delay = 0, window = 80
//300 = 195 rpm
//400 = 210rpm

//del 5 window 80
//300 = 227 rpm
//400 = 250 rpm
//450 = 250rpm

//del 5 window 60
//450 = 330rpm
//500 = current limit

//delay 10 window 60
//450 = 374 rpm

//delay 10 window 50
//450 = 396 (76,77))
//450 = 483 but backwards??
//400 = 440? but weird backwards
//400 = 377 but normal
//300  = 311 (96))
//200 =  193 (153))

//delay 10 window 40
//450 = 385 rpm (77))
//300 = 275 (110))
//delay = 10 window = 30
//400 = 290 rpm

//delay = 5 window = 30
//200 = 
  
int currlim = 4000; //2667 above is 5.0A. this should be good enough for now?
void __attribute__ ((interrupt,no_auto_psv)) _T1Interrupt(void){
    //Csn pin for encoder has to be active for at least 500ns
    //with _delay32, smaller delay is possible. 173 at ~lat = lat produces 100 kHz (10000ns)
    //so 173 equals 5000 ns (5us). According to datasheet, Tclkfe (time after Csn before clock starts)
    //is 500ns, so to be safe, __delay(32) 18? But experiments say it works even without!
    //__delay32(1);
    
  
    //__delay32(1);
   
    //sigA = 1;
    //sigB = 0;
    //sigC = 0;
    //start_read_pos =1;
    if (sigA == 1){
        AD1CHS0bits.CH0SA = 0x18; //RA4 = AN24
        ADCvalue2 = sampling1();
    }
    else{
        ADCvalue2 = 20000;
    }
    if (sigB == 1){
        AD1CHS0bits.CH0SA = 0x00;//RA0= AN0
        ADCvalue3 = sampling1();
    }
    else{
        ADCvalue3 = 20000;
    }
    if (sigC == 1){
        AD1CHS0bits.CH0SA = 0x10; //RG9 = AN16
        ADCvalue = sampling1();
    }
    else{
        ADCvalue = 20000;
    }
    //AD1CHS0bits.CH0SA = 0x13; //RG9 = AN16
     //   ADCvalue = sampling1();
      //  PDC2 = ADCvalue>>2;
    //2190<x<2195 results in 1.5A
    //2265<x<2270 results in 2.0A
    //2325<x<2330, test next lower, results 2.5A?
    //2395<x<2405, 3.0A
    //2530<x<2540, 4.0A
    //2667<x<2675, 5.0A?

    /*
     * angle is scaled such that 360 =2048... divided by 8 = 256
     * so 1 bit is 0.17578125degrees. 1 degree is 5.6889 bits
     */
    //sigA = 1;
    //sigB = 1;
    //sigC = 0;
    if(is_starting == 1){
        PhA_Ihigh = 2330;
    PhB_Ihigh = 2330;
    PhC_Ihigh = 2330;
    
    PhA_Ilow = 2325;
    PhB_Ilow = 2325;
    PhC_Ilow = 2325;
        if (ADCvalue > PhC_Ihigh){
           // _LATE14 =0;// switch off sometimes!
            PDC3 = 0;
        }
        else if ((ADCvalue < PhC_Ilow) && (sigC == 1)){  //used to be 2190
            //_LATE14 = 1;
            PDC3 = 1000;
        }
        //A control
        if (ADCvalue2 > PhA_Ihigh){
            //_LATC8 = 0;
            PDC1 = 0;
        }
        else if ((ADCvalue2 < PhA_Ilow) && (sigA == 1)){
            //_LATC8 = 1;
            PDC1 = 1000;
        }
    
        //B control
        if (ADCvalue3 > PhB_Ihigh){
           // _LATC6 = 0;
            PDC2 = 0;
        }
        else if ((ADCvalue3 < PhB_Ilow) && (sigB == 1)){
           // _LATC6 = 1;
            PDC2 = 1000;
        }
    }       
    else if(is_starting == 0){
        if ((sigA == 1) ){
            PDC1 = pwm_signal;
        }
        else{
            PDC1 = 0;
        }
        //&& (ADCvalue2<currlim)
        if ((sigB == 1) ){
            PDC2 = pwm_signal;
        }
        else{
            PDC2 = 0;
        }
        if((sigC == 1)){
            PDC3 = pwm_signal;
        }
        else{
            PDC3 = 0;
        }
    }
   
    //_LATC4 = ~_LATC4;
    IFS0bits.T1IF = 0;
}

int curr_setpoint = 0;
int speed_setpoint = 75;    //rpm speed is 30000/setpoint

int error0 = 0;
int error1 = 0;
int error2 = 0;

float PIDA1 = 0.0; 
float PIDA2 = 0.0;
float PIDA3 = 0.0;
float integral = 0.0;
float derivative = 0.0;
float dt = 0.001;
float KpA = -50.0; 
float KiA = -10.0; 
float KdA = 0.05;
float pid_fout = 0.0;

void PIDDefs(){
    //PIDA1 = KpA + (KiA*dt) + (KdA/dt);
   // PIDA2 = -KpA - ((2*KdA)/dt);
    //PIDA3 = KdA/dt;
}
float trialling = 0.0;
//https://en.wikipedia.org/wiki/PID_controller#Discrete_implementation
//https://batchloaf.wordpress.com/2013/06/11/simple-pi-control-using-the-dspic30f4011/
int PIDcontrollerA(int speed_meas){
   
    error2 = error1;
    error1 = error0;
    error0 = speed_setpoint - speed_meas;
    integral = integral + ((float)error0 * dt);
    derivative = ((float)(error0 - error1))/dt;  
    //trialling = (PIDA1 * (float)error0) + (PIDA2 * (float)error1) + (PIDA3 * (float)error2);
    //pid_out = pid_out + trialling;
    pid_out = KpA*(float)error0 + KiA*integral + KdA*derivative;
    
    if(pid_out>800.0){
        pid_out = 800.0;
    }
    if(pid_out<1.0){
        pid_out = 0.0;
    }
  
    return (int)pid_out;
}

int sw_rngA = 40;     //window which signal is open; 18*5.6889 = 102.4002 Emobility values: 102, and 51 for sw_rng05
int sw_rngB = 40;
int sw_rngC = 40;
int sw_delA = 10;    //if the delay is positive, then it works backwards. E.g. delay of 2 deg, means turned on from 358 degrees and so on.
int sw_delB = 10;
int sw_delC = 10;
int sw_rng05 = 15;      //for phase C only
int sw_brd = 256;    //window of neighboring intervals; 45*5.6889
int strtA = 0;     //2.5*5.6889 =  14.2225; 
int strtB = 85;    //18*5.6889 = 102.4002; 15 = 85.3335
int strtC= 171;      //32*5.6889 =  182.0448; 30 = 170.667

int initialA = 0;
int initialB = 0;
int initialC = 0;

int togglecount = 0;
int togglecount2 = 1;

int spd_a = 0;
int spd_b = 0;

int starter = 0;


void __attribute__ ((interrupt,no_auto_psv)) _T2Interrupt(void){
    rotorpos = readSPI();  //just testing
    
    //the next if else just moves the 0 deg position somewhere else while
    //maintaining "circularity" i.e. the new 4095 is the previous 999 when the offset is 1000.
    if ((unsigned int) rotorpos >= rot_offset){
        rot_adj = (int)rotorpos - rot_offset;
    }
    else{
        rot_adj = rot_max + 1 + ((int)rotorpos - rot_offset);
     }
    spd_b = spd_a;
    spd_a = rot_adj;
    if(is_starting == 1){
        starter++;
    }
    if(starter>10000){
        is_starting = 0;//set to zero for jumpstart
    }
    if ((rot_adj > 2036) || (rot_adj < 479)){
   
        spd_counter++;
        
        if( (spd_a > (30+spd_b)) || ((spd_a+30) < spd_b) ){
            togglecount = 1;
        }
        if(spd_counter>10000){
            spd_counter = 10000;    //if the rotor is stalled
        }
    }
    else if ((rot_adj > 480) && (rot_adj<520)){
        if((togglecount == 1) && ((spd_a > (30 + spd_b)) || ((spd_a+30)>spd_b))){
            spd_act = spd_counter;
            spd_counter = 0;
            togglecount = 0;
        }
    }
    if(is_starting == 0){
        pwm_signal = PIDcontrollerA(spd_act);
    }
    if (((rot_adj - strtA + sw_delA) % sw_brd)<= sw_rngA){
        sigA = 1;
    }
    else{
        sigA = 0;
    }
    if (((rot_adj - strtB + sw_delB) % sw_brd)<= sw_rngB){
        sigB = 1;
    }
    else{
        sigB = 0;
    }
    if (((rot_adj - strtC + sw_delC) % sw_brd)<= sw_rngC){
        sigC = 1;
    }
    else{
        sigC = 0;
    }
    //_LATC4 = ~_LATC4;
    IFS0bits.T2IF = 0;
}
int temp_cntr = 0;
void __attribute__ ((interrupt,no_auto_psv)) _T3Interrupt(void){
    //_LATC4 = ~_LATC4;
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
    PR1 = 1000;
    T1CONbits.TCKPS = 0b00; //0x0000;
    
    //interrupt
    IPC0bits.T1IP = 7;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    
    //turn on
    T1CONbits.TON = 1;
}

void timer2setup(){
    
    //when TCKPS is 0b10 (1:64), 1000 is equal to approx 312.2 Hz (theo 312.5)
    //so PR2 = 312 = 1 Khz
    T2CON = 0x0000;
    TMR2 = 0x0000;
    PR2 = 312;//312 1000Khz
    T2CONbits.TCKPS = 0b10; //used to be 0b0011;
    
    //interrupts
    IPC1bits.T2IP = 7;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    
    //turn on
    T2CONbits.TON = 1;
}

//2048 bits per 360 deg, ~5.69bits/deg. 1rpm = 360deg/min = 6deg/sec = 34.13bits/sec
//since pos sensor is 2048, sampling rates faster than ~2kHz is pointless, wont detect changes.
void timer3setup(){
    T3CON = 0x0000;
    TMR3 = 0x0000;
    PR3 = 20000; //5000 is 4kHz
    T3CONbits.TCKPS = 0b00;
    
    //interrupts
    IPC2bits.T3IP = 6;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    
    //turn on
    T3CONbits.TON = 1;
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
float dummy = -89.9;
int dummy2 = 2;
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
    timer2setup();
    timer3setup();
    
    _TRISB14 = 0;
    _TRISB15 = 0;
    _TRISC4 = 0;    //LED output
    _TRISC6 = 0;    //output for B upper switch
    _TRISC7 = 0;    //output for B lower switch
    _TRISC8 = 0;    //output for A upper switch
    _TRISC9 = 0;    //output for A lower switch
    _TRISC13 = 0;   //output for Csn for position sensor
    _TRISE14 = 0;   //output for C lower switch
    _TRISE15 = 0;   //output for C upper switch
    _TRISG6 = 1;    //input for ADC
    _TRISG8 = 0;    //output for devboard pot source
    
    _LATC4 = 0;
    _LATC13 = 1;    //initially on
    
    //FOR SAFETY turn both switches off at start up.
    _LATC6 = 0;     //off
    _LATC7 = 0;     //off
    _LATC8 = 0;     //off
    _LATC9 = 0;     //off 
    _LATE14 = 0;    //off initially
    _LATE15 = 0;    //off initially
    _LATG6 = 0;
    _LATG8 = 1;     //power supply for pot
    PIDDefs();
    initPWM();
    
    INTCON1bits.NSTDIS = 0;
    initadc1();
    SPI_init();
    printf("ABCDEDF!\n");
    //can use Lat flags befor and after the motor run loop to test if timing is
    //correct!
    //LATE14 is the C lower switch! so keep it on for softswitching
    init_pos = readSPI();
    //originally in the hysteresis part of the code
    _LATC9 = 1;         //soft switching A lower switch always on
    _LATC7 = 1;         //soft switching B lower switch always on
    _LATE15 = 1;        //soft switching C lower switch always on
    while (1)
    {
       //__delay32(173);
        
        /*According to oscilloscope measurements, with simple t1 trigger to if 
         * below and nothing else, from the moment it is triggered, it takes
         * (DS00032.png) 3us. THen after the alt falling edge, it is turned off
         * after 9.19us. Chec: Pulsewidth of LATE14 is 24.8us,24.8-3-9.0 = 12.8us
         * SPI freq is 1.25 MHz, period 800ns. x16 = 12800 ns = 12.8us. Close enough.
         * DS0032 to DS0034
         */
       /*
        if(start_read_pos == 1){
            //the next if else just moves the 0 deg position somewhere else while
            //maintaining "circularity" i.e. the new 4095 is the previous 999 when the offset is 1000.
            if ((unsigned int) rotorpos >= rot_offset){
                rot_adj = (int)rotorpos - rot_offset;
            }
            else{
                rot_adj = rot_max + 1 + ((int)rotorpos - rot_offset);
            }
            start_read_pos = 0;
            //__delay_us(100);
            //_LATE14 = 0;
        }*/
       //printf("ADC:%u,%d \n", ADCvalue,(int)(ADCvalue*1.1));
       // printf("adjusted data:%f\n", (rot_adj*angle_scale)); //apparently this line takes 5ms to send, interesting
        //printf("orig data:%u\n", rot_adj);
       // printf("pos:%d, %d, %d,t:%d\n",rot_adj, spd_act, spd_counter,togglecount);
        //printf("%f,%d\n", (30000.0)/((float)spd_act), starter);
        //printf("A:%d B:%d C:%d\n",error0, error1, error2);
        printf("prevpos:%d, sig:%d\n", spd_act, ADCvalue);
      // printf("error: %dpwm:%d,%f\n", error0, pwm_signal, pid_out);
        //__delay_us(20);
    }
    return 1; 
}

uint16_t readSPI()
{
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
