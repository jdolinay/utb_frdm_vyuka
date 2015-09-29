/*
 * Ukazkovy program pro Programovani mikropocitacu 
 * Ovladani LED pomoci tlacitek s vyuzitim "vysokourovnovych" funkci
 * jako pinWrite a pinRead.
   Tyto funkce jsou obsazeny v ovladaci drv_gpio.
   Stiskem SW1 se rozsviti LED LD1, stiskem SW2 se zhasne.
 *
 *
 * Mame k dispozici 4 tlacitka SW1 az SW4 na pinech:
 * A4 - SW1
 * A5 - SW2
 * A16 - SW3
 * A17 - SW4
 * Pri stisku tlacitka je z pinu ctena log. 0.
 *
 * Mame k dispozici LED na pinech:
 * (LED primo na FRDM-KL25Z)
 * B18 	- Red LED
 * B19 	- Green LED
 * D1	- Blue LED (Arduino pin D13)
 * (LED na UTB kitu)
 * B8 - LD1 cervena
 * B9 - LD2 zluta
 * B10 - LD3 zelena
 * Vsechny LED sviti, pokud je pin 0 (LOW).
 *
 * Tip: pozdeji se naucime ovladat piny primo pomoci registru.
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"

// Prototyp funkce delay() definovane nize
void delay(void);

int main(void)
{

	GPIO_Initialize();

	// Nastavit pin pro LED jako vystup
	pinMode(LD1, OUTPUT);
	// Piny pro tlacitka jako vstup
	pinMode(SW1, INPUT);
	pinMode(SW2, INPUT);

	// Zapsat na pin log. 1, tim LED zhasne.
	pinWrite(LD1, HIGH);

	// Cekame na stisk tlacitek a podle toho ovladame LED
	while(1)
	{
		// Pokud je stisknuto tlacitko SW1...
		if ( pinRead(SW1) == LOW )
			pinWrite(LD1, LOW);	// ...rozsvitime LED zapisem log. 0

		// Pokud je stisknuto tlacitko SW2...
		if ( pinRead(SW2) == LOW )
			pinWrite(LD1, HIGH);	// ...zhasneme LED zapisem log. 1

		// Chvili cekame; je zbytecne testovat tlacitka casteji nez napr. 10x za sekundu
		delay();
	}


    /* Never leave main */
    return 0;
}

/* delay
 * Jednoducha cekaci funkce - busy wait
 * */
void delay(void)
{
	unsigned long n = 200000L;
	while ( n-- )
		;
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
