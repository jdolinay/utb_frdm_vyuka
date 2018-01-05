/**
 * @file stm160_kl25.c
 * @brief driver for temperature sensor SMT 160-30 for Freedom board FRDM-KL25Z
 * 
 * @note
 * For input we can use 2 pins:
 * PTD3 (TMP0_CH3) - input capture
 * PTD4  - used as GPIO to find out if rising or falling
 * 			edge generated the timer channel interrupt. 
 *
 * Principle:
 * In theory we could use 1 pin (input capture channel) and switch detection between 
 * rising and falling. But it is not good to do this in ISR, attempts failed :(
 * Also, switching the same pin between timer channel and GPIO to detect the
 * level on the pin in ISR did not work, seems to generate false interrupts.
 * 
 * Times: 
 * Typical signal is 3 kHz (period is about 330 us) 
 * and the pulse is about 150 us for temperature 20 C
 * so there is about 50% duty. 
 * With CPU clock 20 MHz the timer tick is 0.05 us, max value is
 * reached in about 3.3 ms and we get value about 3000 for pulse
 * and 6000 for period. This seems OK. 
 * With max CPU 40 MHz the values would be 6000 and 12000, still OK.
 * For e.g. 5 MHz CPU the tick would be 0.2 us and we get 750 for
 * pulse - still ok.
 *  
*/
#include "MKL25Z4.h"
//#include "derivative.h"	// jd
#include "smt160_kl25.h"

// TODO: move to generic header!
/*!< Macro to enable all interrupts. */
#define EnableInterrupts __asm ("CPSIE  i")

/*!< Macro to disable all interrupts. */
#define DisableInterrupts __asm ("CPSID  i")


/* Maximum values for signal period - for sanity check 
  the freq. is between 1 kHz and 4 kHz, so the period is
  between 1 ms and 250 us. 
  We should not set strict limits if we do not know the CPU speed
  but it is reasonable to assume the timer clock is max 40 MHz and
  min 1 MHz (1 us to 0.025 us per timer tick 
  Min period: 250 us --> SMT160_MIN_PERIOD = 250 for 1 us tick 
  Max period: 1 ms --> SMT160_MAX_PERIOD = 40000 */
#define		SMT160_MAX_PERIOD	(40000)
#define		SMT160_MIN_PERIOD	(250)

/* how many samples (periods) we sum before computing average */
#define		SMT160_SUM_NUMBER	(24)

/** Pin used by this driver as GPIO to detect if falling or rising edge
 * Note that we assume PTD4; Changing just the number in #define is
 * not enough,see its use in code.
 * */
#define	SMT160_INPUT_PIN_NO			(4)

volatile uint16_t gsmt_start;	/* time of pulse start */
volatile uint16_t gsmt_pulse;	/* averaged pulse length in ticks */
volatile uint16_t gsmt_period;	/* averaged signal period */

volatile uint16_t gsmt_tmp_pulse;	/* pulse length in ticks */
volatile uint32_t gsmt_pulses;	/* sum of pulses */
volatile uint32_t gsmt_period_sum;	/* sum of periods */
volatile uint32_t gsmt_sumcnt;		/* sum counter */



/** init the timer TPM1 for measuring temperature using input capture
 * 
 */
void smt160_init()
{
	int prio = 2;	// priorita interruptu; 0 az 3, 0 = max
	
	/* Initialize our global variables */
	gsmt_start = 0;
	gsmt_pulse = 0;
	gsmt_period = 0;
	gsmt_tmp_pulse = 0;
	gsmt_pulses = 0;
	gsmt_period_sum = 0;	
	gsmt_sumcnt = 0;
	
	/* Enable Port A clock (pins used for input capture) */
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
	
	/* enable input pins for TPM1 channels 0 and 1 
	  The timer channel is Alt 3 function of the pins */
	/* Model je na PTD3 a PTD4 = TPM0 ch 3 a 4*/
	PORTD_PCR3 = PORT_PCR_MUX(4);		/* PTD3 = channel 3 for TPM3 */
	PORTD_PCR4 = PORT_PCR_MUX(1);		/* using PTD4 as GPIO for deciding falling/rising */
	GPIOD_PDDR &= ~(1 << SMT160_INPUT_PIN_NO);			/* set pin to input mode */
	
	/* Enable clock for timer TMP0 */
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
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

	/* and clock source */
	//SIM_SOPT2 |= SIM_SOPT2_TPMSRC(1); // select the PLLFLLCLK as TPM clock source
	
	/*  Set timer to input capture mode etc. */
	// clock je 21 MHz prescaler je 1
	TPM0_SC = TPM_SC_CMOD(1) | TPM_SC_PS(0);	// CMOD = 01, prescaler = 1
	


	// Input capture config
	// Version with 1 pin used; wait for rising edge
	TPM0_C3SC = 0;	/* clear any pending interrupt and set all values to default */
	/* input capture on rising and falling edge + enable channel interrupt */
	TPM0_C3SC |= TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_CHIE_MASK;
		
	
	// Enable the interrupt
	// Preruseni je treba povolit take v NVIC
	// ...smazat pripadny priznak cekajiciho preruseni
	NVIC_ClearPendingIRQ(TPM0_IRQn);
	// ...povolit preruseni od TPM0
	NVIC_EnableIRQ(TPM0_IRQn);
	// ...nastavit prioritu preruseni: 0 je nejvysi, 3 nejnizsi
	NVIC_SetPriority(TPM0_IRQn, 2);

	
}

/** Get current temperature in C x 100
 *  For example: temperature 22.5 C is returned as 2250.
 *  @returns temperature in C x 100 (22.5 C is returned as 2250)
 */
uint16_t smt160_get_temp()
{
	uint32_t pulse, period, duty ;
	
	/* Get the values with disabled timer interrupt */
	TPM0_C3SC &= ~TPM_CnSC_CHIE_MASK;	/* disable interrupt */
	pulse = gsmt_pulse;
	period = gsmt_period;
	
	/* We might have missed some pulses, restart the measuring to state of waiting for 
	  new pulse */
	gsmt_start = 0;	
	gsmt_tmp_pulse = 0;
	TPM0_C3SC |= TPM_CnSC_CHIE_MASK;	/* enable interrupt again */
	
	/* Compute temperature */
	/* DC = 0.32 + 0.0047 x T 
	  -->  T = (DC - 0.32) / 0.0047 (with DC 0.5 is 50%)
	  T =  DC_PERCENT / 0.47 - 68.0851	
	  100T = 100* (DC_PERCENT / 0.47) - 6809
	  100T = 100* (DC_PERCENT*100 / 47) - 6809 
	  100T = 10000 * DC_PERCENT / 47 - 6809 
	  
	  Example: DC = 0.32 which is T = 0C
	  100T = 100 * ((32*100) / 47) - 6809 =  0
	  DC = 0,93 which is T = 130 °C
	  100T = 100 * ((93*100)/47) - 6909 = 12978 which is 129.78 °C :)
	 */
	if ( pulse < period )
	{		
		// works but resolution is small 
		//duty = (pulse * 100) / period;				
		//pulse = 10000 * duty / 47 - 6809;
		
		duty = (pulse * 1000) / period;		
		/* using pulse to store temperature */
		pulse = 10000 * duty / 470 - 6809;
	}
	else
		pulse = 0;	/* invalid pulse or period */
	
	return (uint16_t)pulse;
}

/* TMP0 interrupt handler
 * We need to read TPM registers to find out which TMP0 interrupt this is;
 *  
 */
void TPM0_IRQHandler()
{
	volatile uint16_t tmp;
	
	// Channel interrupt?
	if ( (TPM0_C3SC & TPM_CnSC_CHF_MASK) != 0 )
	{		
    	// channel 0 interrupt occurred
		TPM0_C3SC |= TPM_CnSC_CHF_MASK;		// clear the interrupt flag
		       		
		// is it rising or falling edge?
		// We check separate pin which is used as GPIO and connected to the
		// SMT160 output as well.
		if ( (GPIOD_PDIR & (1 << SMT160_INPUT_PIN_NO)) != 0 )
		{
			/* RISING edge detected */
			
			if ( gsmt_start == 0 )
			{
				/* start of pulse - first after our init  */
				gsmt_start = TPM_CnV_VAL(TPM0_C3V);
			}
			else
			{
				/* end of period */
				tmp = TPM_CnV_VAL(TPM0_C3V);
				if ( tmp > gsmt_start )
					tmp = tmp - gsmt_start;
				else	/* overflow of counter while measuring */
					tmp = (0xffff - gsmt_start) + tmp;
				
				//gsmt_period = tmp;	/* save period */
				/* save to sums if valid */
				if ( tmp > SMT160_MIN_PERIOD && tmp < SMT160_MAX_PERIOD
					&& gsmt_tmp_pulse > SMT160_MIN_PERIOD && gsmt_tmp_pulse < SMT160_MAX_PERIOD )
				{
					// pokusne vyrazeno prumerovani...
					//gsmt_period = tmp;
					//gsmt_pulse = gsmt_tmp_pulse;
					
					gsmt_period_sum += tmp;
					gsmt_pulses += gsmt_tmp_pulse;
					gsmt_sumcnt++;
					if ( gsmt_sumcnt >= SMT160_SUM_NUMBER )
					{
						gsmt_period = gsmt_period_sum / gsmt_sumcnt;
						gsmt_pulse = gsmt_pulses / gsmt_sumcnt;
						gsmt_sumcnt = 0;
						gsmt_period_sum = 0;
						gsmt_pulses = 0;
					}
					
				}
				
				/* end of period means also start of a new period */
				gsmt_start = TPM_CnV_VAL(TPM0_C3V);
			}
			
			      
		}
		else
		{
			/* falling edge detected */
			/* = end of pulse. Wait for rising edge to measure the period */
			if ( gsmt_start != 0 )
			{
				/* end of pulse */
				tmp = TPM_CnV_VAL(TPM0_C3V);
				if ( tmp > gsmt_start )
					tmp = tmp - gsmt_start;
				else	/* overflow of counter while measuring */
					tmp = (0xffff - gsmt_start) + tmp;
				gsmt_tmp_pulse = tmp;
				//gsmt_period = 0;	/* invalidate the period */				
			}
						
				
		}
				
	
	}  // end of channel 0 interrupt
	
}

