/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "MKL25Z4.h"
#include "drv_audio_mod.h"
#include "drv_lcd.h"
#include <stdio.h>

void delay(void);


int main(void)
{
    /* Write your code here */
	uint8_t volume = 63;	// Minimalni hlasitost
	char tbuff[5];


	audio_Init();			// Init I2C a audio modulu (vstup 0, min. hlasitost)
	LCD_initialize();		// Init displeje

	LCD_clear();
	LCD_set_cursor(1,1);
	LCD_puts("Hlasitost:");

	// Init GPIO
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;		// clock
	PORTA->PCR[4] = PORT_PCR_MUX(1);		// MUX GPIO
	PORTA->PCR[5] = PORT_PCR_MUX(1);		// MUX GPIO
	GPIOA->PDDR &= ~(1 << 4);				// PTA4 vstup
	GPIOA->PDDR &= ~(1 << 5);				// PTA5 vstup

	// Zvysovani a snizovani hlasitosti pomoci tlacitek SW1 a SW2
	// Pozor, 0 = max. hlasitost, 63 = min. hlasitost
	while(1)
	{
		if ((GPIOA->PDIR & (1 << 4)) == 0)
		{
			// Stisknuto tlacitko SW1 -> zvyseni hlasitosti
			if (volume > 0) volume --;
			audio_Set_volume(volume);
		}
		if ((GPIOA->PDIR & (1 << 5)) == 0)
		{
			// Stisknuto tlacitko SW2 -> snizeni hlasitosti
			if (volume < 63) volume ++;
			audio_Set_volume(volume);
		}

		sprintf(tbuff, "%2d", volume);
		LCD_set_cursor(1,12);
		LCD_puts(tbuff);

		delay();
	}


    /* This for loop should be replaced. By default this loop allows a single stepping. */
    for (;;) {
    }
    /* Never leave main */
    return 0;
}


void delay (void)
{
	uint32_t i;
	for (i=0; i<100000; i++);
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
