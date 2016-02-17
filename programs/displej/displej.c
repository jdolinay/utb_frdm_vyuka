/*
 * Sample program for MCU programming course
 * LDC display on the kit
 * Shows how to print characters and test string on the display
 *

 *
 */

#include "MKL25Z4.h"
#include "drv_lcd.h"

void delay(void);

int main(void)
{

	// 1. Initialize display
	LCD_initialize();

	// 2. Clear the display; cursor will be at line 1, column 1.
	LCD_clear();


	// Print 1 character
	LCD_putch('U');
	// Move cursor to the center of first line
	LCD_set_cursor(1,10);
	LCD_putch('T');
	LCD_set_cursor(1,20);
	LCD_putch('B');

	// Print text string on the second, 3rd and 4th line
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
