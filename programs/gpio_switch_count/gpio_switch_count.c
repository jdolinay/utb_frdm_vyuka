/*
 * Sample program for MCU programming course
 *
 * Reads (and debounces) switch SW1 on the pin PTA4.
 * If switch is pressed it will increment the value in variable "counter".
 * Stop the program in debugger to look at the variable and check the
 * number of switch presses.
 *
 * Uses driver drv_gpio.
 *
 * Switches are available on these pins:
 * A4 - SW1
 * A5 - SW2
 * A16 - SW3
 * A17 - SW4
 * When pressed, we read 0 from the pin.
 *
  * LED available on the board:
 * (LED directly on FRDM-KL25Z board)
 * B18 	- Red LED
 * B19 	- Green LED
 * D1	- Blue LED (Arduino pin D13)
 * (LED on the mother board)
 * B8 - LED_RED
 * B9 - LED_YELLOW
 * B10 - LED_GREEN
 * All LEDs turn on with low level on the pin.
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"

#define SWITCH_PRESSED  	(1)
#define SWITCH_NOT_PRESSED  (0)


// Read switch state. Return SWITCH_PRESSED or SWITCH_NOT_PRESSED.
int switch1_readw(void);
// Initialize pin for switch SW1
void switch1_init(void);
// Delay for debouncing
void delay_debounce(void);

// Variable to hold number of switch presses
int counter;

int main(void)
{
	int sw_state;

	// initialize GPIO driver
	GPIO_Initialize();

	// initialize pin for the switch
	switch1_init();

	counter = 0;	// reset the counter of switch presses
	while (1)
	{
		sw_state = switch1_readw();
		// If the switch was pressed (and released),
		// add 1 to the counter (increment the counter)
		if ( sw_state == SWITCH_PRESSED )
		{
			counter++;
		}
	}

    /* Never leave main */
    return 0;
}

/*
 switch1_init
 Initialization of the pin for switch SW1
*/
void switch1_init(void)
{
	// // Set the pin as input with pull up resistor enabled
	pinMode(SW1, INPUT_PULLUP);
}

/*
 switch1_readw
 Read switch SW1 state with debounce.
 Returns SWITCH_NOT_PRESSED if switch is not depressed,
 If the switch is depressed, the function wait for it to be released
 and then return SWITCH_PRESSED.
*/
int switch1_readw(void)
{
    int switch_state = SWITCH_NOT_PRESSED;
    if ( pinRead(SW1) == LOW )
    {
        // switch is pressed

        // debounce = wait
        delay_debounce();

        // check switch state again
        if ( pinRead(SW1) == LOW )
        {
        	// wait for the switch to be released
        	while( pinRead(SW1) == LOW )
        		;
            switch_state = SWITCH_PRESSED;
        }
    }
    // return the status of the switch
    return switch_state;
}


/* delay_debounce
 * Simple busy wait for debouncing.
 * */
void delay_debounce(void)
{
	unsigned long n = 200000L;
	while ( n-- )
		;
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
