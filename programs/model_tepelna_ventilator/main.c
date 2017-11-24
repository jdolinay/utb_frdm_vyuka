/*
 Example program for heat plant with fan

 Zobrazuje teplotu na displeji. Pri prekroceni 40C vypne topeni, jinak zapne.
 Stav topeni je na displeji.
 */

#include "MKL25Z4.h"
#include "drv_lcd.h"
#include "drv_systick.h"
#include "heat_fan.h"
#include <stdio.h>

static int i = 0;
char buff[20];

int main(void)
{
	LCD_initialize();
	LCD_clear();
	LCD_puts("Start");

	SYSTICK_initialize();

	HEATFAN_Init();
	HEATFAN_CtrlSignalSel(HEATFAN_Heater);
	HEATFAN_DoPWMPulse(0);

    for (;;) {
    	int teplota = HEATFAN_GetTemperature();
    	sprintf(buff, "%d", teplota);
    	LCD_clear();
    	LCD_puts(buff);
    	LCD_set_cursor(2,1);

    	int duty = 0;
    	if ( teplota < 40 ) {
    		duty = 100;
    		LCD_puts("ON");
    	} else {
    		LCD_puts("OFF");
    	}

    	int n = 1000;
    	while ( n-- > 0 ) {
    		HEATFAN_DoPWMPulse(duty);
    	}
    	//SYSTICK_delay_ms(1000);
    }
    /* Never leave main */
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
