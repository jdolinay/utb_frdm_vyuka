/*
 * Sample program for MCU programming course
 *
 * Reads (and debounces) switch SW1 on the pin PTA4.
 * If switch is pressed it will increment the value in variable "counter".
 * Stop the program in debugger to look at the variable and check the
 * number of switch presses.
 *
 * Uses driver drv_gpio.
 *
 * Switches are available on these pins:
 * A4 - SW1
 * A5 - SW2
 * A16 - SW3
 * A17 - SW4
 * When pressed, we read 0 from the pin.
 *
  * LED available on the board:
 * (LED directly on FRDM-KL25Z board)
 * B18 	- Red LED
 * B19 	- Green LED
 * D1	- Blue LED (Arduino pin D13)
 * (LED on the mother board)
 * B8 - LED_RED
 * B9 - LED_YELLOW
 * B10 - LED_GREEN
 * All LEDs turn on with low level on the pin.
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
	GPIO_Initialize();

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
