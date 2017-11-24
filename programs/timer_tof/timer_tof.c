/*
 * Sample program for MCU programming course
 * Timer TPM, interrupt from timer overflow (TOF)
 * Program blinks LED in the TOF interrupt service routine (ISR)
 * LED blinks once per second (0.5 s on/ 0.5 s off), so the interrupt is triggered twice per second.
 *
 * NOTE: In project properties > compiler > preprocesor must be defined: CLOCK_SETUP=1
 * so that the CPU runs at 48 MHz!
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"


int main(void)
{
	uint32_t counter;

	// using GPIO driver for LED
	GPIO_Initialize();
	pinMode(LD1, OUTPUT);
	pinWrite(LD1, HIGH);


	// Enable clock for TPM0
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;


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
	TPM0->SC = TPM_SC_CMOD(0);

	// ...wait for the change to be "acknowledged in the LPTPM clock domain"
	while (TPM0->SC & TPM_SC_CMOD_MASK )
		;

	// ... also set modulo while counter is disabled
	// For clock = 8 MHz / 128 = 62500 Hz,
	// To get 2 interrupts per second et modulo to 31250
	TPM0->CNT = 0;
	TPM0->MOD = 31250;

	// ... finally, set desired values
	TPM0->SC = ( TPM_SC_TOIE_MASK	// interrupt enabled
			| TPM_SC_TOF_MASK	// clear any pending interrupt
			| TPM_SC_CMOD(1)	// select internal clock source
			| TPM_SC_PS(7) );	// prescaler = 128


	// Enable the interrupt also in NVIC
	// ...1st clear any pending interrupt
	NVIC_ClearPendingIRQ(TPM0_IRQn);
	// ...enable interurpt from TPM0
	NVIC_EnableIRQ(TPM0_IRQn);
	// ...set interupt priority (0 is max, 3 is lowest priority)
	NVIC_SetPriority(TPM0_IRQn, 2);


	// Nothing else to do here in the main loop; everything happens in the ISR
	while(1)
	{
		counter++;
	}

	/*
	 Note: What would be the value of "counter" if we used this code in the while
	 loop instead of interrupt:
	  turn led on
	  wait
	  turn led off
	  wait
	*/

    /* Never leave main */
    return 0;
}



/* ISR for TOF interrupt.
   The name if the ISR is pre-defined.
   In your code just create function with this name and it will be called
   when the interrupt occurs.
*/
void TPM0_IRQHandler(void)
{
	// static variable keeps the value between function calls
	static uint8_t ledOn = 0;

	// If the interrupt source is TOF
	if (TPM0->SC & TPM_SC_TOF_MASK) {

		// clear TOF flag
		TPM0->SC |= TPM_SC_TOF_MASK;

		// Change LED state
		if (ledOn) {
			pinWrite(LD1, HIGH);	// off
			ledOn = 0;
		}
		else {
			pinWrite(LD1, LOW);		// on
			ledOn = 1;
		}
	}

}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
