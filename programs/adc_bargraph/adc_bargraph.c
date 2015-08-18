/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Analogove-Digitalni prevodnik.
 * Program ukazuje ziskani hodnoty z A/D prevodniku.
 * Napeti na vstupu A/D prevodniku se nastavuje potenciometrem na kitu.
 * Podle velikosti napeti se rozsviti 0 az 4 LED na kitu.
 *
 * Uzitecne informace:
 *
 *
 */

#include "MKL25Z4.h"

void delay(void);
void ADCInit(void);
uint32_t ADCCalibrate(void);

int main(void)
{
	// TODO: vyuzit asi driver GPIO pro LED

	// Inicializace A/D prevodniku
	ADCInit();



    /* Never leave main */
    return 0;
}

// Inicializuje A/D prevodnik na....
void ADCInit(void)
{
	// Povolit hodinovy signal pro ADC
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;


	/* Note1: The SC1 and result registers may have multiple instances (A and B in case of KL25).
	 in the CMSIS definitions there is an array for these registers. We use A, i.e. SC1[0] */
	/* Note 2: Write to SC1A starts a conversion if the channel bit field is not all 1s.
	 * Write to SC1B does not start conversion (it must be hw triggered, not sw triggered)*/

	// Zakazeme preruseni, nastavime kanal 31 = A/D prevodnik vypnut, select single-ended mode */
	ADC0->SC1[0] =  ADC_SC1_ADCH(AIN_ADC_DISALED);

	// Vyber hodinoveho signalu, preddelicky a rozliseni
	ADC0->CFG1 = ADC_CFG1_ADICLK(WMSF_ADC_CLOCK)
		| ADC_CFG1_ADIV(WMSF_ADC_PRESCALER)
		| ADC_CFG1_MODE(WMSF_ADC_RESOLUTION);

	/* ADC A or B, long sample time selection */
	ADC0->CFG2 = 0;	/* default values: ADxxa selected, longest sample time 24 ADCK total */

	/* Select reference, triggering, compare mode */
	ADC0->SC2 = 0;	/* default values: sw trigger, default reference=VREFH,VREFL pins, ... */

	ADC0->SC3 = 0;	/* default values, no averaging */

	// TODO: zde procest i kalibraci? spis ne viz musi se pak znova volat init...

}

/* Kalibrace ADC
 * Kod prevzat z ukazkoveho kodu pro FRDM-KL25Z.
 * Pri chybe kalibrace vraci 1, pri uspechu vraci 0 */
uint32_t ADCCalibrate(void)
{
	 unsigned short cal_var;

	  ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;	/* Enable Software Conversion Trigger for Calibration Process */
	  ADC0->SC3 &= ( ~ADC_SC3_ADCO_MASK & ~ADC_SC3_AVGS_MASK );    /* set single conversion, clear avgs bitfield for next writing */

	  ADC0->SC3 |= ( ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(AVGS_32) ); /* turn averaging ON and set desired value */
	  //ADC_SC3_REG(adcmap) |= ( ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(AVGS_32) );

	  ADC0->SC3 |= ADC_SC3_CAL_MASK;      /* Start CAL */
	  //ADC_SC3_REG(adcmap) |= ADC_SC3_CAL_MASK ;      // Start CAL

	  while ( !WMSF_ADCA_COMPLETE(adc->reg) )
		  ; /* Wait calibration end */
	  //while ( (ADC_SC1_REG(adcmap,A) & ADC_SC1_COCO_MASK ) == COCO_NOT ); // Wait calibration end

	  if ( (ADC0->SC3 & ADC_SC3_CALF_MASK) != 0 )
		  return 1;
	  /*if ((ADC_SC3_REG(adcmap)& ADC_SC3_CALF_MASK) == CALF_FAIL )
	  {
	   return(1);    // Check for Calibration fail error and return
	  }*/

	  // Calculate plus-side calibration
	  cal_var = 0;
	  cal_var =  adc->reg->CLP0;	//ADC_CLP0_REG(adcmap);
	  cal_var += adc->reg->CLP1;	// ADC_CLP1_REG(adcmap);
	  cal_var += adc->reg->CLP2;	//ADC_CLP2_REG(adcmap);
	  cal_var += adc->reg->CLP3;	//ADC_CLP3_REG(adcmap);
	  cal_var += adc->reg->CLP4;	//ADC_CLP4_REG(adcmap);
	  cal_var += adc->reg->CLPS;	// ADC_CLPS_REG(adcmap);

	  cal_var = cal_var/2;
	  cal_var |= 0x8000; // Set MSB
	  ADC0->PG = ADC_PG_PG(cal_var);
	  //ADC_PG_REG(adcmap) = ADC_PG_PG(cal_var);


	  // Calculate minus-side calibration
	  cal_var = 0;
	  cal_var =  ADC0->CLM0;	//ADC_CLM0_REG(adcmap);
	  cal_var += ADC0->CLM1;	//ADC_CLM1_REG(adcmap);
	  cal_var += ADC0->CLM2;	//ADC_CLM2_REG(adcmap);
	  cal_var += ADC0->CLM3;	//ADC_CLM3_REG(adcmap);
	  cal_var += ADC0->CLM4;	//ADC_CLM4_REG(adcmap);
	  cal_var += ADC0->CLMS;	//ADC_CLMS_REG(adcmap);

	  cal_var = cal_var/2;
	  cal_var |= 0x8000; // Set MSB
	  ADC0->MG = ADC_MG_MG(cal_var);
	  //ADC_MG_REG(adcmap) = ADC_MG_MG(cal_var);

	  ADC0->SC3 &= ~ADC_SC3_CAL_MASK ;
	  //ADC_SC3_REG(adcmap) &= ~ADC_SC3_CAL_MASK ; /* Clear CAL bit */

	  return 0;
}

// Kratke cekani
void delay(void)
{
	uint32_t n = 700000;
	while( n -- )
		;
}





////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
