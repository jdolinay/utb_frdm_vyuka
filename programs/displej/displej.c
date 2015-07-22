/*
 * Ukazkovy program pro Programovani mikropocitacu
 * LDC displej na kitu.
 * Program ukazuje vypis znaku a textoveho retezce na displej.
 *
 * Uzitecne informace:
 *
 *
 */

#include "MKL25Z4.h"
#include "drv_lcd.h"

void delay(void);

int main(void)
{

	// 1. Inicializujeme displej
	lcd_initialize();

	// 2. Smazeme displej. Kurzor bude na radku 1, sloupci 1.
	lcd_clear();

	// Vypis jednoho znaku
	lcd_putch('U');
	// posun kurzoru zhruba na stred prvniho radku
	lcd_set_cursor(1,8);
	lcd_putch('T');
	lcd_set_cursor(1,15);
	lcd_putch('B');

	// Na dalsi radky vypiseme texty
	lcd_set_cursor(2,1);
	lcd_puts("Ahoj svete!");

	lcd_set_cursor(3,1);
	lcd_puts("Radek 3,");

	lcd_set_cursor(4,1);
	lcd_puts("Radek 4");


	while(1)
	{
		lcd_backlight_on();
		delay();
		lcd_backlight_off();
		delay();
	}
    /* Never leave main */
    return 0;
}

void delay(void)
{
	uint32_t n = 700000;
	while( n -- )
		;
}





////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
