/*
 * Sample program for MCU programming course
 * Blink LED using direct access to GPIO registers.
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
 * Note: there are 2 versions of the code to illustrate options for accessing the registers.
 * To choose one version change the value of  #define VERZE_KODU below.
 */

#include "MKL25Z4.h"

/* Choose which version of code will be used:
 * 0 = "object oriented" access to port registers
 * 1 = not object oriented...  */
#define	CODE_VERSION		0

// Number of the pin to use, e.g. 18 is the RED LED on FRDM board.
// Note: this will only work for pins (LEDs) on port B!
#define		LED_PIN		(19)


void delay(void);


int main(void)
{
	// 1. Enable clock for ports A and B
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK );




	///////////// Code version A: "object oriented" access to registers
#if CODE_VERSION == 0
	// 2. Set pin function to GPIO
	PORTB->PCR[LED_PIN] = PORT_PCR_MUX(1);

	// 3. Set pin direction: output
	PTB->PDDR |= (1 << LED_PIN);

	// 4. Write 1 and 0 to the pin with delays to blink the LED.
	// To set pin high use PSOR - port set register
	// To set pin low (clear) use PCOR - port clear
	// In both registers we write 1 to perform the "action" of setting or clearing.
	while(1)
	{
		PTB->PSOR |= (1 << LED_PIN);	// pin = log. 1
		delay();
		PTB->PCOR |= (1 << LED_PIN);	// pin = log. 0
		delay();
	}


	///////////// Code version B: "not-object oriented"
#elif CODE_VERSION == 1
	// This is another way to work with the registers - using macros defined in MKL25Z4.h


	// 2. Set pin function to GPIO
	PORT_PCR_REG(PORTB, LED_PIN) = PORT_PCR_MUX(1);

	// 3. Set pin direction: output
	GPIO_PDDR_REG(PTB) |= (1 << LED_PIN);

	// 4. Write 1 and 0 to the pin with delays to blink the LED.
	while(1)
	{
		GPIO_PSOR_REG(PTB) |= (1 << LED_PIN);
		delay();
		GPIO_PCOR_REG(PTB) |= (1 << LED_PIN);
		delay();
	}
#else
	#error Neplatna hodnota VERZE_KODU!
#endif

    /* Never leave main */
    return 0;
}

/* delay
 * simple busy-wait delay
 * */
void delay(void)
{
	unsigned long n = 1000000L;
	while ( n-- )
		;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
