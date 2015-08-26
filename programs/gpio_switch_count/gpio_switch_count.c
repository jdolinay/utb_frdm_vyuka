/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Cteni stavu tlacitka SW1 na pinu PTA4.
 * Pokud je tlacitko stisknuto, program zvetsi hodnotu promenne "counter" o 1.
 * Zastavte program v debuggeru a podivejte se na hodnotu promenne
 * "counter" - mela by odpovidat poctu stisku tlacitka.
 *
 * Reads (and debounces) switch SW1 on the pin PTA4.
 * If switch is pressed it will increment the value in variable "counter".
 * Stop the program in debugger to look at the variable and check the
 * number of switch presses.
 *
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
int switch1_readw(void);
// Inicializace pinu pro tlacitko
void switch1_init(void);
// Zpozdeni pro osetreni zakmitu
void delay_debounce(void);

// pocitadlo stisku tlacitka
int counter;

int main(void)
{
	int sw_state;

	// inicializace ovladace GPIO
	GPIO_initialize();

	// inicializace pinu pro tlacitko
	switch1_init();

	counter = 0;	// vynulovat pocitadlo stisku
	while (1)
	{
		sw_state = switch1_readw();
		// Pokud bylo tlacitko stisknuto (a uvolneno),
		// inkrementujeme pocitadlo
		if ( sw_state == SWITCH_PRESSED )
		{
			counter++;
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
 switch1_readw
 Cte stav tlacitka SW1 s osetrenim zakmitu.
 Pokud je stisknuto tlacitko, ceka na uvolneni a pak
 vrati SWITCH_PRESSED.
 Pokud neni stisknuto, vrati SWITCH_NOT_PRESSED.
*/
int switch1_readw(void)
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
        	// cekame na uvolneni tlacitka
        	while( pinRead(SW1) == LOW )
        		;
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
