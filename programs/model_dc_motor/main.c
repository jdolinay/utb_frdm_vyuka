/*
 * Ukazka pro model DC motorku pripojeny k vyvojovemu kitu UTB.
 */

#include "MKL25Z4.h"
#include "drv_lcd.h"
#include "dc_motor.h"

static int i = 0;

int main(void)
{

    LCD_initialize();
    LCD_clear();

    DCMOTOR_Init();
    DCMOTOR_setDirection(0);

    for (;;) {
        i++;
    }

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
