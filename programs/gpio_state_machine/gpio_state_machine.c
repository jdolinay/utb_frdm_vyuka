/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Blikani LED ovladane pomoci tlacitka - stavovy automat.
 * Vyuziva "vysokourovnovych" funkci jako pinWrite a pinRead.
 * Tyto funkce jsou obsazeny v ovladaci drv_gpio.
 *
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"

// Prototypy funkci definovanych nize
void delay(void);
void led_update_state(void);

// Stavy, ve kterych se muze nachazet LED
typedef enum { LED_VYPNUTA, LED_BLIKA, } LED_state;

LED_state stav_led = LED_VYPNUTA;

int main(void)
{

	GPIO_initialize();

	// Nastavit pin pro LED jako vystup
	pinMode(LD3, OUTPUT);
	// Piny pro tlacitka jako vstup
	pinMode(SW1, INPUT);
	pinMode(SW2, INPUT);

	// Zapsat na pin log. 1, tim LED zhasne.
	pinWrite(LD3, HIGH);

	// Cekame na stisk tlacitek a podle toho ovladame LED
	while(1)
	{
		// Testujeme tlacitka a podle toho aktualizujeme stav programu
		led_update_state();

		// Podle aktualniho stavu provedeme prislusnou akci
		switch (stav_led)
		{
		case LED_VYPNUTA:
			pinWrite(LD3, HIGH);	// zhasneme LED (i opakovane)
			delay();	// pockame chvili, at netestujeme stisk tlacitka zbytecne casto
			break;

		case LED_BLIKA:
			pinWrite(LD3, LOW);	// rozsvitit
			delay();
			pinWrite(LD3, HIGH);	// zhasnout
			delay();
			break;
		}

		// TODO: kod pro stav BLIKA je sice takto srozumitelny, ale dlouho trva
		// nez se znovu otestuje tlacitko. Zkuste kratce stisknout SW2; nekdy se
		// blikani nezastavi...
		// Dokazete program upravit tak, aby se tlacitko testovalo casteji?
	}


    /* Never leave main */
    return 0;
}

// Aktualizuje stav LED
void led_update_state(void)
{
	// Pokud je stisknuto tlacitko, menime stav
	// Pokud je stisknuto SW1...
	if ( pinRead(SW1) == LOW )
		stav_led = LED_BLIKA;	// ...stav bude blikani

	// Pokud je stisknuto tlacitko SW2...
	if ( pinRead(SW2) == LOW )
		stav_led = LED_VYPNUTA;	// ...stav bude zhasnuto
}

/* delay
 * Jednoducha cekaci funkce - busy wait
 * */
void delay(void)
{
	unsigned long n = 700000L;
	while ( n-- )
		;
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
