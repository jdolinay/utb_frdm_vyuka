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
	LCD_initialize();

	// 2. Smazeme displej. Kurzor bude na radku 1, sloupci 1.
	LCD_clear();


	// Vypis jednoho znaku
	LCD_putch('U');
	// posun kurzoru na stred prvniho radku
	LCD_set_cursor(1,10);
	LCD_putch('T');
	LCD_set_cursor(1,20);
	LCD_putch('B');

	// Na dalsi radky vypiseme texty
	LCD_set_cursor(2,1);
	LCD_puts("Ahoj svete!");

	LCD_set_cursor(3,1);
	LCD_puts("Radek 3,");

	LCD_set_cursor(4,1);
	LCD_puts("Radek 4");


	while(1)
		;

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
