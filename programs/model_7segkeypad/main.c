/*
 Example program for 7segment display with keypad.

 Vypisuje na LCD kod stisknute klavesy.
 Na 7segmentovem displeji zobrazuje cisla od 0 do 9 a postupne prepina
 vsechny displeje.

 */

#include "MKL25Z4.h"
#include "drv_systick.h"
#include "drv_lcd.h"
#include "7segkeypad.h"
#include <stdio.h>

static int i = 0;
char buff[20];

int main(void)
{
	SEGKEYPAD_InitializeDisplay();
	SEGKEYPAD_InitializeKeyboard();
	SYSTICK_initialize();

	LCD_initialize();
	LCD_clear();

	int disp = 1;
	SEGKEYPAD_SelectDisplay(disp);
	int number = 0;
	int key;

    for (;;) {
    	// vypis stisknutou klavesu
    	key = SEGKEYPAD_GetKey();
    	sprintf(buff, "%d", key);
    	LCD_clear();
    	LCD_puts(buff);

    	// vypis cislo na displeji
    	SEGKEYPAD_ShowNumber(number);
    	SYSTICK_delay_ms(500);

    	// posun na dalsi cislo a dalsi display
    	number++;
    	if ( number > 9) {
    		number = 0;
    		disp++;
    		if ( disp > 8 )
    			disp = 1;
    		SEGKEYPAD_SelectDisplay(disp);
    	}

    }

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
