/*
 * Sample program for MCU programming course
 * Control LED by 2 buttons. Direct register access.
 * SW1 turns LED on; SW2 turns it off.
 *
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

// Number of the pin to use, e.g. 18 is the RED LED on FRDM board.
// Note: this will only work for pins (LEDs) on port B!
#define		LED_PIN		(19)

// Number of pins for switches on port A
// 4 = SW1, 5 = SW2.
#define		ON_PIN		(4)
#define		OFF_PIN		(5)


void delay(void);
static inline int IsKeyPressed(int pin);


int main(void)
{
	// 1. Enable clock for ports
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK );

	// 2. Set pin function to GPIO
	PORTB->PCR[LED_PIN] = PORT_PCR_MUX(1);
	PORTA->PCR[ON_PIN] = PORT_PCR_MUX(1);
	PORTA->PCR[OFF_PIN] = PORT_PCR_MUX(1);

	// 3. LED as output
	PTB->PDDR |= (1 << LED_PIN);
	// switches as inputs
	PTA->PDDR &= ~(1 << ON_PIN);
	PTA->PDDR &= ~(1 << OFF_PIN);

	// 4. Enable internal pull-ups for buttons
	// PORT_PCR_PS_MASK - select pull-up and not pull down
	// PORT_PCR_PE_MASK - enables pull up/down resistors
	PORTA->PCR[ON_PIN] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);
	PORTA->PCR[OFF_PIN] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);

	// Turn LED off
	PTB->PSOR |= (1 << LED_PIN);

	// 5. Wait for switch press and control LED
	while(1)
	{
		// On switch pressed?
		// if yes, there will be 0 on the pin.
		// Read value from PDIR register (Data In register)
		if ( IsKeyPressed(ON_PIN) )
			PTB->PCOR |= (1 << LED_PIN);	// turn on LED (clear pin)

		if ( (PTA->PDIR & (1 << OFF_PIN)) == 0 )
			PTB->PSOR |= (1 << LED_PIN);	// turn off LED (set pin)

		delay();
	}


    /* Never leave main */
    return 0;
}

/* delay
 * busy wait
 * */
void delay(void)
{
	unsigned long n = 30000L;
	while ( n-- )
		;
}

/* Return 1 if the switch on given pin is pressed, 0 if not pressed.
 * */
static inline int IsKeyPressed(int pin)
{
	if ((PTA->PDIR & (1 << pin)) == 0)
		return 1;
	else
		return 0;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
