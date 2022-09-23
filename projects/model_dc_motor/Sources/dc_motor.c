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


// Nastaveni casovace pro TTOF = 2 ms
#define	TPM0_TTOF_MILLIS			(2)
#define TPM0_MODULO					(TPM0_TTOF_MILLIS * 8000)
#define TPM0_PRESCALER				(0)

// Pocet impulsu na otacku ventilatoru
#define PULSES_PER_REVOLUTION		(3)
// Delka mereni poctu pulsu v ms
#define PULSES_MEASSURE_TIME_MILLIS	(500)
// Timeout pro mereni otacek v ms
#define PULSES_MEASSURE_TOUT_MILLIS	(1000)
// Prepocet PULSES_MEASSURE_TIME na pocet TOF casovace
#define PULSES_MEASSURE_TOUT_TOFS	(PULSES_MEASSURE_TOUT_MILLIS / TPM0_TTOF_MILLIS)
// Prepocet PULSES_MEASSURE_TIME na pocet TOF casovace
#define PULSES_MEASSURE_TIME_TOFS	(PULSES_MEASSURE_TIME_MILLIS / TPM0_TTOF_MILLIS)

#define	SPEED				(2) // snimani otacek

#define MOTOR_DIR           (6) // smer
#define	MOTOR_DIR_MASK		(1 << MOTOR_DIR)
#define	MOTOR_DIR_TOGGLE()	PTD->PTOR |= MOTOR_DIR_MASK
#define	MOTOR_DIR_ON()		PTD->PCOR |= MOTOR_DIR_MASK
#define	MOTOR_DIR_OFF()		PTD->PSOR |= MOTOR_DIR_MASK


#define MOTOR_SPIN          (4) // toceni
#define	MOTOR_SPIN_MASK		(1 << MOTOR_SPIN)
#define	MOTOR_SPIN_TOGGLE()	PTD->PTOR |= MOTOR_SPIN_MASK
#define	MOTOR_SPIN_ON()		PTD->PSOR |= MOTOR_SPIN_MASK
#define	MOTOR_SPIN_OFF()	PTD->PCOR |= MOTOR_SPIN_MASK


// Globalni promenne pro ukladani vysledku
static volatile uint32_t g_rpm_sensor_pulses;


static void TPMInitialize(void);



void DCMOTOR_Init()
{
	// Snimac otacek PTD2/TPM0CH2
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[SPEED] = PORT_PCR_MUX(4);	// TPM0CH2
	PTD->PDDR &= ~(1 << SPEED);

	// Rizeni smeru otaceni PTD6
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[MOTOR_DIR] = PORT_PCR_MUX(1);
	MOTOR_DIR_OFF();
	PTD->PDDR |= MOTOR_DIR_MASK;

	// PWM signal pro motor PTD4/TPMOCH4
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[MOTOR_SPIN] = PORT_PCR_MUX(4); //TPMOCH4
	PTD->PDDR |= MOTOR_SPIN_MASK;

	TPMInitialize();
}



// Nastaveni smeru otaceni. Vstup je 0 nebo 1.
// 0 = rele sepnuto
// 1 = rele vypnuto
void DCMOTOR_setDirection(char dir)
{
	if(dir == 1)
	{
		MOTOR_DIR_ON();
	}
	else
	{
		MOTOR_DIR_OFF();
	}
}





void DCMOTOR_setRpm(uint8_t n)
{
	TPM0->CONTROLS[MOTOR_SPIN].CnV = TPM0_MODULO * (uint32_t)n / 100;
}






// Vraci otacky za minutu
uint16_t DCMOTOR_GetRpm()
{
	uint32_t rpm_sensor_pulses;
	// Cteni sdileneho prostredku
	__disable_irq();												// Disable interrupts
	rpm_sensor_pulses = g_rpm_sensor_pulses;						// Vytvoreni lokalni kopie globalni promenne
	__enable_irq();													// Enable interrupts
	// Vypocet RPM a navrat
	return ((uint16_t)((60 * rpm_sensor_pulses) / PULSES_PER_REVOLUTION) * 1000 / PULSES_MEASSURE_TIME_MILLIS);
}





static void TPMInitialize(void)
{
	// Nastaveni casovace pro ttof = 1 ms (ftof = 1 kHz)
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

	// Nastaveni IC na kanalu 2 - snimac otacek
	TPM0->CONTROLS[2].CnSC = 0;
	TPM0->CONTROLS[2].CnSC |= (TPM_CnSC_ELSA_MASK | TPM_CnSC_CHIE_MASK);	// Capture on Rising Edge only, Enable channel interrupts

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





// TPM0 Interrupt handler
void TPM0_IRQHandler(void)
{
	static uint8_t duty_detect_state = 0;
	static uint8_t pulse_detect_state = 0;
	static uint8_t pulse_timeout_state = 0;
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

	// Je preruseni od udalosti na kanalu 2 (snimac otacek)?
	if ((TPM0->CONTROLS[2].CnSC & TPM_CnSC_CHF_MASK) != 0)
	{
		// Ano, vynulovani priznaku CHF
		TPM0->CONTROLS[2].CnSC |= TPM_CnSC_CHF_MASK;

		// Pocitani CHF udalosti pro timeout
		chf_events++;

		// Pocitani impulsu od snimace po dobu PULSES_MEASSURE_TIME_TOFS
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




