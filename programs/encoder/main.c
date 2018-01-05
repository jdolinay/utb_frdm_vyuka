/*
 Test rotacniho enkoderu na kitu
 Pripojen na pinech PTB2 a PTB3
 V klidu je na pin log 1 (musi byt povolen interni pull-up)
 Pri otaceni se objevuji pulsy log 0.
 Program pri detekovani 0 na pinu rozsviti zelenou RGB LED a zvysi pocitadlo na displeji.
 V kodu je mozno nastavit, ktery z obou pinu enkoderu se snima.
 */

#include "MKL25Z4.h"
#include "drv_lcd.h"
#include <stdio.h>

// Number of the pin to use, e.g. 18 is the RED LED on FRDM board.
// Note: this will only work for pins (LEDs) on port B!
#define		LED_PIN		(19)

// Piny pro enkoder
#define		ON_PIN		(2)
#define		OFF_PIN		(3)


void delay(void);
static inline int IsKeyPressed(int pin);
int count;

int main(void)
{
	// 1. Enable clock for ports
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK );

	// 2. Set pin function to GPIO
	PORTB->PCR[LED_PIN] = PORT_PCR_MUX(1);
	PORTB->PCR[ON_PIN] = PORT_PCR_MUX(1);
	PORTB->PCR[OFF_PIN] = PORT_PCR_MUX(1);

	// 3. LED as output
	PTB->PDDR |= (1 << LED_PIN);
	// switches as inputs
	PTB->PDDR &= ~(1 << ON_PIN);
	PTB->PDDR &= ~(1 << OFF_PIN);

	// 4. Enable internal pull-ups for buttons
	// PORT_PCR_PS_MASK - select pull-up and not pull down
	// PORT_PCR_PE_MASK - enables pull up/down resistors
	PORTB->PCR[ON_PIN] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);
	PORTB->PCR[OFF_PIN] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);

	// Turn LED off
	PTB->PSOR |= (1 << LED_PIN);

	LCD_initialize();
	LCD_clear();
	char buf[20];

	// 5. Wait for switch press and control LED
	while(1)
	{
		// Detekovana 0 na pinu?
		if ( IsKeyPressed(ON_PIN) ) {
			PTB->PCOR |= (1 << LED_PIN);	// turn on LED (clear pin)
			delay();
			count++;
			sprintf(buf, "%3d", count);
			LCD_set_cursor(1,1);
			LCD_puts(buf);
		}
		else
			PTB->PSOR |= (1 << LED_PIN);	// turn off LED (set pin)

	}


    /* Never leave main */
    return 0;
}

/* delay
 * busy wait
 * */
void delay(void)
{
	unsigned long n = 100000L;
	while ( n-- )
		;
}

/* Return 1 if the switch on given pin is pressed, 0 if not pressed.
 * */
static inline int IsKeyPressed(int pin)
{
	if ((PTB->PDIR & (1 << pin)) == 0)
		return 1;
	else
		return 0;
}
