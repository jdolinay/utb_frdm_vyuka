/*
 * drv_spi.c
 *
 *  Created on: 19. 11. 2021
 *      Author: student
 */

// Driver pro spravnou funkci vyzaduje nastaveni CLOCK_SETUP = 1


#include "MKL25Z4.h"


void spi_init(void)
{
	SIM->SCGC4 |= SIM_SCGC4_SPI0_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
	PORTC->PCR[5] = PORT_PCR_MUX(2);
	PORTC->PCR[6] = PORT_PCR_MUX(2);

	SPI0->C1 &= ~SPI_C1_SPIE_MASK;				// SPIE disable
	SPI0->C1 &= ~SPI_C1_SPTIE_MASK;				// SPTI disable
	SPI0->C1 |= SPI_C1_MSTR_MASK;				// Master mode
	SPI0->C1 &= ~SPI_C1_CPOL_MASK;				// CPOL = 0
	SPI0->C1 &= ~SPI_C1_CPHA_MASK;				// CPHA = 0
	SPI0->C1 &= ~SPI_C1_SSOE_MASK;				// SS pin is GPIO
	SPI0->C1 &= ~SPI_C1_LSBFE_MASK;				// TX MSB first

	SPI0->C2 = 0;
	SPI0->BR = 0;								// SPI Baudrate = fbus / 2

	SPI0->C1 |= SPI_C1_SPE_MASK;				// SPI system enable
}



void spi_send_byte(uint8_t value)
{
	while((SPI0->S & SPI_S_SPTEF_MASK) == 0);	// Cekej na uvolneni vysilaciho bufferu
	SPI0->D = value;
	while((SPI0->S & SPI_S_SPTEF_MASK) == 0);	// Cekej na uvolneni vysilaciho bufferu
}
