/**
  @Generated PIC24 / dsPIC33 / PIC32MM MCUs Source File

  @Company:
    Microchip Technology Inc.

  @File Name:
    system.h

  @Summary:
    This is the sysetm.h file generated using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.0
        Device            :  dsPIC33EV256GM106
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.70
        MPLAB             :  MPLAB X v5.50
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

// Configuration bits: selected in the GUI

// FSEC
#pragma config BWRP = OFF    //Boot Segment Write-Protect Bit->Boot Segment may be written
#pragma config BSS = DISABLED    //Boot Segment Code-Protect Level bits->No Protection (other than BWRP)
#pragma config BSS2 = OFF    //Boot Segment Control Bit->No Boot Segment
#pragma config GWRP = OFF    //General Segment Write-Protect Bit->General Segment may be written
#pragma config GSS = DISABLED    //General Segment Code-Protect Level bits->No Protection (other than GWRP)
#pragma config CWRP = OFF    //Configuration Segment Write-Protect Bit->Configuration Segment may be written
#pragma config CSS = DISABLED    //Configuration Segment Code-Protect Level bits->No Protection (other than CWRP)
#pragma config AIVTDIS = DISABLE    //Alternate Interrupt Vector Table Disable Bit ->Disable Alternate Vector Table

// FBSLIM
#pragma config BSLIM = 8191    //Boot Segment Code Flash Page Address Limit Bits->8191

// FOSCSEL
#pragma config FNOSC = FRC    //Initial oscillator Source Selection Bits->FRC
#pragma config IESO = OFF    //Two Speed Oscillator Start-Up Bit->Start up device with user selected oscillator source

// FOSC
#pragma config POSCMD = HS    //Primary Oscillator Mode Select Bits->HS Crystal Oscillator mode
#pragma config OSCIOFNC = OFF    //OSC2 Pin I/O Function Enable Bit->OSC2 is clock output
#pragma config IOL1WAY = ON    //Peripheral Pin Select Configuration Bit->Allow Only One reconfiguration
#pragma config FCKSM = CSECMD    //Clock Switching Mode Bits->Clock Switching is enabled,Fail-safe Clock Monitor is disabled
#pragma config PLLKEN = ON    //PLL Lock Enable Bit->Clock switch to PLL source will wait until the PLL lock signal is valid

// FWDT
#pragma config WDTPOST = PS1    //Watchdog Timer Postscaler Bits->1:1
#pragma config WDTPRE = PR128    //Watchdog Timer Prescaler Bit->1:128
#pragma config FWDTEN = OFF    //Watchdog Timer Enable Bits->WDT and SWDTEN Disabled
#pragma config WINDIS = OFF    //Watchdog Timer Window Enable Bit->Watchdog timer in Non-Window Mode
#pragma config WDTWIN = WIN25    //Watchdog Window Select Bits->WDT Window is 25% of WDT period

// FPOR
#pragma config BOREN0 = ON    //Brown Out Reset Detection Bit->BOR is Enabled

// FICD
#pragma config ICS = PGD1    //ICD Communication Channel Select Bits->Communicate on PGEC1 and PGED1

// FDMTINTVL
#pragma config DMTIVTL = 0    //Lower 16 Bits of 32 Bit DMT Window Interval->0

// FDMTINTVH
#pragma config DMTIVTH = 0    //Upper 16 Bits of 32 Bit DMT Window Interval->0

// FDMTCNTL
#pragma config DMTCNTL = 0    //Lower 16 Bits of 32 Bit DMT Instruction Count Time-Out Value->0

// FDMTCNTH
#pragma config DMTCNTH = 0    //Upper 16 Bits of 32 Bit DMT Instruction Count Time-Out Value->0

// FDMT
#pragma config DMTEN = DISABLE    //Dead Man Timer Enable Bit->Dead Man Timer is Disabled and can be enabled by software

// FDEVOPT
#pragma config PWMLOCK = ON    //PWM Lock Enable Bit->Certain PWM registers may only be written after key sequence
#pragma config ALTI2C1 = OFF    //Alternate I2C1 Pins Selection Bit->I2C1 mapped to SDA1/SCL1 pins

// FALTREG
#pragma config CTXT1 = NONE    //Interrupt Priority Level (IPL) Selection Bits For Alternate Working Register Set 1->Not Assigned
#pragma config CTXT2 = NONE    //Interrupt Priority Level (IPL) Selection Bits For Alternate Working Register Set 2->Not Assigned

#include "pin_manager.h"
#include "clock.h"
#include "system.h"
#include "system_types.h"
#include "interrupt_manager.h"
#include "traps.h"

void SYSTEM_Initialize(void)
{
    PIN_MANAGER_Initialize();
    CLOCK_Initialize();
    INTERRUPT_Initialize();
    INTERRUPT_GlobalEnable();
    SYSTEM_CORCONModeOperatingSet(CORCON_MODE_PORVALUES);
}

/**
 End of File
*/