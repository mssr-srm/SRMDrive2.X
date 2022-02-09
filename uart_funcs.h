char command[100];
int uart_receiver(){
   // printf("4");
    static int i = 0;
    
    
    if(U2STAbits.URXDA == 0){
        return 0;
    }
    
    command[i] = U2RXREG;
   // printf("3");
    if((command[i] == 10) || (command[i] == 13)){
        command[i+1] = '\0';
        printf("Rcv:%s",command);
        i=0;
        return 1;
    }
    i++;
   // printf("2");
   // printf("Waiting1...\n");
    return 0;
}

void establishContact(){
    //this function establishes contact by sending A while buffer is empty at 
    //START
//    int j = 0;
    printf("Establishing contact...\n");
    while(U2STAbits.URXDA == 0){
        //printf("A");            //
    }
    while(U2RXREG != 'A'){
        //printf("A\n");
    }
}