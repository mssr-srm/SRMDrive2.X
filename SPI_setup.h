void SPI_init(){
/* The following code sequence shows SPI register configuration for Master mode */
 //   IFS0bits.SPI1IF = 0; // Clear the Interrupt flag
  //  IEC0bits.SPI1IE = 0; // Disable the interrupt
// SPI1CON1 Register Settings
    SPI1CON1bits.DISSCK = 0; // Internal serial clock is enabled
    SPI1CON1bits.DISSDO = 1; // SDOx unused. In this case only receive data from encoder
    SPI1CON1bits.MODE16 = 1; // Communication is word-wide (16 bits). data however is only 12bits
    SPI1CON1bits.MSTEN = 1; // Master mode enabled
    SPI1CON1bits.SMP = 0; // Input data is sampled at the middle of data output time
    SPI1CON1bits.CKE = 0; // Serial output data changes on transition from
// Idle clock state to active clock state
    SPI1CON1bits.CKP = 1; // Idle state for clock is a high level; see encoder datasheet
// active state is a high level
    /*
     * 000 8:1  00 64:1
     * 001  7   01 16:1
     * 010  6   10 4:1
     * 011  5   11 1:1
     * 100  4
     * 101  3
     * 110  2
     * 111  1
     */
    //ff results into 32:1, so SPIclk is 125 0kHz (1.25 MHz) which confirms clock is 40 MHz
    SPI1CON1bits.SPRE = 0b110;
    SPI1CON1bits.PPRE = 0b01;
    SPI1BUF = 0x0000;
    SPI1STATbits.SPIEN = 1; // Enable SPI module

// Interrupt Controller Settings
    IFS0bits.SPI1IF = 0; // Clear the Interrupt flag
    IEC0bits.SPI1IE = 0; // Enable the interrupt
}