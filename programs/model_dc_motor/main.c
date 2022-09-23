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


char buff[20];

void delay(void);
void short_delay(void);


int main(void)
{
	int rpm;
	uint8_t pwm_duty = 0;

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
    	// Zvyseni stridy PWM
        if ( pinRead(SW1) == LOW)
        {
        	if (pwm_duty < 100) pwm_duty ++;
        }

        // Snizeni stridy PWM
        if ( pinRead(SW2) == LOW)
        {
			if (pwm_duty > 0) pwm_duty --;
		}

        // smer
        if ( pinRead(SW3) == LOW ) {
        	// zastaveni motoru a bezpecnostni prodleva pred zmenou smeru
        	DCMOTOR_setRpm(0);
        	delay();
            DCMOTOR_setDirection(0);
        }

        // smer
        if ( pinRead(SW4) == LOW) {
        	DCMOTOR_setRpm(0);
        	delay();
        	DCMOTOR_setDirection(1);
        }

        DCMOTOR_setRpm(pwm_duty);
        rpm = DCMOTOR_GetRpm();

        // Vypis na displej
        sprintf(buff, "RPM  = %4d", rpm);
        LCD_set_cursor(1,1);
        LCD_puts(buff);
        sprintf(buff, "PWMD = %4d %%", pwm_duty);
        LCD_set_cursor(2,1);
        LCD_puts(buff);

        short_delay();
    }
    return 0;
}


void delay(void) {
	long n = 10000000;
	while ( n-- > 0 )
		;
}



void short_delay(void) {
	long n = 100000;
	while ( n-- > 0 )
		;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
