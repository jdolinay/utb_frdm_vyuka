/*
 * drv_DA_mod.c
 *
 *  Created on: 19. 11. 2021
 *      Author: student
 */

#include "MKL25Z4.h"
#include "drv_DA_mod.h"
#include "drv_spi.h"


#define	MCP4922_CS_PIN		(6)
#define	MCP4922_LDAC_PIN	(7)



void DA_Init(void)
{
	spi_init();

	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;					// PORTD clock enable
	PORTD->PCR[MCP4922_CS_PIN] = PORT_PCR_MUX(1);		// GPIO
	PORTD->PCR[MCP4922_LDAC_PIN] = PORT_PCR_MUX(1);		// GPIO
	GPIOD->PDOR |= (1 << MCP4922_CS_PIN);				// MCP4922_CS_PIN = 1
	GPIOD->PDOR &= ~(1 << MCP4922_LDAC_PIN);			// MCP4922_LDAC_PIN = 0
	GPIOD->PDDR |= (1 << MCP4922_CS_PIN);				// output direction
	GPIOD->PDDR |= (1 << MCP4922_LDAC_PIN);				// output direction
}


void DA_Update_Channel_A(uint16_t value)
{
	uint8_t txval1 = 0b00110000;	// ~A/B = 0, BUF = 0, ~GA = 1, ~SHDN = 1
	uint8_t txval2 = 0;

	if (value > 4095) value = 4095;

	txval1 = txval1 + (uint8_t)(value >> 8);
	txval2 = txval2 + (uint8_t)(value & 0xff);

	GPIOD->PCOR |= (1 << MCP4922_CS_PIN);
	spi_send_byte(txval1);
	spi_send_byte(txval2);
	GPIOD->PSOR |= (1 << MCP4922_CS_PIN);
}


void DA_Update_Channel_B(uint16_t value)
{
	uint8_t txval1 = 0b10110000;	// ~A/B = 1, BUF = 0, ~GA = 1, ~SHDN = 1
	uint8_t txval2 = 0;

	if (value > 4095) value = 4095;

	txval1 = txval1 + (uint8_t)(value >> 8);
	txval2 = txval2 + (uint8_t)(value & 0xff);

	GPIOD->PCOR |= (1 << MCP4922_CS_PIN);
	spi_send_byte(txval1);
	spi_send_byte(txval2);
	GPIOD->PSOR |= (1 << MCP4922_CS_PIN);
}
