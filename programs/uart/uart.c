/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Seriova komunikace (UART).
 * Program ukazuje prijem a vysilani znaku pres modul UART0.
 * Vysila na seriovou linku text "Ahoj". Poslanim znaku "s" lze tento
 * vypis zastavit a znovu spustit.
 * Komunikacni rychlost 9600 bd.
 *
 * POZOR: v nastaveni projektu > compiler > preprocesor musi byt CLOCK_SETUP=1
 * aby byl CPU clock 48 MHz!
 *
  Uzitecne informace
  1. KL25Z obsahuje 3 UART moduly: UART0, UART1 a UART2. Modul UART0 je pripojen
   	   take na piny pro USB komunikace, lze s nim tedy komunikovat pres virtualni
   	   seriovy port pres USB kabel zapojeny do SDA konektoru na desce FRDM.
   	   Je dostupny na pocitaci jako "Open SDA - CDC Serial Port".

  2. Soubor MKL25Z4.h definuje strukturu pro pristup k registrum UART (dle CMSIS)
 *
 */

#include "MKL25Z4.h"
#include "drv_uart.h"

void delay_ms(uint32_t millis);

volatile uint32_t g_delaycnt;

int main(void)
{
	uint8_t vypis = 1;
	char c;

	// Nastaveni preruseni od casovace SysTick na 1 ms periodu
	// vyuzito v delay_ms
	SysTick_Config(SystemCoreClock / 1000u );

	UART0_Initialize(BD9600);

	while ( 1 )
	{
		// Pokud je "vypis" povolen, vypisujeme text
		if (vypis )
			UART0_puts("ahoj\n");

		// poslanim znaku s jde vypis zastavit a znovu spustit
		// Testujeme, zda je ve vyrovnavaci pameti UART modulu prijaty znak...
		if ( UART0_Data_Available() )
		{
			// a pokud ano, precteme jej
			c = UART0_getch();
			// odesleme znak zpet = echo
			UART0_putch(c);

			// vyhodnotime precteny znak
			if ( c == 's' )
			{
				vypis = !vypis;	// negujeme hodnotu "vypis"
				UART0_puts("\nPrikaz prijat.\n");// vypiseme potvrzeni prikazu
			}
		}

		delay_ms(1000);
	}

    /* Never leave main */
    return 0;
}


/* delay_ms(ms)
 * Cekaci funkce s vyuzitim SysTick casovace (CMSIS).
 * SysTick timer je nastaven tak, aby jeho "tik" odpovidal 1 ms
 * pomoci SysTick_Config().
 * */
void delay_ms(uint32_t millis)
{
	g_delaycnt = millis;

    /* Cekame az SysTick dekrementuje pocitadlo na 0 */
    while (g_delaycnt != 0u)
       ;
}

/* Obsluha pro SysTick interrupt.
   Jmeno obsluzne funkce je preddefinovano.
   Staci v nasem programu vytvorit funkci tohoto jmena a bude
   volana ona misto vychozi prazdne funckce.
*/
void SysTick_Handler(void)
{
    /* Dekrementujeme pocitadlo pouzivane funkci delay_ms */
    if (g_delaycnt != 0u)
    {
        g_delaycnt--;
    }
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
