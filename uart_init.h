void UART_initing(){
    U2MODEbits.UARTEN = 0;      //disable interrupt, reenable at end of func
    U2MODEbits.USIDL = 0 ;      //continue in idle
    U2MODEbits.IREN = 0;        //disable irda
    U2MODEbits.RTSMD = 0;       //simplex mode
    U2MODEbits.UEN = 0;         //TX, RX enabled, CTS, RTS not
    U2MODEbits.WAKE = 0;        //no wake up since no sleep
    U2MODEbits.LPBACK = 0;      //no loopback
    U2MODEbits.ABAUD = 0;       //no autobaud
    U2MODEbits.URXINV = 0;      //uxrx idle state 0
    U2MODEbits.BRGH = 0;        //standard speed mode
    U2MODEbits.PDSEL = 0;       //8bit no parity
    U2MODEbits.STSEL = 0;       // one stop bit
    
    _TRISB4 = 0;                //set RB4 as digital output
    RPOR1bits.RP36R = 0x03;     //remap UART2 transmit to RB4
    U2BRG =  250;                 //U2brg = ((Fp)/(16*baudrate))+1, Fp = Fosc/2 = 4Mhz.
                                //U2BRG = (4Mhz)/(16*9600) - 1 = 25.01
    
    U2STAbits.UTXISEL1 = 0;     //int when char is tx'ed
    U2STAbits.UTXINV = 0;       //N/a irda config
    U2STAbits.UTXISEL0 = 0;
    U2STAbits.UTXBRK = 0;       //sync break tx disabled
    U2STAbits.UTXEN = 0;        //rx disabled, rx pin controlled by port
    U2STAbits.UTXBF = 0;        //tx buffer not full;at leaset 1 word can be written
    U2STAbits.TRMT = 0;         //read only; 
    U2STAbits.URXISEL = 0;      //interrupt on char rx'ed
    U2STAbits.ADDEN = 0;        //address detect disabled
    U2STAbits.RIDLE = 0;        //readonly
    U2STAbits.PERR = 0;         //readonly
    U2STAbits.FERR = 0;         //readonly
    U2STAbits.OERR = 0;         //readonly
    U2STAbits.URXDA = 0;        //REAd only
    
   // IPC7 = 0x4400;
    IPC7bits.U2TXIP = 6;
    
    IFS1bits.U2TXIF = 0;        //clear tx interrupt flag
    IEC1bits.U2TXIE = 1;        //enable transmit interrupts
  //  IFS1bits.U2RXIF = 0;        //clear rx interrupt flag
   // IEC1bits.U2RXIE = 1;        //enable rx interrupts  
}

void UART2_start(){
    U2MODEbits.UARTEN = 1;      //turn on peripheral
    U2STAbits.UTXEN = 1;        //enable transmit
    //U2STAbits.URXEN = 1;        //enable receive 
}