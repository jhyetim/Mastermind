#include <msp430.h>
#include <stdlib.h>
#include <string.h>
#include <libemb/serial/serial.h>
#include <libemb/conio/conio.h>

#include "dtc.h"
#include "input.h"

//
// REQUIRED WIRING:
 /**************************************
 * P1.0 <- SEGF      -    SEGD -> P2.6 *
 * P1.1    UART RXD  -    SEGE -> P2.7 *
 * P1.2    UART TXD  -                 *
 * P1.3    Button    -                 *
 * P1.4 <- Trimpot   -    DIG1 -> P1.7 *
 * P1.5 <- SEGG      -    DIG4 -> P1.6 *
 * -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- *
 * P2.0 <- SEGA      -    BLUE    P2.5 *
 * P2.1    RED       -    SEGC -> P2.4 *
 * P2.2 <- SEGB      -   GREEN    P2.3 *
 **************************************/

unsigned int pot_value;
char answer_array[5] = {0};
char guess_array[4] = {0};

char number_patterns1[] = {BIT5, BIT5|BIT0, BIT0, BIT0, 0};			//pattern for 0, 1, 2, 3, 4
char number_patterns2[] = {0, BIT0|BIT6|BIT7, BIT4, BIT7, BIT0|BIT6|BIT7};	//pattern for 0, 1, 2, 3, 4

char led_patterns[] = {BIT0, BIT1, BIT3, BIT1|BIT3, BIT5, BIT3|BIT5, BIT1|BIT5, BIT1|BIT3|BIT5};

char colour;

int index = 0;

int guess_counter = 0;
int tries = 0;
int p = 0;
int l = 0;

int new_code = 1;
int button_press = 0;

int main(void) {
	// WIZARD WORDS
	WDTCTL  = WDTPW | WDTHOLD;
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;

	// INITIALIZATION
	P2SEL  &= ~(BIT6 | BIT7);	// bic
	P2DIR = 0b11111111;
	P2OUT = 0;

	P1DIR = 0b11100001;
	P1OUT = 0;

	TA0CTL   = TASSEL_2 | ID_0 | MC_1;
	TA0CCR0  = 1024;
	TA0CCTL0 = CCIE;
		
	// INITIALIZE BUTTON
	P1IE  |= BIT3;
	P1IES |= BIT3;
	P1IFG |= BIT3;
	
	// INITIALIZE SERIAL
	serial_init(9600);

	// INITIALIZE DTC
	initialize_dtc(INCH_4, &pot_value);

	// ENABLE GLOBAL INTERRUPTS
	__eint();

	for(;;) {
		// LOOP FOREVER
		if (new_code == 1) {
		     // Select code to break
		     prompt_for_line(answer_array);
		     cio_printf("\n\r? %s  P L", answer_array);
		     new_code = 0;
		     guess_counter = 0;		// Reset counter
		     tries = 0;			// Reset tries counter
		     p = 0;		 	// Reset P value
		     l = 0;			// Reset L value
	 	     button_press = 1;		// Allow button press
		}

		__delay_cycles(250000);
	}

	// NEVER GET HERE
	return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void button_isr(void)
{
	// BUTTON INTERRUPT

	// Get guess_array
	if (button_press == 1) {
	     if (guess_counter < 4) {
	          guess_array[guess_counter] = colour;
  	          guess_counter++;
	     } 
	}
	// After four colors have been entered for the code
	if (guess_counter == 4) {
	     int checkAnswer[4] = {1,1,1,1}; 
	     int checkGuess[4] = {1,1,1,1};
	     p = 0;
	     l = 0;

	     // Get P value
    	     for(int k = 0; k < 4; k++) {
       	          if(answer_array[k] == guess_array[k]) {
            	       checkAnswer[k] = checkGuess[k] = 0;
		       p++;
        	  }   
	     }         
	     // Get L value
    	     for(int i = 0; i < 4; i++) {
       	          for(int j = 0; j < 4; j++) {   
            	       if(answer_array[i] == guess_array[j] &&  checkGuess[i]  &&  checkAnswer[j]  &&  i != j) { 
                            checkAnswer[j] = checkGuess[i] = 0;
                            l++;
            	       }
		   }
	      }

	     // If statement to print guess_array, p, and l values
	     if (tries <= 9) {
	          cio_printf("\n\r%i %s  %i %i", tries, guess_array, p, l);
		  // If the player guesses the correct colors in the correct positions 
		  if (p == 4) {
		       cio_printf(" Won!\n\r");
                       tries = 0;
		       new_code = 1;
		       button_press = 0;
	          } 
		  // If the player guesses incorrectly and runs out of tries 
		  else if (tries == 9 && p != 4) {
		       cio_printf(" Lost :(\n\r");
		       tries = 0;
		       new_code = 1;
		       button_press = 0;
		  } 
		  // If the player guesses incorrectly, but still has tries
		  else {
		       cio_printf("\r");
		  }
	          tries++;
	     } 

	     guess_counter = 0;
	     
	}
	
	// DEBOUNCE ROUTINE
	while (!(BIT3 & P1IN));
	__delay_cycles(32000);
	P1IFG &= ~BIT3;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer0_isr(void)
{
	// TIMER 0 ISR

	// Set colour and display different RGB LED depending on the POT values
	if (pot_value >= 0 && pot_value <= 127) {
		P2OUT |= led_patterns[0];
		colour = 'K';
	} else if (pot_value >= 128 && pot_value <= 255) {
		P2OUT |= led_patterns[1];
		colour = 'R';
	} else if (pot_value >= 256 && pot_value <= 383) {
		P2OUT |= led_patterns[2];
		colour = 'G';
	} else if (pot_value >= 384 && pot_value <= 511) {
		P2OUT |= led_patterns[3];
		colour = 'Y';
	} else if (pot_value >= 512 && pot_value <= 639) {
		P2OUT |= led_patterns[4];
		colour = 'B';
	} else if (pot_value >= 640 && pot_value <= 767) {
		P2OUT |= led_patterns[5];
		colour = 'C';
	} else if (pot_value >= 768 && pot_value <= 895) {
		P2OUT |= led_patterns[6];
		colour = 'M';
	} else if (pot_value >= 896 && pot_value <= 1023) {
		P2OUT |= led_patterns[7];
		colour = 'W';
	}

	// Clear digits
	P1OUT &= ~(BIT7|BIT6);	// bic
	// Clear segments
	P1OUT &= ~(BIT0|BIT5);
	P2OUT &= ~(BIT0|BIT2|BIT4|BIT6|BIT7);

	if (index == 0) {
            // Configure segements
	    P1OUT = number_patterns1[p];
	    P2OUT = number_patterns2[p];
	    // Turn on digit 1
	    P1OUT |= BIT7;	// bis
	} else {
	    // Configure segements
	    P1OUT = number_patterns1[l];
	    P2OUT = number_patterns2[l];
	    // Turn on digit 4
	    P1OUT |= BIT6;	// bis
	}

	index++;
	
	index %= 2;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void timer1_isr(void)
{
	// TIMER 1 ISR
}

