/*
 Program for controlling the EduMod model of a mixing unit. 
 */

#include "MKL25Z4.h"

// Pin names
#define	SWITCH_PIN	(4)		// A4
#define	SV1_PIN		(5)		// E5
#define	H1_PIN		(3)		// D3
#define	H2_PIN		(16)	// C16
#define	H3_PIN		(2)		// D2

static int i = 0;

// Prototype
void init(void);
static inline int IsKeyPressed(int pin);

int main(void)
{

    /* Write your code here */
	// Initialize the pins
	init();

	// Wait for the user to press switch SW1. When pressed, continue with the next step.
	while ( !IsKeyPressed(SWITCH_PIN) )
		;

	// Fill TANK1 to the level H2 (medium strong coffee).
		// Open valve SV1
	PTE->PSOR = PTE->PSOR | (1 << SV1_PIN);

	// Wait for sensor H2 to go HIGH, then close valve SV1:
	// while ( H2 is not HIGH ) do nothing;
	while ( (PTC->PDIR & (1 << H2_PIN )) == 0 )
		;
	// Close SV1
	PTE->PCOR = PTE->PCOR | (1 << SV1_PIN);


	// Fill TANK2 to H4 (big coffee – full cup).

	// Fill TANK3 to H8 (little sugar).

	// Fill the mixer (open SV4).

	// Mix everything for about 5 seconds.

	// Fill the cup (empty the mixer) – open SV5.

	// Be ready for next order – wait for button press.


    /* This for loop should be replaced. By default this loop allows a single stepping. */
    for (;;) {
        i++;
    }
    /* Never leave main */
    return 0;
}


// Initialize the pins for the Mix model
void init(void)
{
	// Enable clock for ports A, B, C, D, E
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK );

	// Set pin function to GPIO
	PORTA->PCR[SWITCH_PIN] = PORT_PCR_MUX(1);
	PORTE->PCR[SV1_PIN] = PORT_PCR_MUX(1);
	PORTD->PCR[H1_PIN] = PORT_PCR_MUX(1);
	PORTC->PCR[H2_PIN] = PORT_PCR_MUX(1);
	PORTD->PCR[H3_PIN] = PORT_PCR_MUX(1);

	// Set pin direction
	PTE->PDDR |= (1 << SV1_PIN);	// SV1 is output - valve 1

	// Hx are inputs (sensors)
	PTD->PDDR &= ~(1 << H1_PIN);
	PTC->PDDR &= ~(1 << H2_PIN);
	PTD->PDDR &= ~(1 << H3_PIN);

	// pull ups are not needed.

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
