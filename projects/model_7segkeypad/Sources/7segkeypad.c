/*
* Driver for model of 7-segment display with keypad.
* Ovladac pro modul 7-segmentoveho displeje s klavesnici
* Vyuziva casovac TPM1
*/


#include "MKL25Z4.h"
#include "drv_gpio.h"
#include "7segkeypad.h"


//deklarace pinu sloupcù a øádkù
#define		SW1	(4)
#define		SW2	(5)
#define		SW3	(16)
#define		SW4	(17)
#define		radek1	(21)
#define		radek2	(0)
#define		radek3	(30)
#define		radek4	(20)

//deklarace pinu pro segmenty
#define		segmentA	(2)
#define		segmentB	(11)
#define		segmentC	(0)
#define		segmentD	(1)
#define		segmentE	(4)
#define		segmentF	(3)
#define		segmentG	(5)
#define		vyber_A0	(7)
#define		vyber_A1	(6)
#define		vyber_A2	(5)

// prototypy funkcí
void TPMInitialize(void);
void TPM1_IRQHandler(void);

// pomocné promìnné
int radek = -1;
int sloupec = -1;
int r1=21;
int r2=0;
int r3=30;
int r4=20;
int s1=4;
int s2=5;
int s3=16;
int s4=17;
int stav;

// vraci kod stisknute klavesy
int SEGKEYPAD_GetKey(void) {
	// promennou aktualizuje casovac v ISR
	return stav;
}

//inicializace displeje
void SEGKEYPAD_InitializeDisplay(void)
{
	// povolime hodinovy signal pro porty A, B, C, D a E
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK |SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK| SIM_SCGC5_PORTE_MASK);

	//nastavime piny jednotlivych segmentù do rezimu GPIO
	PORTE->PCR[segmentA] = PORT_PCR_MUX(1);
	PORTB->PCR[segmentB] = PORT_PCR_MUX(1);
	PORTE->PCR[segmentC] = PORT_PCR_MUX(1);
	PORTE->PCR[segmentD] = PORT_PCR_MUX(1);
	PORTE->PCR[segmentE] = PORT_PCR_MUX(1);
	PORTE->PCR[segmentF] = PORT_PCR_MUX(1);
	PORTE->PCR[segmentG] = PORT_PCR_MUX(1);
	PORTC->PCR[vyber_A0] = PORT_PCR_MUX(1);
	PORTC->PCR[vyber_A1] = PORT_PCR_MUX(1);
	PORTC->PCR[vyber_A2] = PORT_PCR_MUX(1);

	//nastavime piny jako vystupni
	PTE->PDDR |= (1 << segmentA);
	PTB->PDDR |= (1 << segmentB);
	PTE->PDDR |= (1 << segmentC);
	PTE->PDDR |= (1 << segmentD);
	PTE->PDDR |= (1 << segmentE);
	PTE->PDDR |= (1 << segmentF);
	PTE->PDDR |= (1 << segmentG);
	PTC->PDDR |= (1 << vyber_A0);
	PTC->PDDR |= (1 << vyber_A1);
	PTC->PDDR |= (1 << vyber_A2);

	//nastavime piny radku a sloupcu klavesnice do rezimu GPIO
	PORTA->PCR[SW1] = PORT_PCR_MUX(1);
	PORTA->PCR[SW2] = PORT_PCR_MUX(1);
	PORTA->PCR[SW3] = PORT_PCR_MUX(1);
	PORTA->PCR[SW4] = PORT_PCR_MUX(1);
	PORTE->PCR[radek1] = PORT_PCR_MUX(1);
	PORTB->PCR[radek2] = PORT_PCR_MUX(1);
	PORTE->PCR[radek3] = PORT_PCR_MUX(1);
	PORTE->PCR[radek4] = PORT_PCR_MUX(1);

	// nastavime smer pinu radku (vstupu) a sloupcu (vystupu) klavesnice na vystupni   //!!!!!!!!!!!!!!
	PTA->PDDR &=~ (1 << SW1);
	PTA->PDDR &=~ (1 << SW2);
	PTA->PDDR &=~ (1 << SW3);
	PTA->PDDR &=~ (1 << SW4);
	PTE->PDDR |= (1 << radek1);
	PTB->PDDR |= (1 << radek2);
	PTE->PDDR |= (1 << radek3);
	PTE->PDDR |= (1 << radek4);

	// inicializace TMP casovace
	TPMInitialize();
}

//inicializace klavesnice
void SEGKEYPAD_InitializeKeyboard(void)
{
	SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK);

	//nastaveni pinu radku a sloupcu klavesnice do rezimu GPIO
	PORTA->PCR[SW1] = PORT_PCR_MUX(1);
	PORTA->PCR[SW2] = PORT_PCR_MUX(1);
	PORTA->PCR[SW3] = PORT_PCR_MUX(1);
	PORTA->PCR[SW4] = PORT_PCR_MUX(1);
	PORTE->PCR[radek1] = PORT_PCR_MUX(1);
	PORTB->PCR[radek2] = PORT_PCR_MUX(1);
	PORTE->PCR[radek3] = PORT_PCR_MUX(1);
	PORTE->PCR[radek4] = PORT_PCR_MUX(1);

	//nastaveni smeru pinu radku (vystupu) a sloupcu (vstupu) klavesnice na vystupni
	PTA->PDDR &= ~(1 << SW1);
	PTA->PDDR &= ~(1 << SW2);
	PTA->PDDR &= ~(1 << SW3);
	PTA->PDDR &= ~(1 << SW4);
	PTE->PDDR |= (1 << radek1);
	PTB->PDDR |= (1 << radek2);
	PTE->PDDR |= (1 << radek3);
	PTE->PDDR |= (1 << radek4);

	//Pro vstupy (sloupce) aktivujeme pull-up rezistory. 
	PORTA->PCR[SW1] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);
	PORTA->PCR[SW2] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);
	PORTA->PCR[SW3] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);
	PORTA->PCR[SW4] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);

	//Nastavíme pin radek1-radek2 jako výstup
	pinMode(radek1, OUTPUT);
	pinMode(radek2, OUTPUT);
	pinMode(radek3, OUTPUT);
	pinMode(radek4, OUTPUT);

	//Nastavíme pin SW1-SW4 jako vstup s povolenym pull-up rezistorem
	pinMode(SW1, INPUT_PULLUP);
	pinMode(SW2, INPUT_PULLUP);
	pinMode(SW3, INPUT_PULLUP);
	pinMode(SW4, INPUT_PULLUP);

	stav = 0;

}


// Funkce pro aktivaci jednotlivých displejù, pomocí parametru je urèeno,
// který displej bude využíván.
// Roysah 1 az 8.
void SEGKEYPAD_SelectDisplay(int display_c)
{
	switch(display_c)
	{
		case 1:
			PTC->PCOR |= (1 << vyber_A0);
			PTC->PCOR |= (1 << vyber_A1);
			PTC->PCOR |= (1 << vyber_A2);
			break;
		case 2:
			PTC->PSOR |= (1 << vyber_A0);
			PTC->PCOR |= (1 << vyber_A1);
			PTC->PCOR |= (1 << vyber_A2);
			break;
		case 3:
			PTC->PCOR |= (1 << vyber_A0);
			PTC->PSOR |= (1 << vyber_A1);
			PTC->PCOR |= (1 << vyber_A2);
			break;
		case 4:
			PTC->PSOR |= (1 << vyber_A0);
			PTC->PSOR |= (1 << vyber_A1);
			PTC->PCOR |= (1 << vyber_A2);
			break;
		case 5:
			PTC->PCOR |= (1 << vyber_A0);
			PTC->PCOR |= (1 << vyber_A1);
			PTC->PSOR |= (1 << vyber_A2);
			break;
		case 6:
			PTC->PSOR |= (1 << vyber_A0);
			PTC->PCOR |= (1 << vyber_A1);
			PTC->PSOR |= (1 << vyber_A2);
			break;
		case 7:
			PTC->PCOR |= (1 << vyber_A0);
			PTC->PSOR |= (1 << vyber_A1);
			PTC->PSOR |= (1 << vyber_A2);
			break;
		case 8:
			PTC->PSOR |= (1 << vyber_A0);
			PTC->PSOR |= (1 << vyber_A1);
			PTC->PSOR |= (1 << vyber_A2);
			break;
		default:
			break;
	}
}

// Vypise cislo v rozsahu 0 az 9 na aktivnim displeji
void SEGKEYPAD_ShowNumber(int cislo)
{
	switch(cislo)
	{
		case 0:
			PTE->PCOR |= (1 << segmentA);
			PTB->PCOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PCOR |= (1 << segmentD);
			PTE->PCOR |= (1 << segmentE);
			PTE->PCOR |= (1 << segmentF);
			PTE->PSOR |= (1 << segmentG);
			break;
		case 1:
			PTE->PSOR |= (1 << segmentA);
			PTB->PCOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PSOR |= (1 << segmentD);
			PTE->PSOR |= (1 << segmentE);
			PTE->PSOR |= (1 << segmentF);
			PTE->PSOR |= (1 << segmentG);
			break;
		case 2:
			PTE->PCOR |= (1 << segmentA);
			PTB->PCOR |= (1 << segmentB);
			PTE->PSOR |= (1 << segmentC);
			PTE->PCOR |= (1 << segmentD);
			PTE->PCOR |= (1 << segmentE);
			PTE->PSOR |= (1 << segmentF);
			PTE->PCOR |= (1 << segmentG);
			break;
		case 3:
			PTE->PCOR |= (1 << segmentA);
			PTB->PCOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PCOR |= (1 << segmentD);
			PTE->PSOR |= (1 << segmentE);
			PTE->PSOR |= (1 << segmentF);
			PTE->PCOR |= (1 << segmentG);
			break;
		case 4:
			PTE->PSOR |= (1 << segmentA);
			PTB->PCOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PSOR |= (1 << segmentD);
			PTE->PSOR |= (1 << segmentE);
			PTE->PCOR |= (1 << segmentF);
			PTE->PCOR |= (1 << segmentG);
			break;
		case 5:
			PTE->PCOR |= (1 << segmentA);
			PTB->PSOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PCOR |= (1 << segmentD);
			PTE->PSOR |= (1 << segmentE);
			PTE->PCOR |= (1 << segmentF);
			PTE->PCOR |= (1 << segmentG);
			break;
		case 6:
			PTE->PCOR |= (1 << segmentA);
			PTB->PSOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PCOR |= (1 << segmentD);
			PTE->PCOR |= (1 << segmentE);
			PTE->PCOR |= (1 << segmentF);
			PTE->PCOR |= (1 << segmentG);
			break;
		case 7:
			PTE->PCOR |= (1 << segmentA);
			PTB->PCOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PSOR |= (1 << segmentD);
			PTE->PSOR |= (1 << segmentE);
			PTE->PCOR |= (1 << segmentF);
			PTE->PSOR |= (1 << segmentG);
			break;
		case 8:
			PTE->PCOR |= (1 << segmentA);
		    PTB->PCOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PCOR |= (1 << segmentD);
			PTE->PCOR |= (1 << segmentE);
			PTE->PCOR |= (1 << segmentF);
			PTE->PCOR |= (1 << segmentG);
			break;
		case 9:
			PTE->PCOR |= (1 << segmentA);
			PTB->PCOR |= (1 << segmentB);
			PTE->PCOR |= (1 << segmentC);
			PTE->PCOR |= (1 << segmentD);
			PTE->PSOR |= (1 << segmentE);
			PTE->PCOR |= (1 << segmentF);
			PTE->PCOR |= (1 << segmentG);
			break;
		default:
			PTE->PSOR |= (1 << segmentA);
			PTB->PSOR |= (1 << segmentB);
			PTE->PSOR |= (1 << segmentC);
			PTE->PSOR |= (1 << segmentD);
			PTE->PSOR |= (1 << segmentE);
			PTE->PSOR |= (1 << segmentF);
			PTE->PSOR |= (1 << segmentG);
			break;
	}
}

/*
Inicializuje casovac TPM1 pro pouziti TPM1_IRQHandler.
*/
void TPMInitialize(void)
{
	// Povolit clock pro TPM1
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;

	// Nastavit zdroj hodin pro casovac TPM (sdileno vsemi moduly TPM)
	// Dostupne zdroje hodinoveho signalu zavisi na CLOCK_SETUP
	// Pro CLOCK_SETUP = 1 nebo 4 je mozno pouzit OSCERCLK (8 MHz)
	// Pro CLOCK_SETUP = 0 (vychozi v novem projektu) PLLFLLCLK (20.97152 MHz)
	// Mozne hodnoty:
	// 0 - clock vypnut
	// 1 - MCGFLLCLK nebo MCGFLLCLK/2
	// 2 - OSCERCLK
	// 3 - MCGIRCLK  (interni generator, 32 kHz nebo 4 MHz)
	// !!! Pozor pri zapisu do SOPT2 nespolehat na to, ze oba bity
	// pole TPMSRC jsou vynulovany, nestaci SOPT2[TPMSRC] |= nova_hodnota;
	// je nutno nejprve vynulovat a pak "ORovat" novou hodnotu.
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(2);

	// Nastavit casovac
	// Pole PS (prescale) muze byt zmeneno pouze, kdyz je
	// citac casovace zakazan (counter disabled), tj. pokud SC[CMOD] = 0
	// ...nejprve zakazat counter
	TPM1->SC = TPM_SC_CMOD(0);

	// ...pockat az se zmena projevi (acknowledged in the LPTPM clock domain)
	while (TPM1->SC & TPM_SC_CMOD_MASK)
		;

	// ... pri zakazanem citaci provest nastaveni modulo
	// Pri clock = 8 MHz / 128 = 62500 Hz,
	// tedy citac napocita do 62500 za 1 sekundu.
	// Pro 2 preruseni za sekundu modulo nastavit na 31250
	// Pri clock = 48 MHz / 128 = 375000 Hz
	// Pro 1 ms zvolime modulo reg 375
	TPM1->CNT = 0;	// manual doporucuje vynulovat citac
	TPM1->MOD = 24000;


	// ... a nakonec nastavit pozadovane hodnoty vcetne delicky (prescale)
	TPM1->SC = ( TPM_SC_TOIE_MASK	// povolit preruseni
			| TPM_SC_TOF_MASK	// smazat pripadny priznak preruseni
			| TPM_SC_CMOD(1)	// vyber interniho zdroje hodinoveho signalu
			| TPM_SC_PS(1) );	// delicka = 128


	// Preruseni je treba povolit take v NVIC
	// ...smazat pripadny priznak cekajiciho preruseni
	NVIC_ClearPendingIRQ(TPM1_IRQn);
	// ...povolit preruseni od TPM0
	NVIC_EnableIRQ(TPM1_IRQn);
	// ...nastavit prioritu preruseni: 0 je nejvysi, 3 nejnizsi
	NVIC_SetPriority(TPM1_IRQn, 2);
}

int keyboard()
	{
	int stav_klavesnice = -1;

	for (int i = 0; i < 4; i++)
	{
		PTE->PSOR |= (1 << radek1);  //PSOR nastavi pin na log 1
		PTB->PSOR |= (1 << radek2);
		PTE->PSOR |= (1 << radek3);
		PTE->PSOR |= (1 << radek4);

		//øádek, jehož stav se má zjišovat, je vybrán pomocí stavu logické "0"
		if (i == 0) {
			PTE->PCOR |= (1 << radek1); //PCOR nastavuje pin na log 0
		}
		else if (i == 1) {
			PTB->PCOR |= (1 << radek2);
		}
		else if (i == 2) {
			PTE->PCOR |= (1 << radek3);
		}
		else if (i == 3) {
			PTE->PCOR |= (1 << radek4);
		}

		//priradime index pro radky dle stisknute klavesy na klavesnici
		if (pinRead(SW1) == LOW) //ctu stav tlacitek prvního sloupce
		{
			if (i == 0)
				stav = 0;
			else if (i == 1)
				stav = 1;
			else if (i == 2)
				stav = 2;
			else if (i == 3)
				stav = 3;
		}
		else if (pinRead(SW2) == LOW) //ctu stav tlacitek druhého sloupce
		{
			if (i == 0)
				stav = 4;
			else if (i == 1)
				stav = 5;
			else if (i == 2)
				stav = 6;
			else if (i== 3)
				stav = 7;
		}
		else if (pinRead(SW3) == LOW) //ctu stav tlacitek tøetího sloupce
		{
			if (i == 0)
				stav = 8;
			else if (i == 1)
				stav = 9;
			else if (i == 2)
				stav = 10;
			else if (i == 3)
				stav = 11;
		}
		else if (pinRead(SW4) == LOW) //ctu stav tlacitek ètvrtého sloupce
		{
			if (i== 0)
				stav = 12;
			else if (i == 1)
				stav = 13;
			else if (i == 2)
				stav = 14;
			else if (i== 3)
				stav = 15;
		}
		else
		{
			//i = -1;
		}
	}
	return stav;

}

/* Obsluha pro TOF interrupt.
   Jmeno obsluzne funkce je preddefinovano.
   Staci v nasem programu vytvorit funkci tohoto jmena a bude
   volana ona misto vychozi prazdne funkce.
*/
void TPM1_IRQHandler(void)
{
	// Pokud je zdrojem preruseni TOF
	if (TPM1->SC & TPM_SC_TOF_MASK)
	{
		// vymazat priznak preruseni
		TPM1->SC |= TPM_SC_TOF_MASK;
		stav = keyboard();
	}
}


