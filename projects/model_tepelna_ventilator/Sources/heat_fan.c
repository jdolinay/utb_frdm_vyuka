/*
 * heat_fan.c
 *
 * Driver for heating plant with fan which connects to UTB learning kit
 * with FRDM kl25z.
 * Uses timer TPM0.
 */

// V2 - Uses IC function on TPM0_CH0 (temperature sensor) and TPM0_CH3 (fan tachometer) for lower CPU usage during pulse measuring
//	  - Uses PWM function on TPM0_CH4 with 500 Hz frequency


#include "MKL25Z4.h"
#include "heat_fan.h"
#include <stdbool.h>

// Nastaveni casovace pro TTOF = 2 ms
#define	TPM0_TTOF_MILLIS			(2)
#define TPM0_MODULO					(TPM0_TTOF_MILLIS * 8000)
#define TPM0_PRESCALER				(0)

// Pocet impulsu na otacku ventilatoru
#define PULSES_PER_REVOLUTION		(2)
// Delka mereni poctu pulsu v ms
#define PULSES_MEASSURE_TIME_MILLIS	(500)
// Timeout pro mereni otacek v ms
#define PULSES_MEASSURE_TOUT_MILLIS	(1000)
// Prepocet PULSES_MEASSURE_TIME na pocet TOF casovace
#define PULSES_MEASSURE_TOUT_TOFS	(PULSES_MEASSURE_TOUT_MILLIS / TPM0_TTOF_MILLIS)
// Prepocet PULSES_MEASSURE_TIME na pocet TOF casovace
#define PULSES_MEASSURE_TIME_TOFS	(PULSES_MEASSURE_TIME_MILLIS / TPM0_TTOF_MILLIS)

// Cisla pinu portu
#define PIN_VYBER_CLENU_OUTPUT 		(7)
#define	PIN_PWM_SIGNAL_OUTPUT 		(4)
#define PIN_PWM_TEPLOMER_INPUT 		(0)
#define PIN_IMPULS_VENTILATOR_INPUT (3)

static void TPMInitialize(void);

// Globalni promenne pro ukladani vysledku
static volatile uint16_t g_temperature_sensor_pwm_duty;
static volatile uint32_t g_rpm_sensor_pulses;


void HEATFAN_Init(void) {

	//Povolime hodinovy signal pro port D
	SIM->SCGC5 |= (SIM_SCGC5_PORTD_MASK);

	// GPIO pins
	PORTD->PCR[PIN_VYBER_CLENU_OUTPUT] = PORT_PCR_MUX(1);			// PTD7
	PORTD->PCR[PIN_PWM_SIGNAL_OUTPUT] = PORT_PCR_MUX(4);			// TPM0_CH4
	PORTD->PCR[PIN_PWM_TEPLOMER_INPUT] = PORT_PCR_MUX(4);			// TPM0_CH0
	PORTD->PCR[PIN_IMPULS_VENTILATOR_INPUT] = PORT_PCR_MUX(4);		// TPM0_CH3

	// pin direction
	PTD->PDDR |= (1 << PIN_VYBER_CLENU_OUTPUT);
	PORTD->PCR[PIN_PWM_TEPLOMER_INPUT] |= (PORT_PCR_PS_MASK | PORT_PCR_PE_MASK); 	// Enable Pull-UP on PTD0

	// Init globalnich promennych obsluh preruseni
	g_temperature_sensor_pwm_duty = 0;
	g_rpm_sensor_pulses = 0;

	// Init casovace TPM0
	TPMInitialize();
}





// Measure the temperature from SMT160 sensor with pwm output.
// Returns temperature in tenths degrees celsius
int16_t HEATFAN_GetTemperature(void)
{
	uint16_t temperature_sensor_pwm_duty;
	// Cteni sdileneho prostredku
	__disable_irq();												// Disable interrupts
	temperature_sensor_pwm_duty = g_temperature_sensor_pwm_duty;	// Vytvoreni lokalni kopie globalni promenne
	__enable_irq();													// Enable interrupts
	// Vypocet teploty v desetinach deg.C a navrat
	return ((int16_t)((((float)temperature_sensor_pwm_duty/10000)-0.320)/0.000470));
}





// Return the speed of the fan
uint16_t HEATFAN_GetFanRPM(void)
{
	uint32_t rpm_sensor_pulses;
	// Cteni sdileneho prostredku
	__disable_irq();												// Disable interrupts
	rpm_sensor_pulses = g_rpm_sensor_pulses;						// Vytvoreni lokalni kopie globalni promenne
	__enable_irq();													// Enable interrupts
	// Vypocet RPM a navrat
	return ((uint16_t)((60 * rpm_sensor_pulses) / PULSES_PER_REVOLUTION) * 1000 / PULSES_MEASSURE_TIME_MILLIS);
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





// Set PWM duty on the output to control heater or fan
void HEATFAN_SetPWMDuty(uint8_t pwm_percents)
{
	// Limit pwm_percents to the valid range
	if (pwm_percents > 100) pwm_percents = 100;
	TPM0->CONTROLS[4].CnV = (uint32_t)pwm_percents * TPM0_MODULO / 100;
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
	// Pri clock = 8 MHz / 1 = 8000000 Hz,
	// citac napocita do 8000 za 1 ms.
	TPM0->CNT = 0;	// manual doporucuje vynulovat citac
	TPM0->MOD = TPM0_MODULO;

	// ... a nakonec nastavit pozadovane hodnoty:
	// ... delicka (prescale) = 1
	// ... interni zdroj hodinoveho signalu
	// ... povoleni generovani preruseni od TOF
	TPM0->SC = ( TPM_SC_CMOD(1) | TPM_SC_PS(TPM0_PRESCALER) | TPM_SC_TOIE_MASK);

	// Nastaveni IC na kanalech 0 a 3
	TPM0->CONTROLS[0].CnSC = 0;
	TPM0->CONTROLS[0].CnSC |= (TPM_CnSC_ELSA_MASK | TPM_CnSC_CHIE_MASK);	// Capture on Rising Edge only, Enable channel interrupts
	TPM0->CONTROLS[3].CnSC = 0;
	TPM0->CONTROLS[3].CnSC |= (TPM_CnSC_ELSB_MASK | TPM_CnSC_CHIE_MASK);	// Capture on Falling Edge only, Enable channel interrupts

	// Nastaveni PWM na kanalu 4
	TPM0->CONTROLS[4].CnSC = 0;
	TPM0->CONTROLS[4].CnSC |= (TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK);		// Edge-aligned PWM, High-true pulses
	TPM0->CONTROLS[4].CnV = 0;

	// Povoleni preruseni v NVIC
	// ...smazat pripadny priznak cekajiciho preruseni
	NVIC_ClearPendingIRQ(TPM0_IRQn);
	// ...povolit preruseni od TPM0
	NVIC_EnableIRQ(TPM0_IRQn);
	// ...nastavit prioritu preruseni: 0 je nejvysi, 3 nejnizsi
	NVIC_SetPriority(TPM0_IRQn, 2);
}






// TPM0 TOF Interrupt handler
void TPM0_IRQHandler(void)
{
	static uint8_t duty_detect_state = 0;
	static uint8_t pulse_detect_state = 0;
	static uint8_t pulse_timeout_state = 0;
	static uint32_t edge1_val, edge2_val, edge3_val;
	static uint32_t pwm_pulse_width, pwm_period;
	static uint32_t timeout_start_time, start_time, current_time = 0;
	static uint32_t rpm_sensor_pulses;
	static uint32_t chf_events_start_val, chf_events = 0;

	// Preruseni od TOF?
	if ((TPM0->SC & TPM_SC_TOF_MASK) != 0)
	{
		// Ano, vynulovani priznaku TOF
		TPM0->SC |= TPM_SC_TOF_MASK;
		// Aktualizace casu
		current_time++;
		// Timeout pro pocitani impulsu od snimace otacek
		switch (pulse_timeout_state)
		{
			case 0:
			{
				// Inicializace
				timeout_start_time = current_time;
				chf_events_start_val = chf_events;
				pulse_timeout_state = 1;
				break;
			}
			case 1:
			{
				// Vyhodnoceni poctu chf_events po PULSES_MEASSURE_TOUT_TOFS
				if ((current_time - timeout_start_time) >= PULSES_MEASSURE_TOUT_TOFS)
				{
					if ((chf_events_start_val - chf_events) == 0)
					{
						g_rpm_sensor_pulses = 0;
						pulse_detect_state = 0;
					}
					pulse_timeout_state = 0;
				}
				break;
			}
		}
	}

	// Je preruseni od udalosti na kanalu 0 (teplomer)?
	if ((TPM0->CONTROLS[0].CnSC & TPM_CnSC_CHF_MASK) != 0)
	{
		// Ano, vynulovani priznaku CHF
		TPM0->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;

		// Detekce stridy signalu z teplomeru
		switch (duty_detect_state)
		{
			case 0:
			{
				edge1_val = TPM0->CONTROLS[0].CnV;
				TPM0->CONTROLS[0].CnSC = 0;
				while((TPM0->CONTROLS[0].CnSC & (TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK)) != 0);
				TPM0->CONTROLS[0].CnSC |= (TPM_CnSC_ELSB_MASK | TPM_CnSC_CHIE_MASK);		// Prepneme na detekci sestupne hrany
				duty_detect_state = 1;
				break;
			}
			case 1:
			{
				edge2_val = TPM0->CONTROLS[0].CnV;
				TPM0->CONTROLS[0].CnSC = 0;
				while((TPM0->CONTROLS[0].CnSC & (TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK)) != 0);
				TPM0->CONTROLS[0].CnSC |= (TPM_CnSC_ELSA_MASK | TPM_CnSC_CHIE_MASK);		// Prepneme na detekci nabezne hrany
				// Vypocet sirky pulsu
				if (edge2_val < edge1_val)
				{
					// Pri mereni nastal TOF
					pwm_pulse_width = (TPM0_MODULO - edge1_val) + edge2_val;
				}
				else
				{
					pwm_pulse_width = edge2_val - edge1_val;
				}
				duty_detect_state = 2;
				break;
			}
			case 2:
			{
				edge3_val = TPM0->CONTROLS[0].CnV;
				// Vypocet periody a stridy
				if (edge3_val < edge2_val)
				{
					// Pri mereni nastal TOF
					pwm_period = (TPM0_MODULO - edge2_val) + edge3_val + pwm_pulse_width;
				}
				else
				{
					pwm_period = edge3_val - edge2_val + pwm_pulse_width;
				}
				g_temperature_sensor_pwm_duty = (uint16_t)(10000UL * pwm_pulse_width / pwm_period);			// PWM duty v setinach procenta
				duty_detect_state = 0;
				break;
			}
		}
	}

	// Je preruseni od udalosti na kanalu 3 (snimac otacek)?
	if ((TPM0->CONTROLS[3].CnSC & TPM_CnSC_CHF_MASK) != 0)
	{
		// Ano, vynulovani priznaku CHF
		TPM0->CONTROLS[3].CnSC |= TPM_CnSC_CHF_MASK;

		// Pocitani CHF udalosti pro timeout
		chf_events++;

		// Pocitani impulsu od snimace po dobu 1000 ms
		switch (pulse_detect_state)
		{
			case 0:
			{
				// Init noveho mereni, ulozeni pocatecniho casu mereni
				start_time = current_time;
				rpm_sensor_pulses = 0;
				pulse_detect_state = 1;
				break;
			}
			case 1:
			{
				if ((current_time - start_time) < PULSES_MEASSURE_TIME_TOFS)
				{
					rpm_sensor_pulses++;
				}
				else
				{
					// Pocitani pulsu dokonceno
					g_rpm_sensor_pulses = rpm_sensor_pulses;
					pulse_detect_state = 0;
				}
				break;
			}
		}
	}
}




