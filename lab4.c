/*--------------------------------------------------------
GEORGE MASON UNIVERSITY
ECE 447 - Lab4 In-Lab Exercise
  P2.6 unused
  P2.7 Row Initialization
  P3.6 Column Clock
  P3.7 Column Done - is now also Row Clock
  P9.0 Row Data (serial, one column at a time with LSB first,
                 LSB will show as column 8 in matrix)

Date:   Fall 2020
Author: Jens-Peter Kaps

Change Log:
20200923 Initial Version, Jens-Peter Kaps
--------------------------------------------------------*/

#include <msp430.h> 

void delay(void) {
    volatile unsigned loops = 10; // Start the delay counter at 2500
    while (--loops > 0);             // Count down until the delay counter reaches 0
}


int main(void)
{
    unsigned char rowcnt;       // row counter
    unsigned char colcnt;       // column counter
    unsigned char matrix[8];    // content for LED matrix
    unsigned char row;          // data for the current row of the LED matrix
    unsigned char i;            // all purpose

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    P2DIR |= BIT7;              // Make Port 2.7 output
    P2OUT &= ~BIT7;             // Set port to 0
    P3DIR |= (BIT6 | BIT7);     // Make Port 3.6 and 3.7 outputs
    P3OUT &= ~(BIT6 | BIT7);    // Set ports to 0
    // Initialize LED Matrix column serial output
    P9DIR |= BIT0;
    P9OUT &= ~BIT0;

    PM5CTL0 &= ~LOCKLPM5;       // Unlock ports from power manager

    // Initialize matrix with test pattern
    i=2;
    for(rowcnt=0; rowcnt<8; rowcnt++){
        matrix[rowcnt]=i-1;     // i is always a power of 2
        i=i<<1;                 // multiply by 2
    }

    rowcnt = 0;                 // starting row
    colcnt = 0;                 // starting column

    while(1)                    // continuous loop
    {
        delay();
        if(P3OUT & BIT6) {              // If column clock 1
            P3OUT &= ~BIT6;             //   Set column clock 0
            // -------------- FALLING EDGE -------------------

            if(row & 0x01)              //   output one bit (column) of current row
                P9OUT |= BIT0;
            else
                P9OUT &= ~BIT0;
            row = row >>1;              //   move to next column in row

            if(colcnt == 0)             //   If column counter = 0, i.e. next falling edge after colcnt=7
                P3OUT |= BIT7;          //     Set column done 1

            if(colcnt == 7) {           //   If on last column
                rowcnt++;               //     increment row counter
                if(rowcnt == 8)
                    rowcnt = 0;
                colcnt = 0;             //     go to first column
                row = matrix[rowcnt];   //     update current row
            } else {
                colcnt++;               // increment column counter
            }
        }
        else
        {                               // If column clock 0
            P3OUT |= BIT6;              //   Set column clock 1
            // -------------- RISING EDGE -------------------

            if(colcnt == 0) {           //   If column counter = 7 + 1
                                        //   as we just incremented it on the falling edge
                if(rowcnt == 0)         //     if on row 7 + 1
                                        //     as we just incremented it on the falling edge
                    P2OUT |= BIT7;      //       Set row init 1

            }  else {                   //   on all other columns even if not needed
                                        //   this will be faster than more if-statements
                                        //   and not change the wave form
                P3OUT &= ~BIT7;         //       Set column done 0
                P2OUT &= ~BIT7;         //       Set row init 0
            }
        }
    }

	return 0;
}
