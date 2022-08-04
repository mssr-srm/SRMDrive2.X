/*
 * File:   PWMcontrols.c
 * Author: ADMIN
 *
 * Created on August 4, 2022, 2:56 PM
 */


#include "xc.h"

void initPWM(void){
    
    /*pwm period*/
    PTPER = 1000;
    
    /*phase shift*/
    PHASE1 = 0;
//    SPHASE1 = 0;
    PHASE2 = 0;
  //  SPHASE2 = 0;
    PHASE3 = 0;
 //   SPHASE3 = 0;
    
    /*duty cycles*/
    PDC1 = 0;
  //  SDC1 = 100;
    PDC2 = 100;
    //SDC2 = 0;
    PDC3 = 100;
    //SDC3 = 0;
    
    /*set dead time values*/
    DTR1 = DTR2 = DTR3 = 0;
    ALTDTR1 = ALTDTR2 = ALTDTR3 = 0;
    
    /*set to independent*/
    IOCON1 = IOCON2 = IOCON3 = 0xC400;
    
    /*set primary time base, edge aligned, indepedent dC*/
    PWMCON1 = PWMCON2 = PWMCON3 = 0x0000;
    
    /*faults*/
    FCLCON1 = FCLCON2 = FCLCON3 = 0x0003;
    
    /*prescaler*/
    //PTCON2 = 0x0000;
    PTCON2bits.PCLKDIV = 0b010;
    //PTCON = 0x8000;
    PTCONbits.PTEN = 1;
    
}
