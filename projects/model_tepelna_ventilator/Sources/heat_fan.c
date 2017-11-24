/*
 * heat_fan.c
 *
 * Driver for heating plant with fan which connects to UTB learning kit
 * with FRDM kl25z.
 */

#include "MKL25Z4.h"
#include "heat_fan.h"
#include <stdbool.h>
#include "drv_systick.h"

#define PIN_VYBER_CLENU_OUTPUT (7)
#define	PIN_PWM_SIGNAL_OUTPUT (4)
#define PIN_PWM_TEPLOMER_INPUT (0)
#define PIN_IMPULS_VENTILATOR_INPUT (3)

void HEATFAN_Init(void) {
	//Systick to measure rpm
	SYSTICK_initialize();

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
	start = SYSTICK_millis();
	for (i = 0; i < 1000; i++) {
		if ((PTD->PDIR & (1 << PIN_IMPULS_VENTILATOR_INPUT)) == 0) {
			pocetImpulzu++;
		}
	}
	konec = SYSTICK_millis();
	uint32_t cas = konec - start;
	return (uint16_t) (pocetImpulzu / cas * 1000);
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

