/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Ovladani LED pomoci tlacitek.
 *
 *
 * Mame k dispozici 4 tlacitka SW1 az SW4:
 * A4 - SW1
 * A5 - SW2
 * A16 - SW3
 * A17 - SW4
 * Pri stisku tlacitka je z pinu ctena log. 0.
 * Na pinu by mel byt zapnut pull up rezistor.
 *
 * Mame k dispozici tyto LED:
 * LED primo na FRDM-KL25Z:
 * B18 	- Red LED
 * B19 	- Green LED
 * D1	- Blue LED (Arduino pin D13)
 * Na UTB kitu jsou dalsi LED:
 * B8 - LED_RED
 * B9 - LED_YELLOW
 * B10 - LED_GREEN
 *  Vsechny LED sviti pokud je pin 0 (LOW).
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

 *
 * tip: upravit aby i test off tlacitka pouzival funkci
 * tip: vytvorit vlastni funkci pro zap/vyp led.
 */

#include "MKL25Z4.h"

// Cislo pinu (na portu B) pro LED, kterou ovladame.
// Napr. 18 pro cervenou LED primo na FRDM desce.
// Pozor: zmena je mozna pouze v ramci portu B, kod neni univerzalni,
// nebude fungovat pro piny na jinem portu
#define		LED_PIN		(19)

// Cisla pinu na portu A pro tlacitka, ktera pouzivame.
// 4 = SW1, 5 = SW2.
#define		ON_PIN		(4)
#define		OFF_PIN		(5)


// Prototypy funkci definovanych nize
void delay(void);
static inline int IsKeyPressed(int pin);


int main(void)
{
	// 1. Povolime hodinovy signal pro port A a B
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK );

	// 2. Nastavime funkci pinu na GPIO
	// PORT_PCR_MUX je makro definovane v MKL25Z4.h, vstupem je
	// cislo funkce pinu (zjistime v datasheetu)
	PORTB->PCR[LED_PIN] = PORT_PCR_MUX(1);
	PORTA->PCR[ON_PIN] = PORT_PCR_MUX(1);
	PORTA->PCR[OFF_PIN] = PORT_PCR_MUX(1);

	// 3. Nastavime smer pinu LED na vystupni
	// Zapisem 1 do prislusneho bitu registru PDDR
	PTB->PDDR |= (1 << LED_PIN);
	// Pro tlacitka na vstupni
	PTA->PDDR &= ~(1 << ON_PIN);
	PTA->PDDR &= ~(1 << OFF_PIN);

	// 4. Zapneme interni pull-up rezistory pro tlacitka
	// Poznamka: pro nas kit neni potreba, jsou osazeny externi, ale
	// pro ukazku zapiname.
	PORTA->PCR[ON_PIN] |= (1 << ON_PIN);
	PORTA->PCR[OFF_PIN] |= (1 << OFF_PIN);

	// Zhasneme LED zapisem log. 1 na pin
	PTB->PSOR |= (1 << LED_PIN);

	// 5. Cekame na stisk tlacitek a podle toho ovladame LED
	while(1)
	{
		// Tlacitko pro zapnuti stisknuto?
		// stisknuto = na pinu je log. 0.
		// Hodnotu pinu cteme z registru PDIR (Data In register)
		if ( IsKeyPressed(ON_PIN) )
			PTB->PCOR |= (1 << LED_PIN);	// zapnout LED (clear pin)

		if ( (PTA->PDIR & (1 << OFF_PIN)) == 0 )
			PTB->PSOR |= (1 << LED_PIN);	// vypnout LED (set pin)

		delay();
	}



    /* Never leave main */
    return 0;
}

/* delay
 * Jednoducha cekaci funkce - busy wait
 * */
void delay(void)
{
	unsigned long n = 30000L;
	while ( n-- )
		;
}

/* Vraci 1 pokud je tlacitko na danem pinu stisknuto a 0 pokud neni stisknuto
 * */
static inline int IsKeyPressed(int pin)
{
	if ((PTA->PDIR & (1 << pin)) == 0)
		return 1;
	else
		return 0;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
