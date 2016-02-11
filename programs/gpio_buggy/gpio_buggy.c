/*
 * Sample program for MCU programming course
 * The program uses high-level functions from GPIO driver (drv_gpio) such as pinWrite and pinRead.
 *
 *
 * THERE ARE ERRORS IN THE PROGRAM!!!
 * Fix the program so that it works as defined below.
 *
 * The LED should be blinking about once per second with the ON time
 * the same as OFF time.
 *
 * You can set the difficulty of the task for yourself below using the #define	DIFFICULTY value 0 or 1.
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"

// Define the difficulty:
// 0 = normal
// 1 = nightmare
#define	DIFFICULTY 	0


//////////////////////////////////////////////////////////////////////////

/* difficulty 0 = easy */
#if DIFFICULTY == 0


void delay(void);

int main(void)
{

	GPIO_Initialize();

	// Set pin to output mode
	pinMode(LD2, OUTPUT);


	// Write log 1 to the pin; to turn off LED
	pinWrite(LD2, HIGH);

	// blink the LED LED
	while(1)
	{
		pinWrite(LD2, LOW);		// ...turn on by writing 0
		delay();
		pinWrite(LD2, HIGH);	// ...turn off by writing 1
	}


    /* Never leave main */
    return 0;
}

/* delay
 * Simple busy wait
 * */
void delay(void)
{
	uint32_t cnt;
	for ( cnt = 0; cn<500000L; cnt++ )
		;
}

/* difficulty 1 = high (hard) */
#elif DIFFICULTY == 1

void delay(void);

int main(void)
{

	// Set pin to output mode
	pinMode(LD1, OUTPUT);

	// Write log 1 to the pin; to turn off LED
	pinWrite(LD2, HIGH);

	for(;;)
	{
		pinWrite(LD2, LOW);		// ...turn on by writing 0
		delay();
		pinWrite(LD2, HIGH);	// ...turn off by writing 1
		delay;
	}

    /* Never leave main */
    return 0;
}
void delay(void)
{
	uint8_t cnt;
	for ( cnt = 0; cn<50000; cnt++ )
		;
}
#else
	#error There is no valid value for DIFFICULTY!
#endif



////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
