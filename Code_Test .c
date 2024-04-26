#include <msp430.h> 
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


volatile bool timerFlag1 = false;
volatile int cnt;


typedef enum{LPM, PollSensors} States;
States state = LPM;

#pragma vector = TIMER0_B1_VECTOR
__interrupt void ISR_TB0_Overflow(void)
{
    cnt++; //Increases counter
    if(cnt==24){ //Checks if 6 minutes and 24 seconds has passed
        __bic_SR_register_on_exit(LPM3_bits); //Wake up the CPU from the LPM3 Mode
        timerFlag1 = true; //Turns the flag on to true
    }
    TB0CTL &= ~TBIFG; //Clears timer flag

}

 void tickFunc(){

    //state transitions
    switch(state){;
        case LPM:
           if(timerFlag1){ //Checks for the timerFlag
               cnt=0; //Resets the counter back to 0
               state = PollSensors; //Moves state to PollSensors
               timerFlag1 = false; //Clears the timerFlag
           }
           else{
               state = LPM; //Moves state to LPM
           }
            break;
        case PollSensors:
            state = LPM;  //Moves state back to LPM
            break;
        }

    //state actions
    switch(state){
         case LPM:
             P3OUT |= BIT5;  //Truns on Red LED
             P3OUT &= ~BIT2; //Turns off Blue LED
             P3OUT |= BIT1; //Turns on Green LED
             __bis_SR_register(GIE | LPM3_bits); //Turns the CPU off and puts it in LPM3 Mode
            break;
         case PollSensors:
             //Room to include turning on whichever sensors are connected
             P3OUT &= ~BIT5; //Turns off Red LED
             P3OUT |= BIT2; //Turns on Blue LED
             P3OUT &= ~BIT1; //Turns off Green LED
             __delay_cycles(500000); //Delay by 0.5 Seconds
             P3OUT &= ~BIT2;  //turns off Blue LED
             __delay_cycles(500000); //Delay by 0.5 Seconds
             P3OUT |= BIT2; //Turns on Blue LED
             __delay_cycles(500000); //Delay by 0.5 Seconds
             P3OUT &= ~BIT2; //Turns off Blue LED
            break;
    }
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop wath]dog timer
    PM5CTL0 &= ~LOCKLPM5;  // Turn on GPIO pins

    TB0CTL |= TBCLR; //Clear timer and dividers
    TB0CTL |= TBSSEL__ACLK; //Clock set at 32.768kHz
    TB0CTL |= MC__CONTINUOUS;  //Timer mode set to Continuous
    TB0CTL |= ID__8; //Divide clock frequency by 8
    TB0CTL |= TBIDEX__8; //Divide the clock frequency by another 8

    TB0CTL |= TBIE; //Enable TB0 Overflow IRQ
    TB0CTL &= ~TBIFG; //Clear TB0 flag
    __enable_interrupt(); //Enable Maskable IRQs

    P3DIR |= BIT5;//makes P3.5 an output
    P3DIR |= BIT2;//makes P3.2 an output
    P3DIR |= BIT1;//makes P3.1 an output
    P3OUT &= ~BIT5; //turns off RLED
    P3OUT &= ~BIT2; //turns off GLED
    P3OUT &= ~BIT1; //turns off BLED


    while(1){


       tickFunc(); //Runs the finite state machine Continously
    }

	return 0;
}