// Ukazka odezvy programu
// Pri stisku tlacitka blika LED
// 5.10.2020
///////////////////////////////////////////////
#include "MKL25Z4.h"
#include "drv_gpio.h"
#include "stdbool.h"

#define SWITCH_PRESSED  	(1)
#define SWITCH_NOT_PRESSED  (0)

#define KEY		SW1
#define LED1    LD1
#define LED2    LD2
#define LED3    LD3
#define BLINK_DELAY   1000

#define ST_EFFECT	1
#define ST_OFF		2

int state = ST_OFF;

int switch1_read(void);
void delay_debounce(void);
void LED_control(ool d1, bool d2, bool d3);
int main(void) {
	// inicializace ovladace GPIO
	GPIO_Initialize();

	pinMode(SW1, INPUT_PULLUP);
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinWrite(LED1, HIGH);
	pinWrite(LED2, HIGH);
	pinWrite(LED3, HIGH);

	while (1) {
		if (switch1_read() == )
			stav = ST_EFFECT;
		else
			stav = ST_OFF;

		switch (stav) {
		case ST_OFF:
			ZapniLED(false, false, false);
			break;

		case ST_OFF:
			ZapniLED(true, false, false);
			delay(PRODLEVA);
			ZapniLED(false, true, false);
			delay(PRODLEVA);
			ZapniLED(false, false, true);
			delay(PRODLEVA);
			break;
		}	// switch

	}	// while

	/* Never leave main */
	return 0;
}


void LED_control(bool d1, bool d2, bool d3)
{
    digitalWrite(LED1, !d1);
    digitalWrite(LED2, !d2);
    digitalWrite(LED3, !d3);
}

/*
 switch1_read
 Cte stav tlacitka SW1 s osetrenim zakmitu.
 Vraci SWITCH_NOT_PRESSED pokud tlacitko neni stisknuto,
 SWITCH_PRESSED pokud je stisknuto.

 Reads and debounces switch SW1 as follows:
 1. If switch is not pressed, return SWITCH_NOT_PRESSED.
 2. If switch is pressed, wait for about 20 ms (debounce),
    then:
    if switch is no longer pressed, return SWITCH_NOT_PRESSED.
    if switch is still pressed, return SWITCH_PRESSED.
*/
int switch1_read(void)
{
    int switch_state = SWITCH_NOT_PRESSED;
    if ( pinRead(SW1) == LOW )
    {
    	// tlacitko je stisknuto

        // debounce = wait
        delay_debounce();

        // znovu zkontrolovat stav tlacitka
        if ( pinRead(SW1) == LOW )
        {
            switch_state = SWITCH_PRESSED;
        }
    }
    // vratime stav tlacitka
    return switch_state;
}


/* delay_debounce
 * Jednoducha cekaci funkce pro osetreni zakmitu tlacitka.
 * */
void delay_debounce(void)
{
	unsigned long n = 100000L;
	while ( n-- )
		;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
