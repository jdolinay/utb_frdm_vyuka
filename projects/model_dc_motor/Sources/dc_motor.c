/*
 * DRV_motor.c
 *
 *  Created on: Nov 21, 2016
 *      Author: student
 */
#include "drv_lcd.h"
#include "MKL25Z4.h"
#include "stdbool.h"

#define		Speed		(2) // cteni

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

void DCMOTOR_Init()
{
	// rychlost
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[Speed] = PORT_PCR_MUX(1);
	PTD->PDDR &= ~(1 << Speed);


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

// yastavi motor
void DCMOTOR_SpinOFF()
{
	roll_OFF();
}


int DCMOTOR_LapCheck() {
	int help = 0;
	if (one == false) {
		if (!((PTD->PDIR & (1 << Speed)) == 0)) {
			if (i == 3) {
				// vraceni casu nebo otacek z vypoctu
				i = 1;
				help++;
			} else
				i++;

			one = true;
		}
	} else {
		if (((PTD->PDIR & (1 << Speed)) == 0)) {
			one = false;
		}
	}
	return help;
}


