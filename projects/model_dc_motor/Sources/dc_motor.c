/*
 * DRV_motor.c
 *
 * Jednoduchy ovladac pro model DC motorku pripojeny k vyvojovemu kitu utb.
 * Vyuziva casovac TPM0.
 *
 */
#include "drv_lcd.h"
#include "MKL25Z4.h"
#include "stdbool.h"

#define		SPEED		(2) // cteni

#define direction           (6) // smer
#define	direction_MASK		(1 << direction)
#define	direction_TOGGLE()	PTD->PTOR |= direction_MASK
#define	direction_ON()		PTD->PCOR |= direction_MASK
#define	direction_OFF()		PTD->PSOR |= direction_MASK


#define roll           (4) // toceni
#define	roll_MASK		(1 << roll)
#define	roll_TOGGLE()	PTD->PTOR |= roll_MASK
#define	roll_ON()		PTD->PSOR |= roll_MASK
#define	roll_OFF()		PTD->PCOR |= roll_MASK


static int i=0;
static bool one=false;

static void TPMInitialize(void);

void DCMOTOR_Init()
{
	// rychlost
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[SPEED] = PORT_PCR_MUX(1);
	PTD->PDDR &= ~(1 << SPEED);


	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[direction] = PORT_PCR_MUX(1);
	direction_OFF();
	PTD->PDDR |= direction_MASK;

	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[roll] = PORT_PCR_MUX(1);
	roll_OFF();
	PTD->PDDR |= roll_MASK;

}


// Nastaveni smeru otaceni. Vstup je 0 nebo 1.
// 0 = rele sepnuto
// 1 = rele vypnuto
void DCMOTOR_setDirection(char dir)
{
	if(dir == 1)
	{
		direction_ON();
	}
	else
	{
		direction_OFF();
	}
}


// roytoci motor
void DCMOTOR_SpinON()
{
	roll_ON();
}

// zastavi motor
void DCMOTOR_SpinOFF()
{
	roll_OFF();
}

// Vraci otacky za minutu
int DCMOTOR_GetRpm() {
	int cnt = 0;

	// Init timer to overflow in 0.5 sec.
	TPMInitialize();

	// Wait for overflow and measure the pulses from rpm sensor
	while ( !(TPM0->SC & TPM_SC_TOF_MASK) ) {
		if ( (PTD->PDIR & (1 << SPEED)) != 0 ) {
			// detekovan puls
			cnt++;
			// cekat na konec pulsu
			while ( ((PTD->PDIR & (1 << SPEED)) != 0)
					&& !(TPM0->SC & TPM_SC_TOF_MASK) )
					//&& (SYSTICK_millis() < endTime) )
				;
		}
	}
	// clear TOF flag
	TPM0->SC |= TPM_SC_TOF_MASK;

	// pocet pulsu prepocteny na minutu a vydeleny poctem pulsu na otacku
	return (cnt*2*60)/3;
}


static void TPMInitialize(void)
{
	// Povolit clock pro TPM0
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;

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
	TPM0->SC = TPM_SC_CMOD(0);

	// ...pockat az se zmena projevi (acknowledged in the LPTPM clock domain)
	while (TPM0->SC & TPM_SC_CMOD_MASK)
		;

	// ... pri zakazanem citaci provest nastaveni modulo
	// Pri clock = 8 MHz / 128 = 62500 Hz,
	// tedy citac napocita do 62500 za 1 sekundu.
	// Pro cekani 1 ms potrebujeme, aby citac pretekal pri hodnote 62500
	TPM0->CNT = 0;	// manual doporucuje vynulovat citac
	TPM0->MOD = 31250;

	// ... a nakonec nastavit pozadovane hodnoty:
	// ... delicka (prescale) = 128
	// ... interni zdroj hodinoveho signalu
	TPM0->SC = ( TPM_SC_CMOD(1) | TPM_SC_PS(7));
}
