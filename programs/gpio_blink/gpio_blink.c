/*
 * Sample program for MCU programming course
 * Blink LED LD1.
 * The program uses high-level functions from GPIO driver (drv_gpio) such as pinWrite and pinRead.
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
 *
 * Note that the path for the driver needs to be in project settings;
 * "../../../drivers/gpio" Cross ARM C Compiler > Includes.
 *
 */
#include "MKL25Z4.h"
#include "drv_gpio.h"	//  gpio driver


// Function prototype for delay() function defined at the bottom of this file
void delay(void);

int main(void)
{
	// Initialize the gpio driver; also enable clock for ports A and B
	GPIO_Initialize();

	// Set pin as output
	pinMode(LD1, OUTPUT);

	// blink the LED
	while(1)
	{
		pinWrite(LD1, LOW);		// turn on the LED
		delay();				// wait
		pinWrite(LD1, HIGH);	// turn off
		delay();				// wait
	}

    return 0;
}

/* delay
 * Simple wait routine; busy wait
 * */
void delay(void)
{
	unsigned long n = 400000L;
	while ( n-- )
		;
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
