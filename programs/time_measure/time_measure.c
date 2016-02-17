/*
 * Sample program for MCU programming course
 * Measure time and communicate over serial line (UART) to display desults.
 * Baudrate: 9600
 *
 * NOTE: In project properties > compiler > preprocesor must be defined: CLOCK_SETUP=1
 * so that the CPU runs at 48 MHz!
 *
 *
 */


#include "MKL25Z4.h"
#include <stdio.h>	// sprintf
#include "drv_uart.h"
#include "drv_systick.h"

// LED for debugging; pin A19 is Green LED on FRDM-KL25Z
#define		LED_PIN		(19)


int main(void)
{
	uint32_t start, end, result;
	char buf[32];

	/* LED for debugging
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK );
	PORTB->PCR[LED_PIN] = PORT_PCR_MUX(1);
	PTB->PDDR |= (1 << LED_PIN);
	*/

	UART0_Initialize(BD9600);	// BD115200
	SYSTICK_initialize();

	while ( 1 )
	{
		/*
		PTB->PSOR |= (1 << LED_PIN);	// pin = log. 1
		systick_delay_ms(1000);
		PTB->PCOR |= (1 << LED_PIN);	// pin = log. 0
		systick_delay_ms(1000);
		 */

		start = SYSTICK_millis();
		SYSTICK_delay_ms(10);
		end = SYSTICK_millis();
		result = end - start;

		sprintf(buf, "%u", result );
		UART0_puts("Result: ");
		UART0_puts(buf);
		UART0_puts("\n");

		SYSTICK_delay_ms(3000);
	}

    /* Never leave main */
    return 0;
}







////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
