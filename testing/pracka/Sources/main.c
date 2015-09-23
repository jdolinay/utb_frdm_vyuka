/*
 * Projekt pro testovani modelu Pracka EDU MOD
*/

#include "MKL25Z4.h"

// Model vyuziva piny na portech B, C, D, E
// Makra pro nastaveni vystupu
#define	VODA_ON()			PTE->PSOR |= (1 << 0)
#define	VODA_OFF()			PTE->PCOR |= (1 << 0)
#define	TOP_ON()			PTE->PSOR |= (1 << 1)
#define	TOP_OFF()			PTE->PCOR |= (1 << 1)
#define	OTACKY_ON()			PTE->PSOR |= (1 << 4)
#define	OTACKY_OFF()		PTE->PCOR |= (1 << 4)
#define	BUBEN_VPRAVO_ON()	PTE->PSOR |= (1 << 5)
#define	BUBEN_VPRAVO_OFF()	PTE->PCOR |= (1 << 5)
#define	BUBEN_VLEVO_ON()	PTC->PSOR |= (1 << 1)
#define	BUBEN_VLEVO_OFF()	PTC->PCOR |= (1 << 1)
#define	CERPADLO_ON()		PTD->PSOR |= (1 << 0)
#define	CERPADLO_OFF()		PTD->PCOR |= (1 << 0)



// Makra pro testovani vstupu
#define	IS_TEPLOTA_30()	(PTD->PDIR & (1 << 3))
#define	IS_TEPLOTA_40()	(PTC->PDIR & (1 << 16))
#define	IS_TEPLOTA_60()	(PTD->PDIR & (1 << 2))
#define	IS_TEPLOTA_90()	(PTC->PDIR & (1 << 5))
#define	IS_HLADINA_50()	(PTC->PDIR & (1 << 6))
#define	IS_HLADINA_100()(PTD->PDIR & (1 << 4))

static int i = 0;
// Prototypy funkci definovanych nize
void delay(void);

int main(void)
{
	// 1. Povolime hodinovy signal pro porty
	SIM->SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK |
					SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK);

	// 2. Nastavime funkci pinu na GPIO
	PORTE->PCR[0] = PORT_PCR_MUX(1);	// E0 = Voda
	PORTE->PCR[1] = PORT_PCR_MUX(1);	// E1 = Topeni
	PORTE->PCR[4] = PORT_PCR_MUX(1);	// E4 = Buben otacky
	PORTE->PCR[5] = PORT_PCR_MUX(1);	// E5 = Buben vpravo
	PORTC->PCR[1] = PORT_PCR_MUX(1);	// C1 = Buben vlevo
	PORTD->PCR[0] = PORT_PCR_MUX(1);	// D0 = Cerpadlo

	PORTC->PCR[16] = PORT_PCR_MUX(1);	// C16 = Teplota 40
	//PORTD->PCR[5] = PORT_PCR_MUX(1);	// N/A
	PORTD->PCR[4] = PORT_PCR_MUX(1);	// D4 = Hladina 100%
	PORTD->PCR[2] = PORT_PCR_MUX(1);	// D2 = Teplota 60
	PORTD->PCR[3] = PORT_PCR_MUX(1);	// D3 = Teplota 30
	//PORTC->PCR[7] = PORT_PCR_MUX(1);	// N/A
	PORTC->PCR[6] = PORT_PCR_MUX(1);	// C6 = Hladina 50%
	PORTC->PCR[5] = PORT_PCR_MUX(1);	// C5 = Teplota 90

	// 3. Nastavime prislusne piny jako vystupy a vstupy
	// vystupy
	PTE->PDDR |= (1 << 0);
	PTE->PDDR |= (1 << 1);
	PTE->PDDR |= (1 << 4);
	PTE->PDDR |= (1 << 5);
	PTC->PDDR |= (1 << 1);
	PTD->PDDR |= (1 << 0);

	// vstupy
	PTC->PDDR &= ~(1 << 16);
	PTD->PDDR &= ~(1 << 4);
	PTD->PDDR &= ~(1 << 2);
	PTD->PDDR &= ~(1 << 3);
	PTC->PDDR &= ~(1 << 6);
	PTC->PDDR &= ~(1 << 5);


	// Test vstupu a vystupu
	VODA_ON();
	while ( !IS_HLADINA_100() )
		;
	VODA_OFF();

	// Topit opstupne na jednotlive teploty s malou pauzou
	TOP_ON();
	while (!IS_TEPLOTA_30() )
		;
	TOP_OFF();
	delay();

	TOP_ON();
	while (!IS_TEPLOTA_40() )
		;
	TOP_OFF();
	delay();

	TOP_ON();
	while (!IS_TEPLOTA_60() )
		;
	TOP_OFF();
	delay();

	TOP_ON();
	while (!IS_TEPLOTA_90() )
		;
	TOP_OFF();
	delay();

	// vypustit vodu s pauzou po 100 a 50%
	CERPADLO_ON();
	while ( IS_HLADINA_100() )
		;
	CERPADLO_OFF();
	delay();

	CERPADLO_ON();
	while ( IS_HLADINA_50() )
			;
	CERPADLO_OFF();
	delay();
	CERPADLO_ON();
	delay();
	delay();
	CERPADLO_OFF();

	while(1)
		;


	// Testujeme vsechny vystupy...
	while (1) {
		VODA_ON();
		delay();
		VODA_OFF();

		TOP_ON();
		delay();
		TOP_OFF();

		BUBEN_VPRAVO_ON();
		delay();
		OTACKY_ON();
		delay();
		OTACKY_OFF();
		delay();
		BUBEN_VPRAVO_OFF();
		delay();
		BUBEN_VLEVO_ON();
		delay();
		BUBEN_VLEVO_OFF();

		CERPADLO_ON();
		delay();
		CERPADLO_OFF();
		delay();

	}	// while 1


    /* This for loop should be replaced. By default this loop allows a single stepping. */
    for (;;) {
        i++;
    }
    /* Never leave main */
    return 0;
}

/* delay
 * Jednoducha cekaci funkce - busy wait
 * */
void delay(void)
{
	unsigned long n = 2000000L;
	while ( n-- )
		;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
