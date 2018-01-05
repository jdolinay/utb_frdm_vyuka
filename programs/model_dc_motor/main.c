/*
 * Ukazka pro model DC motorku pripojeny k vyvojovemu kitu UTB.
 *
 * Tlacitky se motor zapne, vypne, zmeni smer.
 * Na displeji se vypisuji otacky.
 */

#include "MKL25Z4.h"
#include "drv_lcd.h"
#include "dc_motor.h"
#include "drv_gpio.h"
#include <stdio.h>

static int i = 0;
char buf[16];

void delay(void);

int main(void)
{
	GPIO_Initialize();
	pinMode(SW1, INPUT_PULLUP);
	pinMode(SW2, INPUT_PULLUP);
	pinMode(SW3, INPUT_PULLUP);
	pinMode(SW4, INPUT_PULLUP);
    LCD_initialize();
    LCD_clear();

    DCMOTOR_Init();
    DCMOTOR_setDirection(0);

    for (;;) {
    	// zapnout motor
        if ( pinRead(SW1) == LOW)
        	DCMOTOR_SpinON();

        // vypnout motor
        if ( pinRead(SW2) == LOW)
            DCMOTOR_SpinOFF();

        // smer
        if ( pinRead(SW3) == LOW ) {
        	// zastaveni motoru a bezpecnostni prodleva pred zmenou smeru
        	DCMOTOR_SpinOFF();
        	delay();
            DCMOTOR_setDirection(0);
        }

        // smer
        if ( pinRead(SW4) == LOW) {
        	DCMOTOR_SpinOFF();
        	delay();
        	DCMOTOR_setDirection(1);
        }

        int rpm = DCMOTOR_GetRpm();
        sprintf(buf, "%d", rpm);
        LCD_clear();
        LCD_puts(buf);
        delay();

    }

    return 0;
}

void delay(void) {
	long n = 500000;
	while ( n-- > 0 )
		;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
