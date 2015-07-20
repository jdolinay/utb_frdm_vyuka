/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Mereni casu a komunikace pres seriovou linku pro zobrazeni vysledku.
 * Seriova komunikace (UART), komunikacni rychlost 9600 bd.
 *
 * POZOR: v nastaveni projektu > compiler > preprocesor musi byt CLOCK_SETUP=1
 * aby byl CPU clock 48 MHz!
 *
 *
 */


#include "MKL25Z4.h"
#include <stdio.h>	// sprintf
#include "drv_uart.h"
#include "drv_systick.h"

// LED pro pripad ladeni; pin A19 je zelena LED na FRDM-KL25Z desce.
#define		LED_PIN		(19)


int main(void)
{
	uint32_t start, end, result;
	char buf[32];

	/* LED pro pripad ladeni
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK );
	PORTB->PCR[LED_PIN] = PORT_PCR_MUX(1);
	PTB->PDDR |= (1 << LED_PIN);
	*/

	uart0_initialize(BD9600);	// BD115200
	systick_initialize();

	while ( 1 )
	{
		/*
		PTB->PSOR |= (1 << LED_PIN);	// pin = log. 1
		systick_delay_ms(1000);
		PTB->PCOR |= (1 << LED_PIN);	// pin = log. 0
		systick_delay_ms(1000);
		 */

		start = systick_millis();
		systick_delay_ms(10);
		end = systick_millis();
		result = end - start;

		sprintf(buf, "%u", result );
		uart0_puts("Vysledek: ");
		uart0_puts(buf);
		uart0_puts("\n");

		systick_delay_ms(3000);
	}

    /* Never leave main */
    return 0;
}







////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
