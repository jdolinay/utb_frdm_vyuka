/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Blikani LED.
 * Vyuziva se "vysokourovnovych" funkci jako pinWrite a pinRead.
 *  Tyto funkce jsou obsazeny v ovladaci drv_gpio.
 *
 * Mame k dispozici LED na pinech:
 * (LED primo na FRDM-KL25Z)
 * B18 	- Red LED
 * B19 	- Green LED
 * D1	- Blue LED (Arduino pin D13)
 * (LED na UTB kitu)
 * B8 - LED_RED
 * B9 - LED_YELLOW
 * B10 - LED_GREEN
 * Vsechny LED sviti, pokud je pin 0 (LOW).
 *
 * Tip: pozdeji se naucime ovladat piny primo pomoci registru.
 *
 * Pro vyuziti ovladace gpio je nutno pridat cestu
 * "../../../drivers/gpio" Cross ARM C Compiler > Includes v nastaveni projektu.
 *
 */
#include "MKL25Z4.h"
#include "drv_gpio.h"	// ovladac gpio


// Prototyp funkce delay() definovane nize
void delay(void);

int main(void)
{
	// Inicializovat ovladac gpio - povoli clock pro porty A a B
	GPIO_Initialize();

	// Nastavit pin jako vystup
	pinMode(LD1, OUTPUT);

	// blikame LED
	while(1)
	{
		pinWrite(LD1, LOW);		// rozsvitit LED
		delay();				// cekat
		pinWrite(LD1, HIGH);	// zhasnout
		delay();				// cekat
	}

    return 0;
}

/* delay
 * Jednoducha cekaci funkce - busy wait
 * */
void delay(void)
{
	unsigned long n = 400000L;
	while ( n-- )
		;
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
