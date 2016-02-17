/*
 * Sample program for MCU programming course
 * Timer TPM, generating PWM.
 * The program controls the brightness of LED using timer module .
 * Red LED on FRDM-KL25Z changes brightness from 0 to 100% using hardwarove PWM
 * of timer TPM2; frequency is 100 Hz.
 *
 * NOTE: In project properties > compiler > preprocesor must be defined: CLOCK_SETUP=1
 * so that the CPU runs at 48 MHz!
 *
 * Information
 * It is possible to control the RGB LED on FRDM-KL25Z board with hw pwm.
 * The LEDs LD1 to LD3 on the "main board" are not connected to timer channels.
 *
 * B18 	- Red LED - TPM2 channel 0 (ALT3)
 * B19 	- Green LED - TPM2 channel 1 (ALT3)
 * D1	- Blue LED - TPM0 channel 1 (ALT4)
 *
 */

#include "MKL25Z4.h"
#include "stdbool.h"

// Channel of TPM2 which generates PWM
// 0 - RED LED
// 1 - GREEN LED
#define	PWM_CHANNEL		(0)

// Pin number where PWM is generated
// NOTE: must by in sync with PWM_CHANNEL!
#define	PWM_PINNUMBER	(18)

void delay(void);

int main(void)
{
	// How many "ticks" of the counter is equal to 1 % of the pulse width.
	// Set in the code below...
	uint32_t ticksPerPercent = 1;


	// Enable clock for TPM0
	SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK;


	// Set the clock source for TPM timers (shared by all TPM modules)
	// Available clock sources depend on CLOCK_SETUP value
	// For CLOCK_SETUP = 1 or 4 you can use OSCERCLK (8 MHz)
	// For CLOCK_SETUP = 0 (default in new project) use PLLFLLCLK (20.97152 MHz)
	// Possible values:
	// 0 - clock off
	// 1 - MCGFLLCLK or MCGFLLCLK/2
	// 2 - OSCERCLK
	// 3 - MCGIRCLK  (internal generator, 32 kHz or 4 MHz)
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;	// first clear the bits
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(2);		// then write requested value


	// Set the timer
	// PS (prescale) field can be changed only when counter is disabled, i.e. SC[CMOD] = 0
	// ...first disable counter
	TPM2->SC = TPM_SC_CMOD(0);

	// ...wait for the change to be "acknowledged in the LPTPM clock domain"
	while (TPM2->SC & TPM_SC_CMOD_MASK )
		;

	//
	// ...  while counter is disabled change the settings
	//
	// Set PWM with pulses aligned to beginning of period (edge aligned).
	// The LED is on when pin is low (0), so we need low-true pulses,
	// i.e. "pulse" is value 0 on pin and delay between pulses is 1 on the pin.
	// The time to overflow is the period of the PWM signal.
	// Required period 100 Hz, so time to overflow is 0.01 s (10 ms):
	// Modulo = (0.01 * 8000000)/Prescale = 80000/8 = 10000
	// Modulo is 10 000, which is 100% of pulse width.
	// So 1% is 100 "ticks" of the timer
	TPM2->CNT = 0;
	TPM2->MOD = 10000;

	// Set the number of tick per 1% of pulse width to make the program easier to understand
	ticksPerPercent = 100;

	// Set timer mode to PWM edge aligned, low true pulses
	TPM2->CONTROLS[PWM_CHANNEL].CnSC = (TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK);

	// Set pin to PWM output (timer channel) funciton
	// ... enable clock for the port
	SIM->SCGC5 |=  SIM_SCGC5_PORTB_MASK;
	// ...set the functino number for the pin to 3 (ALT3)
	PORTB->PCR[PWM_PINNUMBER] = PORT_PCR_MUX(3);


	// Set initial duty cycle (pulse width) to 10%
	TPM2->CONTROLS[PWM_CHANNEL].CnV = 10 * ticksPerPercent;


	// Set the timer and start it by writing non-zero prescale
	// TPM_SC_PS(3): 3 is the value to set prescaler to value 8
	TPM2->SC = ( TPM_SC_CMOD(1)	| TPM_SC_PS(3) );


	// Now change the duty from 0 to 100%
	uint32_t dutyInPercent = 0;
	bool directionUp = true;
	while(1)
	{
		// Write new duty value to timer register
		TPM2->CONTROLS[PWM_CHANNEL].CnV = dutyInPercent * ticksPerPercent;

		if ( dutyInPercent == 0 ) {
			directionUp = true;
		}

		if ( dutyInPercent == 100 ) {
			directionUp = false;
		}

		if ( directionUp )
			dutyInPercent++;
		else
			dutyInPercent--;

		delay();
	}



    /* Never leave main */
    return 0;
}


void delay(void)
{
	unsigned long n = 50000L;
	while ( n-- )
		;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
