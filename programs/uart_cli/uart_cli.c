/*
 * Sample program for MCU programming course
 * Serial communication (UART). Controlling program via Command Line Interface (CLI).
 * The program shows how to process commands sent over serial line.
 * Baud rate 9600 bd.
 * Supported commands:
 * ledon - turn on LED
 * ledoff - turn off LED
 * ver - print program version
 * On PC type the command and press ENTER.
 *
 * NOTE: In project properties > compiler > preprocesor must be defined: CLOCK_SETUP=1
 * so that the CPU runs at 48 MHz!
 *
 * Information
   KL25Z has 3 UART modules: UART0, UART1 a UART2. Module UART0 is connected also
   	   to the pins for USB communication; so it is available via virtual serial port
   	   over the USB cable connected to the SDA connector on FRDM-KL25Z board.
   	   On the desktop PC you will see it as "Open SDA - CDC Serial Port".
 *
 */

#include "MKL25Z4.h"
#include "drv_uart.h"
#include <stdbool.h>
#include <string.h>     // pro funkci strcmp()


// max length of a command
#define   MAX_COMMAND_LEN       (7)

// prototypes
void cliProcessCommand(void);
bool cliBuildCommand(char nextChar);

// buffer for storing the command
char G_CommandBuffer[MAX_COMMAND_LEN + 1];



int main(void)
{
	char rcvChar;
    bool commandReady;

	// Initialize UART (using uart driver)
	UART0_Initialize(BD9600);

	while (1) {
		// Wait for character from serial line
		rcvChar = UART0_getch();

		// Send the character back (echo)
		UART0_putch(rcvChar);

		// Build a new command.
		commandReady = cliBuildCommand(rcvChar);

		/* Call the CLI command processing routine to verify the command entered
		 * and execute the command; then output a new prompt. */
		if (commandReady) {
			commandReady = false;
			cliProcessCommand();
			UART0_putch('>');
		}
	}



    /* Never leave main */
    return 0;
}

/**********************************************************************
 *
 * Function:    cliBuildCommand
 *
 * Description: Put received characters into the command buffer. Once
 *              a complete command is received return 1.
 *
 * Notes:
 *
 * Returns:     1 if a command is complete, otherwise 0.
 *
 **********************************************************************/
bool cliBuildCommand(char nextChar)
{
    static char idx = 0;

    /* Don't store any new line characters or spaces. */
    if ((nextChar == '\n') || (nextChar == ' ') || (nextChar == '\t'))
        return 0;

    /* The completed command has been received. Replace the final carriage
     * return character with a NULL character to help with processing the
     * command. */
    if (nextChar == '\r')
    {
        G_CommandBuffer[idx] = '\0';
        idx = 0;
        return true;
    }

    /* Store the received character in the command buffer. */
    G_CommandBuffer[idx] = nextChar;
    idx++;

    /* If the command is too long, reset the index and process
     * the current command buffer. */
    if (idx > MAX_COMMAND_LEN)
    {
        idx = 0;
        return 1;
    }

    return false;
}

/**********************************************************************
 *
 * Function:    cliProcessCommand
 *
 * Description: Look up the command in the command table. If the
 *              command is found, call the command's function. If the
 *              command is not found, output an error message.
 *
 * Notes:
 *
 * Returns:     None.
 *
 **********************************************************************/
void cliProcessCommand(void)
{
    if ( strcmp(G_CommandBuffer, "ledon") == 0 )
    {
        UART0_puts("\n\rLED1 is on\n\r");
    }
    else if (strcmp(G_CommandBuffer, "ledoff") == 0 )
    {
        UART0_puts("\n\rLED1 is off\n\r");
    }
    else if (strcmp(G_CommandBuffer, "ver") == 0 )
    {
        /* send our version info */
    	UART0_puts("\n\rCLI demo version 1.0\n\r");
    }
    else
    {
    	UART0_puts("\n\rUnknown command\n\r");
    }

}








////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
