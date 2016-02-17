/*
 * Sample program for MCU programming course
 * Serial communication (UART).
 * The program shows how to send and receive characters via UART0 module.
 * It sends text "hello" to serial line. If "s" character is received, it stops/starts the
 * sending of hello.
 * Baud rate 9600 bd.
 *
 * NOTE: In project properties > compiler > preprocesor must be defined: CLOCK_SETUP=1
 * so that the CPU runs at 48 MHz!
 *
 * Information
   KL25Z has 3 UART modules: UART0, UART1 a UART2. Module UART0 is connected also
   	   to the pins for USB communication; so it is available via virtual serial port
   	   over the USB cable connected to the SDA connector on FRDM-KL25Z board.
   	   On the desktop PC you will see it as "Open SDA - CDC Serial Port".


 */

#include "MKL25Z4.h"
#include "drv_uart.h"

void delay_ms(uint32_t millis);

volatile uint32_t g_delaycnt;

int main(void)
{
	uint8_t vypis = 1;
	char c;

	// Configure SysTick timer to generate interrupt every millisecond (ms)
	// used in delay_ms funciton below
	SysTick_Config(SystemCoreClock / 1000u );

	UART0_Initialize(BD9600);

	while ( 1 )
	{
		// If output is enabled, print hello
		if (vypis )
			UART0_puts("Hello!\n");

		// If we receive "s" character from PC, toggle the output enable
		// Test if there is a received character in the UART buffer
		if ( UART0_Data_Available() )
		{
			// if yes, read it
			c = UART0_getch();
			// send if back (echo)
			UART0_putch(c);

			// check what it is
			if ( c == 's' )
			{
				vypis = !vypis;	// toggle the value of "vypis"
				UART0_puts("\nPrikaz prijat.\n");// print confirmation
			}
		}

		delay_ms(1000);
	}

    /* Never leave main */
    return 0;
}


/* delay_ms(ms)
 * WAit function with SysTick timer (CMSIS).
 * SysTick timer is set to 1 tick = 1 ms using SysTick_Config().
 * */
void delay_ms(uint32_t millis)
{
	g_delaycnt = millis;

    /* Wait until SysTick ISR decrements counter to 0 */
    while (g_delaycnt != 0u)
       ;
}

/* ISR for SysTick interrupt.
   The name if the ISR is pre-defined.
   In your code just create function with this name and it will be called
   when the interrupt occurs.
*/
void SysTick_Handler(void)
{
    /* DEcrement the counter used in delay_ms function */
    if (g_delaycnt != 0u)
    {
        g_delaycnt--;
    }
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
