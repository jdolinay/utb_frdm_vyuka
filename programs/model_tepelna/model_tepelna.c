/*
 * Ukazka pro teplenou soustavu bez ventilatoru
*/

#include "MKL25Z4.h"
#include "smt160_kl25.h"
#include "drv_lcd.h"
#include <stdio.h>

static int i = 0;
char buff[32];

void delay(void) {
	uint32_t n;
	for ( n=0; n<1000000; n++) {
		;
	}
}
int main(void)
{
	short teplota;

	/* Inicializace ovladace snimace teploty */
	smt160_init();

	/* Inicializace pinu pouziteho pro ovladani topeni jako vystup */
	/* Enable Port C clock (pin used for output ) */
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	GPIOE_PDDR |= (1 << 31);
	PORTE_PCR31 = PORT_PCR_MUX(1); /* E31 set to function Alt1 (GPIO)*/

	LCD_initialize();
	LCD_clear();
	/* This for loop should be replaced. By default this loop allows a single stepping. */
	for (;;) {
		i++;
		GPIOE_PDOR |= (1 << 31);	// TopOn();
		teplota = smt160_get_temp();
		sprintf(buff, "%02d.%02d", teplota/100, teplota%100);
		LCD_clear();
		LCD_puts(buff);
		delay();
		delay();
		GPIOE_PDOR &= ~(1 << 31); 	// TopOff();
		delay();
		delay();
		delay();
		delay();
		//teplota = smt160_get_temp();
	}
	/* Never leave main */
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
