/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Blikani LED s vyuzitim "vysokourovnovych" funkci
 * jako pinWrite a pinRead.
 * Tyto funkce jsou obsazeny v ovladaci drv_gpio.
 *
 * Program obsahuje chyby! Opravte je, tak aby fungoval podle zadání.
 *
 * Program má blikat LED na kitu tak, aby doba svitu byla stejná jako
 * doba kdy LED nesviti a LED blikala rychlosti asi 1x za sekundu.
 *
 * Podle vlastní odvahy si zvolte obtiznost reseni chyb pomoci #define OBTIZNOST nize.
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"

// Definujte si obtiznost:
// 0 = normal
// 1 = nightmare (nocni mura :) )
#define	OBTIZNOST  	0

#if OBTIZNOST == 0
// Prototyp funkce delay() definovane nize
void delay(void);

int main(void)
{

	gpio_initialize();

	// Nastavit pin jako vystup
	pinMode(LD2, OUTPUT);
	// Piny pro tlacitka jako vstup
	pinMode(SW1, INPUT);
	pinMode(SW2, INPUT);

	// Zapsat na pin log. 1, tim LED zhasne.
	pinWrite(LD2, HIGH);

	// Blikani LED
	while(1)
	{
		pinWrite(LD2, LOW);	// ...rozsvitime LED zapisem log. 0
		delay();
		pinWrite(LD2, HIGH);	// ...zhasneme LED zapisem log. 1
		delay;	// TODO: co se stane pri volani funkce bez zavorek?
	}


    /* Never leave main */
    return 0;
}

/* delay
 * Jednoducha cekaci funkce - busy wait
 * */
void delay(void)
{
	uint32_t cnt;
	for ( cnt = 0; cn<30000; cnt++ )
		;
}

#elif OBTIZNOST == 1
// Prototyp funkce delay() definovane nize
void delay(void);

int main(void)
{

	// Nastavit pin jako vystup
	pinMode(LD1, OUTPUT);
	// Piny pro tlacitka jako vstup
	pinMode(SW1, INPUT);
	pinMode(SW1, INPUT);

	// Zapsat na pin log. 1, tim LED zhasne.
	pinWrite(LD2, HIGH);

	for(;;)
	{
		pinWrite(LD2, LOW);		// ...rozsvitime LED zapisem log. 0
		delay();
		pinWrite(LD2, HIGH);	// ...zhasneme LED zapisem log. 1
	}

    /* Never leave main */
    return 0;
}
void delay(void)
{
	uint8_t cnt;
	for ( cnt = 0; cn<30000; cnt++ )
		;
}
#else
	#error Obtiznost neni nastavena na platnou hodnotu!
#endif



////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
