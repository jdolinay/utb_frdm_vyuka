/*
 * Sample program for MCU programming course
 * Analog to Digital Converter (ADC)
 * The program shows how to obtain data from ADC.
 * Input voltage on the ADC input can be changed using potentiometer on the board.
 * According to the voltage, 0 to 3 LEDs will turn on (like a bargraph)
 *
 * Intormation
 * Potentiometer is on pin PTC2 (which is channel 11 of the ADC)
 *
 * Clock speed of ADC must be between 2 and 12 MHz if the resolution is 16-bit.
 * For resolution <= 13 bit: 1 - 18 MHz.
 * For calibrating clock <= 4 MHz is recommended.
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"

void delay(void);
void ADCInit(void);
uint32_t ADCCalibrate(void);

int main(void)
{

	// GPIO driver is used for LEDs
	GPIO_Initialize();

	// Set LED pins as outputs
	pinMode(LD1, OUTPUT);
	pinMode(LD2, OUTPUT);
	pinMode(LD3, OUTPUT);

	// Turn off all LEDs
	pinWrite(LD1, HIGH);
	pinWrite(LD2, HIGH);
	pinWrite(LD3, HIGH);


	// Initialize ADC
	ADCInit();

	// Calibrate the ADC and initialize again
	// The ADC should be calibrated after each reset.
	// New initialization is needed because the calibration
	// changes the ADC settings.
	ADCCalibrate();
	ADCInit();

	// Configure the pin where potentiometer is connected
	// to be input of ADC: PTC2 to function ALT0.
	// 1. Enable clock for port C
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	// 2. Set pin function to ALT0 = ADC input
	PORTC->PCR[2] = PORT_PCR_MUX(0);


	while (1) {

		// Start conversion on channel 11.
		// Because all the settings in SC1 register are 0, we can just write the number
		// of the channel into the register. A better way would be to use bit manipulation
		// to set just the bits which affect channel number without overwriting the whole register.
		ADC0->SC1[0] = ADC_SC1_ADCH(11);

		// Wait for conversion completion
		while ( (ADC0->SC1[0] & ADC_SC1_COCO_MASK) == 0 )
			;

		// Store result
		uint16_t vysledek = ADC0->R[0];

		// Turn all LEDs off
		pinWrite(LD1, HIGH);
		pinWrite(LD2, HIGH);
		pinWrite(LD3, HIGH);

		// Process the result: with 10-bit result the value is 0 - 1023
		if (vysledek > 255) {
			// LED1 on
			pinWrite(LD1, LOW);
		}

		if (vysledek > 510) {
			// LED2 on
			pinWrite(LD2, LOW);
		}

		if (vysledek > 765) {
			// LED3 on
			pinWrite(LD3, LOW);
		}

		delay();
	}	// while



    /* Never leave main */
    return 0;
}

/*	ADCInit
    Initialize ADC
    Set clock source to bus clock and prescaler to 8,
    resolution 10-bit,...
*/
void ADCInit(void)
{
	// Enable clock for ADC
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

	// Disable interrupt, set channel 31 which means ADC off,
	// otherwise write to the register would start conversion
	// Select single-ended mode
	ADC0->SC1[0] =  ADC_SC1_ADCH(31);

	// Select clock source, prescaler and resolution
	// Set clock <= 4 MHz as recommended for calibration.
	// With CPU clock 58 MHz max, the bus clock is 24 MHz; with
	// prescaler = 8 the clock for ADC will be 24 / 8 = 3 MHz.
	ADC0->CFG1 = ADC_CFG1_ADICLK(0)		/* ADICLK = 0 -> bus clock */
		| ADC_CFG1_ADIV(3)				/* ADIV = 3 -> clock/8 */
		| ADC_CFG1_MODE(2);				/* MODE = 2 -> rozliseni 10-bit */

	// Write default values into other registers:
	// Set the channel set "a", and default, longest conversion time (24 clocks)
	ADC0->CFG2 = 0;

	// Software trigger of conversion, default reference voltage
	ADC0->SC2 = 0;

	// Hardware averaging disabled
	ADC0->SC3 = 0;	/* default values, no averaging */

}

/*
  ADCCalibrate
  Calibrate the ADC.
  The code is from sample code for FRDM-KL25Z.
  It returns 1 if calibration error occured, 0 if ok.
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

// short delay
void delay(void)
{
	uint32_t n = 100000;
	while( n -- )
		;
}





////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
