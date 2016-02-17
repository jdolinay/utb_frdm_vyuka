/*
 * Sample program for MCU programming course
 * Use timer module TPM for precise timed delay (polling TPM register).
 * The program blinks a LED with use of the exact delay function. Uses TPM0 module.
 * LED is on for 1 second and off for 1 s.
 *
 * NOTE: In project properties > compiler > preprocesor must be defined: CLOCK_SETUP=1
 * so that the CPU runs at 48 MHz!
 *
 */

#include "MKL25Z4.h"

// gpio driver is used to control the LED
#include "drv_gpio.h"

// Function prototypes
void TPMDelay1s(void);
void TPMInitialize(void);


int main(void)
{

	GPIO_Initialize();
	pinMode(LD1, OUTPUT);
	pinWrite(LD1, HIGH);

	// Blink the LED
	while(1) {
		pinWrite(LD1, LOW);
		TPMDelay1s();
		pinWrite(LD1, HIGH);
		TPMDelay1s();
	}


    /* Never leave main */
    return 0;
}

/*
 Wait for 1 second using TPM0 timer.
 */
void TPMDelay1s(void)
{
	// Initialize the timer
	TPMInitialize();

	// Wait for timer overflow flag (TOF)
	while ( !(TPM0->SC & TPM_SC_TOF_MASK) )
		;

	// Clear the TOF flag by writing 1 to TOF bit
	TPM0->SC |= TPM_SC_TOF_MASK;

}

/*
 Initialize timer TPM0 for the use in function TPMDelay1s.
 Do not call this funciton! It is called internally by TPMDelay1s().
 */
void TPMInitialize(void)
{
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
	while (TPM0->SC & TPM_SC_CMOD_MASK)
		;

	// ... also set modulo while counter is disabled
	// For clock = 8 MHz / 128 = 62500 Hz,
	// the counter counts to 62500 in 1 second.
	// For 1 overflow every second we need the counter to overflow at value 62500
	TPM0->CNT = 0;	// manual recommends to clear the counter
	TPM0->MOD = 62500;

	// ... finally, set desired values
	// ... prescale = 128
	// ... internal clock source
	TPM0->SC = ( TPM_SC_CMOD(1) | TPM_SC_PS(7));

}








////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
