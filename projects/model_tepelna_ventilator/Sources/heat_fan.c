/*
 * heat_fan.c
 *
 * Driver for heating plant with fan which connects to UTB learning kit
 * with FRDM kl25z.
 * Uses timer TPM0.
 */

#include "MKL25Z4.h"
#include "heat_fan.h"
#include <stdbool.h>


#define PIN_VYBER_CLENU_OUTPUT (7)
#define	PIN_PWM_SIGNAL_OUTPUT (4)
#define PIN_PWM_TEPLOMER_INPUT (0)
#define PIN_IMPULS_VENTILATOR_INPUT (3)

static void TPMInitialize(void);


void HEATFAN_Init(void) {

	//Povolime hodinovy signal pro port D
	SIM->SCGC5 |= (SIM_SCGC5_PORTD_MASK);

	// GPIO pins
	PORTD->PCR[PIN_VYBER_CLENU_OUTPUT] = PORT_PCR_MUX(1);
	PORTD->PCR[PIN_PWM_SIGNAL_OUTPUT] = PORT_PCR_MUX(1);
	PORTD->PCR[PIN_PWM_TEPLOMER_INPUT] = PORT_PCR_MUX(1);
	PORTD->PCR[PIN_IMPULS_VENTILATOR_INPUT] = PORT_PCR_MUX(1);

	// pin direction
	PTD->PDDR |= (1 << PIN_VYBER_CLENU_OUTPUT);
	PTD->PDDR |= (1 << PIN_PWM_SIGNAL_OUTPUT);

	PTD->PDDR &= ~(1 << PIN_IMPULS_VENTILATOR_INPUT);
	PTD->PDDR &= ~(1 << PIN_PWM_TEPLOMER_INPUT);
	PORTD->PCR[PIN_PWM_TEPLOMER_INPUT] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK);
}

// Measure the temperature from SMT160 sensor with pwm output.
// Returns temperature in degrees celsius
int HEATFAN_GetTemperature(void) {
	long kladny = 0;
	long zaporny = 0;
	int i;
	for (i = 0; i < 100000; i++) {
		if ((PTD->PDIR & (1 << PIN_PWM_TEPLOMER_INPUT)) == 0) {
			zaporny++;
		} else {
			kladny++;
		}
	}
	//ROZSAH: -45°C +130°C
	return ((((float) kladny / (kladny + zaporny)) * 175) - 45);
}

// measure the speed of the fan
uint16_t HEATFAN_GetFanRPM(void) {
	int i;
	uint32_t start, konec;
	uint32_t pocetImpulzu = 0;

	// Init timer to overflow in 0.5 sec.
	TPMInitialize();

	// Wait for overflow and measure the pulses from rpm sensor
	while ( !(TPM0->SC & TPM_SC_TOF_MASK) ) {
		if ( (PTD->PDIR & (1 << PIN_IMPULS_VENTILATOR_INPUT)) == 0) {
			// detekovan puls
			pocetImpulzu++;
			// cekat na konec pulsu
			while ( ((PTD->PDIR & (1 << PIN_IMPULS_VENTILATOR_INPUT)) == 0)
					&& !(TPM0->SC & TPM_SC_TOF_MASK) )
				;
		}
	}
	// clear TOF flag
	TPM0->SC |= TPM_SC_TOF_MASK;

	// pocet pulsu prepocteny na minutu a vydeleny poctem pulsu na otacku
	return (pocetImpulzu*2*60)/2;


	/*start = SYSTICK_millis();
	for (i = 0; i < 1000; i++) {
		if ((PTD->PDIR & (1 << PIN_IMPULS_VENTILATOR_INPUT)) == 0) {
			pocetImpulzu++;
		}
	}
	konec = SYSTICK_millis();*/
	//uint32_t cas = konec - start;
	//return (uint16_t) (pocetImpulzu / cas * 1000);
}

// Select heater or fan for control by the MCU
// The other is controlled by potentiometer on the model board.
void HEATFAN_CtrlSignalSel(HEATFAN_Actuator clen) {
	//pin = 0 - heater - topeni
	//pin = 1 - fan - ventilator
	if (clen == HEATFAN_Heater) {
		PTD->PCOR |= (1 << PIN_VYBER_CLENU_OUTPUT);
	} else if (clen == HEATFAN_Fan) {
		PTD->PSOR |= (1 << PIN_VYBER_CLENU_OUTPUT);
	}

}

// Generates "pwm" to the output to control heater or fan
// todo: spatne reseni, muselo by se volat neustale dokola aby byl generovany signal
// spravny. pokud se nevola takto, pak v signalu budou mezery. Pro topeni pouzitelne,
// ale pro rozeni ventilatoru???
void HEATFAN_DoPWMPulse(int pwm_percents) {
	int i;
	for (i = 0; i < 100; i++) {
		//Zaslání nejdøíve kladného signálu podle poètu procent
		for (; i < pwm_percents; i++) {
			PTD->PSOR |= (1 << PIN_PWM_SIGNAL_OUTPUT);

		}
		//Zbývající procenta do 100 zaslání 0 signálu
		PTD->PCOR |= (1 << PIN_PWM_SIGNAL_OUTPUT);
	}
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


