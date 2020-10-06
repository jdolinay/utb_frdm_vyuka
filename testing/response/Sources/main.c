// Ukazka odezvy programu
// Pri stisku tlacitka blikaji postupne 3 LED, pri uvolneni LED zhasnuty.
// 5.10.2020
///////////////////////////////////////////////
#include "MKL25Z4.h"
#include "drv_gpio.h"
#include "drv_systick.h"
#include "stdbool.h"

// Vyber verze kodu.
// 1. stav blikani a nesviti; kontrola tlacitka vzdy az po bliknuti vsech LED.
// 2. kontrola tlacitka po bliknuti kazde LED.
// 3. nepouziva se delay - polling casu.
// 4. rozdeleni kodu na ulohy - tasky.
#define VERSION 1

#if VERSION == 1
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
void LED_control(bool d1, bool d2, bool d3);
int main(void) {
	// inicializace ovladace pinu a delay
	GPIO_Initialize();
	SYSTICK_initialize();

	pinMode(SW1, INPUT_PULLUP);
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinWrite(LED1, HIGH);
	pinWrite(LED2, HIGH);
	pinWrite(LED3, HIGH);

	while (1) {
		if (switch1_read() == SWITCH_PRESSED)
			state = ST_EFFECT;
		else
			state = ST_OFF;

		switch (state) {
		case ST_OFF:
			LED_control(false, false, false);
			break;

		case ST_EFFECT:
			LED_control(true, false, false);
			SYSTICK_delay_ms(BLINK_DELAY);
			LED_control(false, true, false);
			SYSTICK_delay_ms(BLINK_DELAY);
			LED_control(false, false, true);
			SYSTICK_delay_ms(BLINK_DELAY);
			break;
		}	// switch

	}	// while

	/* Never leave main */
	return 0;
}

#elif VERSION == 2
#define KEY		SW1
#define LED1    LD1
#define LED2    LD2
#define LED3    LD3
#define BLINK_DELAY   1000

#define ST_LED1_ON	1
#define ST_LED2_ON	2
#define ST_LED3_ON	3
#define ST_OFF		4

#elif VERSION == 3
#define KEY		SW1
#define LED1    LD1
#define LED2    LD2
#define LED3    LD3
#define BLINK_DELAY   1000

#define ST_LED1_ON	1
#define ST_LED2_ON	2
#define ST_LED3_ON	3
#define ST_OFF		4
#define ST_LED3_WAITING		5
#define STAV_LED1_CEKA       5
#define STAV_LED2_CEKA       6
#define STAV_LED3_CEKA       7
#elif VERSION == 4
#endif

/////////////////////////////////////////////////////////
// pomocne funkce
void LED_control(bool d1, bool d2, bool d3)
{
    pinWrite(LED1, !d1);
    pinWrite(LED2, !d2);
    pinWrite(LED3, !d3);
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
    	SYSTICK_delay_ms(50);

        // znovu zkontrolovat stav tlacitka
        if ( pinRead(SW1) == LOW )
        {
            switch_state = SWITCH_PRESSED;
        }
    }
    // vratime stav tlacitka
    return switch_state;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
