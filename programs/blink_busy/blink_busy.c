/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Blikani LED.
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

  4. Funkce main obsahuje 2 verze kodu. Pomoci komentaru muzete zvolit, ktera se pouzije.

 *
 * tip: clock pro modrou led nezapnut; tak zjisti ze pada po uprave na modrou LED.
 *  + sami mohou najit definice pres prave tl + Open Declaration
 * tip: ukol bude upravit poskytnute funkce na jiny port a pin
 * tip: podle miry porozumneni jde udelat efektivnejsi kod, kdyz beru jako
 * samostatne objekty, vymyslim max. kod if (0 ) pouzij port A, else if (1) pouzij portB.
 * ale kdyz vim ze jsou to adresy v pameti, muzu udelat makro, ktere podle daneho cisla portu
 * vypocte "base" adresu pro jeho objekt.
 */

#include "MKL25Z4.h"

// Cislo pinu, ktery budeme pouzivat, napr. 18 pro cervenou LED primo na FRDM desce.
// Pozor: zmena je mozna pouze v ramci portu B, kod neni univerzalni,
// nebude fungovat pro piny na jinem portu
#define		LED_PIN		(19)


// Prototypy funkci definovanych nize
void delay(void);


int main(void)
{
	// 1. Povolime hodinovy signal pro port A a B
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK );


	// Dalsi kod je ve 2 verzich: A nebo B.

	///////////// Verze A: "objektovy pristup"

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
		delay();
		PTB->PCOR |= (1 << LED_PIN);	// pin = log. 0
		delay();
	}


	///////////// Verze B: "neobjektovy pristup"
	// Toto je alternativni pristup k registrum pomoci maker, ktera jako parametr
	// maji prislusny port. Odpovida to zapisu:
	// 	vrat_registr_smeru_portu(port) = mask;
	// misto objektoveho pristupu:
	// port->registr_smeru_portu = maska;
	// Makra jsou definovana take v MKL25Z4.h

	/*
	// 2. Nastavime funkci pinu na GPIO
	PORT_PCR_REG(PORTB, LED_PIN) = PORT_PCR_MUX(1);

	// 3. Nastavime smer pinu na vystupni
	GPIO_PDDR_REG(PTB) |= (1 << LED_PIN);

	// 4. Zapisujeme na pin stridave log. 1 a log. 0
	while(1)
	{
		GPIO_PSOR_REG(PTB) |= (1 << LED_PIN);
		delay();
		GPIO_PCOR_REG(PTB) |= (1 << LED_PIN);
		delay();
	}
	*/

    /* Never leave main */
    return 0;
}

/* delay
 * Jednoducha cekaci funkce - busy wait
 * */
void delay(void)
{
	unsigned long n = 1000000L;
	while ( n-- )
		;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
