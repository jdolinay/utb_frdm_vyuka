/*
 * Projekt pro testovani modelu Misici jednotky EDU MOD
*/

#include "MKL25Z4.h"

// Model vyuziva piny na portech B, C, D, E
// Makra pro nastaveni vystupu
#define	SV4_ON()			PTE->PSOR |= (1 << 0)
#define	SV4_OFF()			PTE->PCOR |= (1 << 0)
#define	SV5_ON()			PTE->PSOR |= (1 << 1)
#define	SV5_OFF()			PTE->PCOR |= (1 << 1)
#define	SV3_ON()			PTE->PSOR |= (1 << 4)
#define	SV3_OFF()			PTE->PCOR |= (1 << 4)
#define	SV1_ON()			PTE->PSOR |= (1 << 5)
#define	SV1_OFF()			PTE->PCOR |= (1 << 5)
#define	SV2_ON()			PTC->PSOR |= (1 << 1)
#define	SV2_OFF()			PTC->PCOR |= (1 << 1)
#define	MICHADLO_ON()		PTD->PSOR |= (1 << 0)
#define	MICHADLO_OFF()		PTD->PCOR |= (1 << 0)



// Makra pro testovani vstupu
#define	IS_H1()			(PTD->PDIR & (1 << 3))
#define	IS_H2()			(PTC->PDIR & (1 << 16))
#define	IS_H3()			(PTD->PDIR & (1 << 2))
#define	IS_H4()			(PTC->PDIR & (1 << 5))
#define	IS_H5()			(PTD->PDIR & (1 << 4))
#define	IS_H6()			(PTC->PDIR & (1 << 7))
#define	IS_H7()			(PTD->PDIR & (1 << 5))
#define	IS_H8()			(PTC->PDIR & (1 << 6))

// Prototypy funkci definovanych nize
void delay(void);



int main(void) {
	// 1. Povolime hodinovy signal pro porty
	SIM->SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK |
	SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK);

	// 2. Nastavime funkci pinu na GPIO
	PORTE->PCR[0] = PORT_PCR_MUX(1);	// E0 = SV4
	PORTE->PCR[1] = PORT_PCR_MUX(1);	// E1 = SV5
	PORTE->PCR[4] = PORT_PCR_MUX(1);	// E4 = SV3
	PORTE->PCR[5] = PORT_PCR_MUX(1);	// E5 = SV1
	PORTC->PCR[1] = PORT_PCR_MUX(1);	// C1 = SV2
	PORTD->PCR[0] = PORT_PCR_MUX(1);	// D0 = Michadlo

	PORTC->PCR[16] = PORT_PCR_MUX(1);	// C16 = Hladina H2
	PORTD->PCR[5] = PORT_PCR_MUX(1);	// D5 = H7
	PORTD->PCR[4] = PORT_PCR_MUX(1);	// D4 = H5
	PORTD->PCR[2] = PORT_PCR_MUX(1);	// D2 = H3
	PORTD->PCR[3] = PORT_PCR_MUX(1);	// D3 = H1
	PORTC->PCR[7] = PORT_PCR_MUX(1);	// C7 = H6
	PORTC->PCR[6] = PORT_PCR_MUX(1);	// C6 = H8
	PORTC->PCR[5] = PORT_PCR_MUX(1);	// C5 = H4

	// 3. Nastavime prislusne piny jako vystupy a vstupy
	// vystupy
	SV1_OFF();
	SV2_OFF();
	SV3_OFF();
	SV4_OFF();
	SV5_OFF();
	MICHADLO_OFF();
	PTE->PDDR |= (1 << 0);
	PTE->PDDR |= (1 << 1);
	PTE->PDDR |= (1 << 4);
	PTE->PDDR |= (1 << 5);
	PTC->PDDR |= (1 << 1);
	PTD->PDDR |= (1 << 0);

	// vstupy
	PTC->PDDR &= ~(1 << 16);
	PTD->PDDR &= ~(1 << 4);
	PTD->PDDR &= ~(1 << 5);
	PTD->PDDR &= ~(1 << 2);
	PTD->PDDR &= ~(1 << 3);
	PTC->PDDR &= ~(1 << 7);
	PTC->PDDR &= ~(1 << 6);
	PTC->PDDR &= ~(1 << 5);

	while(1) {

		// nadrz 1
		SV1_ON();
		while( !IS_H3() )
			;
		SV1_OFF();
		delay();

		SV1_ON();
		while( !IS_H2() )
			;
		SV1_OFF();
		delay();

		SV1_ON();
		while (!IS_H1())
			;
		SV1_OFF();
		delay();

		// Nadrz 2
		SV2_ON();
		while (!IS_H5())
			;
		SV2_OFF();
		delay();

		SV2_ON();
		while (!IS_H4())
			;
		SV2_OFF();
		delay();

		// Nadrz 3
		SV3_ON();
		while (!IS_H8())
			;
		SV3_OFF();
		delay();

		SV3_ON();
		while (!IS_H7())
			;
		SV3_OFF();
		delay();

		SV3_ON();
		while (!IS_H6())
			;
		SV3_OFF();
		delay();

		// Napustit mixer
		SV4_ON();
		while ( IS_H3() )
			;
		delay();
		SV4_OFF();

		// Michat
		MICHADLO_ON();
		delay();
		delay();
		MICHADLO_OFF();
		delay();

		// Vypustit
		SV5_ON();
		delay();
		delay();
		delay();
		SV5_OFF();

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
