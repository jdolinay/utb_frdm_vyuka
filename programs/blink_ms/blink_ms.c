/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Blikani LED s vyuzitim presneho cekani delay_ms
 *
 * Mame k dispozici tyto LED:
 * LED primo na FRDM-KL25Z:
 * B18 	- Red LED
 * B19 	- Green LED
 * D1	- Blue LED (Arduino pin D13)
 * Vsechny LED sviti pokud je pin 0 (LOW).
 * Na UTB kitu jsou dalsi LED:
 * B8 - LED_RED
 * B9 - LED_YELLOW
 * B10 - LED_GREEN
 *
 *
  Uzitecne informace
  1. U ARM je nutno povolovat hodinovy signal (clock) pro jednotlive porty.
  	  V registru SCGC5 v modulu SIM.

  2. Soubor MKL25Z4.h definuje strukturu pro pristup k registrum portu. (dle CMSIS)
	 Struktura se jmenuje PORT_Type.
	 Definuje take "instance" teto struktury pro jednotlive porty, napr.:
	 #define PORTA   ((PORT_Type *)PORTA_BASE)

  3. Dale definuje struktury pro pristup k portu v rezimu GPIO, GPIO_Type.
	 A instance teto struktury pro jednotlive porty, napr.:
	 #define PTA     ((GPIO_Type *)PTA_BASE)

 */

#include "MKL25Z4.h"

// Cislo pinu, ktery budeme pouzivat, napr. 18 pro cervenou LED primo na FRDM desce.
// Pozor: zmena je mozna pouze v ramci portu B, kod nice neni univerzalni,
// nebudwe fungovat pro piny na jinem portu
#define		LED_PIN		(18)

volatile uint32_t g_delaycnt;

// Prototypy funkci definovanych nize
void delay_ms(uint32_t millis);

int main(void)
{
	// Nastaveni preruseni od casovace SysTick na 1 ms periodu
	// vyuzito v delay_ms
	SysTick_Config(SystemCoreClock / 1000u );

	// 1. Povolime hodinovy signal pro port B
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;

	// 2. Nastavime funkci pinu na GPIO
	// PORT_PCR_MUX je makro definovane v MKL25Z4.h, vstupem je
	// cislo funkce pinu (zjistime v datasheetu)
	PORTB->PCR[LED_PIN] = PORT_PCR_MUX(1);

	// 3. Nastavime smer pinu na vystupni
	// Zapisem 1 do prislusneho bitu registru PDDR
	PTB->PDDR |= (1 << LED_PIN);

	// 4. Zapisujeme na pin stridave log. 1 a log. 0
	// Pro zapis log. 1 = nastaveni pinu = set slouzi registr PSOR
	// Pro zapis log. 0 = clear registr PCOR
	// Do obou registru se zapisuje 1 pro provedeni "akce" set nebo clear
	while(1)
	{
		PTB->PSOR |= (1 << LED_PIN);	// pin = log. 1
		delay_ms(300);
		PTB->PCOR |= (1 << LED_PIN);	// pin = log. 0
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
