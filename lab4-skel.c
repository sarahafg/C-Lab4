/*--------------------------------------------------------
GEORGE MASON UNIVERSITY
ECE 447 - Lab 4 - LED Matrix test for red and green LEDs skeleton
  Using timer ISR for multiplexing

  P2.6 unused
  P2.7 Row Initialization (CCR6)
  P3.6 Column Clock (CCR2)
  P3.7 Column Done  (CCR3) is now also Row Clock
  P9.0 Green Row Data (serial, one column at a time with LSB first,
                 LSB will show as column 8 in matrix)
  P9.1 Red Row Data (serial, one column at a time with LSB first,
                 LSB will show as column 8 in matrix)


Date:   Fall 2020
Author: Jens-Peter Kaps

Change Log:
20200928 Initial Version, Jens-Peter Kaps
--------------------------------------------------------*/

#include <msp430.h> 

unsigned char rowcnt;           // row counter
unsigned char colcnt;           // column counter
unsigned char g_matrix[8];      // content for LED matrix
unsigned char r_matrix[8];      // content for red LED matrix
unsigned char g_row;            // current row of the green LED matrix
unsigned char r_row;            // current row of the red LED matrix
/*
 * main.c
 */
int main(void) {

    unsigned int i;             // all purpose

	WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    // Add code to:
    // connect the ports P2.7, P3.6, and P3.7 to timer
    // B0 CCR6, CCR2, and CCR3 respectively

	P2DIR |= BIT7;              // Make Port 2.7 output
	P3DIR |= (BIT6 | BIT7);     // Make Port 3.6 and 3.7 outputs
	P2OUT &= ~(BIT7);           // Set port to 0
	P3OUT &= ~(BIT6 | BIT7);    // Set ports to 0
	P2SEL1 &= ~(BIT7);          // connect P2.7
	P2SEL0 |= (BIT7);           // to TB0.6
	P3SEL0 &= ~(BIT6 | BIT7);   // connect P3.6 and P3.7
	P3SEL1 |= (BIT6 | BIT7);    // to TB0.2 and TB.3


    // Add code to:
    // make the LED Matrix column serial outputs P9.0 and P9.1 outputs
    // and output a 0 on each.
	P9DIR |= (BIT0 | BIT1);     // Makes Port P9.0 and P9.1 outputs
	P9OUT &= ~(BIT0 | BIT1);    // Sets ports to 0

    PM5CTL0 &= ~LOCKLPM5;       // Unlock ports from power manager

    // CCR2 is connected to P3.6 which is column clock.
    // CCR0 active with interrupt, column clock goes low (set/RESET).
    // When CCR2 triggers (no interrupt) column clock goes high (SET/reset).
    // CCR0 and CCR2 are 4 timer clock cycles apart.
    // Add code to:
    // Initalize TB0CCR0 and TB0CCR2 such that CCR0 has an event 4 clock cyles after CCR2
    // Enable interrupts for CCR0
    // Set the outmod for CCR2
    TB0CCR0  = 8;               // CCR0 = CCR2 + 4
    TB0CCR2 = 4;                // CCR = CLK = 8
    TB0CCTL0 = CCIE;            // Enable interrupts on CCR0
    TB0CCTL2 = OUTMOD_3;        // set/reset
    __enable_interrupt();

    // Add code to:
    // Setup and start timer B in continuous mode, ALCK, clk /1, clear timer
    TB0CTL = MC_2 | ID__1 | TBSSEL_1 | TBCLR;

    // Initialize matrix with test pattern
    i=2;
    for(rowcnt=0; rowcnt<8; rowcnt++){
        g_matrix[rowcnt]=i-1;
        i=i<<1;
    }
    i=255;
    for(rowcnt=0; rowcnt<8; rowcnt++){
        r_matrix[rowcnt]=i;
        i=i<<1;
    }

    rowcnt = 0;                 // starting row
    colcnt = 0;                 // starting column


    __enable_interrupt();

    while(1)                    // continuous loop
    {
        __low_power_mode_3();
    }

	return 0;
}

// Interrupt Service Routine for Timer B channel CCR0,
// active on falling edge on column clock
#pragma vector = TIMER0_B0_VECTOR   // associate funct. w/ interrupt vector
__interrupt void T0B0_ISR(void)     // name of ISR (can be anything)
{
    // Add code to:
    // output one bit (column) of the green and of the red row
    // and then shift them to move to the next column
    // Add code to:
    // create timer events for CCR0 and CCR2.
    // both 8 clock cycles from the last one
    if (BIT0 & g_row) {             // output one bit (column) of the green row
        P9OUT |= BIT0;
    }
    else {
        P9OUT &= ~(BIT0);
    }

    if (BIT0 & r_row) {             // output one bit (column) of the red row
        P9OUT |= BIT1;
    }
    else {
        P9OUT &= ~(BIT1);
    }
    g_row = g_row >>1;              // shift to next column in the green row
    r_row = r_row >>1;              // shift to next column in the red row
    TB0CCR0  += 8;                  // getting the next rising edge, TB0CCR0 has to be incremented by 8
    TB0CCR2  += 8;                  // getting the next falling edge, TB0CCR2 has to be incremented by 8

    if(colcnt == 7) {               // When on last column of matrix

        // Add code to:
        // create events for column_done and row_init based upon the
        // specifications in the lab manual
        // Add code to:
        // increment the row counter and set the column counter back to 0
        // Add code to:
        // update the current row for red and green
        TB0CCTL3 = OUTMOD_1;        // set
        TB0CCR3 = TB0CCR0;          // next event on CCR3 will occur at same time as next event on CCR0
        if (rowcnt == 7) {          // When on last row of matrix
            TB0CCR6 = TB0CCR2;      // next event on CCR6 will occur at same time as next event on CCR2
            TB0CCTL6 = OUTMOD_1;    // set
        }
        rowcnt++;                   // incrementing row counter
        if(rowcnt == 8) {           // checking if out of matrix scope
            rowcnt = 0;             // resetting row counter
        }
        colcnt = 0;                 // go to first column, i.e. next falling edge after colcnt=7
        g_row = g_matrix[rowcnt];   // updating matrix here
        r_row = r_matrix[rowcnt];   // updating matrix here
        }
    else {
        // Add code to:
        // create events for column_done and row_init based upon the
        // specifications in the lab manual
        // Add code to:
        // increment the column counter
        TB0CCR3 = TB0CCR2;          // next event on CCR3 will occur at same time as next event on CCR2
        TB0CCTL3 = OUTMOD_5;        // reset
        TB0CCTL6 = OUTMOD_5;        // reset
        TB0CCR6 = TB0CCR2;          // next event on CCR6 will occur at same time as next event on CCR2
        colcnt++;                   // incrementing column counter
    }

}


