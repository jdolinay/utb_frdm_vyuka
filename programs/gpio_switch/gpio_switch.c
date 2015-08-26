/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Cteni stavu tlacitka SW1 na pinu PTA4.
 * Pokud je tlacitko stisknuto, rozsviti LED LD1 (na pinu PTB8).
 * Pro praci s piny jsou pouzivany "vysokourovnove" funkce
 * jako pinWrite a pinRead.
 * Tyto funkce jsou obsazeny v ovladaci drv_gpio.
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
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"

#define SWITCH_PRESSED  	(1)
#define SWITCH_NOT_PRESSED  (0)


// Cteni stavu tlacitka. Vraci SWITCH_PRESSED nebo SWITCH_NOT_PRESSED
int switch1_read(void);
// Inicializace pinu pro tlacitko
void switch1_init(void);
// Zpozdeni pro osetreni zakmitu
void delay_debounce(void);


int main(void)
{
	int sw_state;

	// inicializace ovladace GPIO
	GPIO_initialize();

	// inicializace pinu pro tlacitko
	switch1_init();

	// inicializace pinu pro LED
	pinMode(LD1, OUTPUT);	// Nastavit pin pro LED jako vystup

	while (1)
	{
		sw_state = switch1_read();

		// display the status of the switch using LED1
		if (sw_state == SWITCH_PRESSED)
		{
			pinWrite(LD1, LOW);	// ...rozsvitime LED zapisem log. 0
		}
		else
		{
			pinWrite(LD1, HIGH);	// ...zhasneme LED zapisem log. 1
		}
	}

    /* Never leave main */
    return 0;
}

/*
 switch1_init
 Initialization of the pin for switch SW1
*/
void switch1_init(void)
{
	// Nastavit pin pro SW1 jako vstup s povolenym pull-up rezistorem
	pinMode(SW1, INPUT_PULLUP);
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
	unsigned long n = 200000L;
	while ( n-- )
		;
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
