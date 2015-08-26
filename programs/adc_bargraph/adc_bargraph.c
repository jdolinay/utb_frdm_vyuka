/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Analogove-Digitalni prevodnik.
 * Program ukazuje ziskani hodnoty z A/D prevodniku.
 * Napeti na vstupu A/D prevodniku se nastavuje potenciometrem na kitu.
 * Podle velikosti napeti se rozsviti 0 az 3 LED na kitu.
 * LED tak predstavuji sloupcovy indikator (bar graph) napeti na vstupu
 * A/D prevodniku, ktere je nastaveno potenciometrem.
 *
 * Uzitecne informace:
 * Potenciometr je na pinu PTC2 (coz je kanal 11 A/D prevodniku)
 *
 * Hodinova frekvence A/D prevodniku musi byt 2 - 12 MHz pri rozliseni 16 bit.
 * Pri rozliseni <= 13 bit 1 - 18 MHz.
 * Pro kalibraci je doporuceno <= 4 MHz.
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"

void delay(void);
void ADCInit(void);
uint32_t ADCCalibrate(void);

int main(void)
{

	// Pro praci s LED vyuzijeme ovladac GPIO
	GPIO_initialize();

	// Piny pro LED jako vystupy
	pinMode(LD1, OUTPUT);
	pinMode(LD2, OUTPUT);
	pinMode(LD3, OUTPUT);

	// Zhasnuti LED zapisem log. 1
	pinWrite(LD1, HIGH);
	pinWrite(LD2, HIGH);
	pinWrite(LD3, HIGH);


	// Inicializace A/D prevodniku
	ADCInit();

	// Kalibrace a nova inicializace
	// Pro dosazeni udavane presnosti musi byt prevodnik kalibrovan po
	// kazdem resetu.
	// Nova inicializace je potreba protoze pri kalibraci
	// je prevodnik prenastaven.
	ADCCalibrate();
	ADCInit();

	// Nastaveni pinu, kde je pripojen potenciometr,
	// do rezimu vstupu pro A/D prevodnik: pin PTC2, funkce ALT0
	// 1. Povolime hodinovy signal pro port C
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	// 2. NAstavime funkci pinu na ALT0 = vstup A/D prevodniku
	PORTC->PCR[2] = PORT_PCR_MUX(0);


	while (1) {

		// Spusteni prevodu na kanalu 11.
		// Protoze ostatni nastaveni v registru SC1 mame na 0, muzeme si dovolit
		// primo v nem prepsat hodnotu cislem kanalu. Lepsi reseni by bylo
		// "namaskovat" cislo kanalu bez zmeny hodnoty registru.
		ADC0->SC1[0] = ADC_SC1_ADCH(11);

		// Cekame na dokonceni prevodu
		while ( (ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0 )
			;

		// Ulozime vysledek prevodu
		uint16_t vysledek = ADC0->R[0];

		// Zhasneme vsechny LED
		pinWrite(LD1, HIGH);
		pinWrite(LD2, HIGH);
		pinWrite(LD3, HIGH);

		// Vyhodnotime vysledek: pri 10 bitovem prevodu je v rozsahu 0 - 1023
		if (vysledek > 255) {
			// Rozsvit LED1
			pinWrite(LD1, LOW);
		}

		if (vysledek > 510) {
			// Rozsvit LED2
			pinWrite(LD2, LOW);
		}

		if (vysledek > 765) {
			// Rozsvit LED3
			pinWrite(LD3, LOW);
		}

		delay();
	}	// while



    /* Never leave main */
    return 0;
}

/*	ADCInit
    Inicializuje A/D prevodnik
    Nastavuje zdroj hodin na bus clock a delicku na 8,
    rozliseni na 10 bit, ...
*/
void ADCInit(void)
{
	// Povolit hodinovy signal pro ADC
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

	// Zakazeme preruseni, nastavime kanal 31 = A/D prevodnik vypnut, jinak by zapisem
	// doslo ke spusteni prevodu
	// Vybereme single-ended mode
	ADC0->SC1[0] =  ADC_SC1_ADCH(31);

	// Vyber hodinoveho signalu, preddelicky a rozliseni
	// Clock pro ADC nastavime <= 4 MHz, coz je doporuceno pro kalibraci.
	// Pri max. CPU frekvenci 48 MHz je bus clock 24 MHz, pri delicce = 8
	// bude clock pro ADC 3 MHz
	ADC0->CFG1 = ADC_CFG1_ADICLK(0)		/* ADICLK = 0 -> bus clock */
		| ADC_CFG1_ADIV(3)				/* ADIV = 3 -> clock/8 */
		| ADC_CFG1_MODE(2);				/* MODE = 2 -> rozliseni 10-bit */

	// Do ostatnich registru zapiseme vychozi hodnoty:
	// Vybereme sadu kanalu "a", vychozi nejdelsi cas prevodu (24 clocks)
	ADC0->CFG2 = 0;

	// Softwarove spousteni prevodu, vychozi reference
	ADC0->SC2 = 0;

	// Hardwarove prumerovani vypnuto
	ADC0->SC3 = 0;	/* default values, no averaging */

}

/*
  ADCCalibrate
  Kalibrace ADC.
  Kod prevzat z ukazkoveho kodu pro FRDM-KL25Z.
  Pri chybe kalibrace vraci 1, pri uspechu vraci 0
*/
uint32_t ADCCalibrate(void)
{
	 unsigned short cal_var;

	  ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;	/* Enable Software Conversion Trigger for Calibration Process */
	  ADC0->SC3 &= ( ~ADC_SC3_ADCO_MASK & ~ADC_SC3_AVGS_MASK );    /* set single conversion, clear avgs bitfield for next writing */

	  ADC0->SC3 |= ( ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(32) ); /* turn averaging ON and set desired value */

	  ADC0->SC3 |= ADC_SC3_CAL_MASK;      /* Start CAL */

	  /* Wait calibration end */
	  while ( (ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0 )
		  ;

	  /* Check for Calibration fail error and return */
	  if ( (ADC0->SC3 & ADC_SC3_CALF_MASK) != 0 )
		  return 1;

	  // Calculate plus-side calibration
	  cal_var = 0;
	  cal_var =  ADC0->CLP0;
	  cal_var += ADC0->CLP1;
	  cal_var += ADC0->CLP2;
	  cal_var += ADC0->CLP3;
	  cal_var += ADC0->CLP4;
	  cal_var += ADC0->CLPS;

	  cal_var = cal_var/2;
	  cal_var |= 0x8000; // Set MSB
	  ADC0->PG = ADC_PG_PG(cal_var);

	  // Calculate minus-side calibration
	  cal_var = 0;
	  cal_var =  ADC0->CLM0;
	  cal_var += ADC0->CLM1;
	  cal_var += ADC0->CLM2;
	  cal_var += ADC0->CLM3;
	  cal_var += ADC0->CLM4;
	  cal_var += ADC0->CLMS;

	  cal_var = cal_var/2;
	  cal_var |= 0x8000; // Set MSB
	  ADC0->MG = ADC_MG_MG(cal_var);

	  ADC0->SC3 &= ~ADC_SC3_CAL_MASK;

	  return 0;
}

// Kratke cekani
void delay(void)
{
	uint32_t n = 100000;
	while( n -- )
		;
}





////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
