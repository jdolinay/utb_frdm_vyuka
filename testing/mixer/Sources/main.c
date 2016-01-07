/*
 * Projekt pro testovani modelu Misici jednotky EDU MOD
 * Verze 1 umoznuje ovladani pres seriovou linku, ryhclost 9600 bd
 * prikazy jako "sv1on" a "sv1off" viz cliProcessCommand.
 * Verze 2 je sekvenci napusteni a vypusteni. Aby se aktivovala verze 2
 * je nutno zakomentovat volani verze 1 v main!
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


// Model vyuziva piny na portech B, C, D, E
// Makra pro nastaveni vystupu
#define	SV4_ON()			PTE->PSOR |= (1 << 0)
#define	SV4_OFF()			PTE->PCOR |= (1 << 0)
#define	SV5_ON()			PTE->PSOR |= (1 << 1)
#define	SV5_OFF()			PTE->PCOR |= (1 << 1)
#define	SV3_ON()			PTE->PSOR |= (1 << 4)
#define	SV3_OFF()			PTE->PCOR |= (1 << 4)
#define	SV1_ON()			PTE->PSOR |= (1 << 5)
#define	SV1_OFF()			PTE->PCOR |= (1 << 5)
#define	SV2_ON()			PTC->PSOR |= (1 << 1)
#define	SV2_OFF()			PTC->PCOR |= (1 << 1)
#define	MICHADLO_ON()		PTD->PSOR |= (1 << 0)
#define	MICHADLO_OFF()		PTD->PCOR |= (1 << 0)



// Makra pro testovani vstupu
#define	IS_H1()			(PTD->PDIR & (1 << 3))
#define	IS_H2()			(PTC->PDIR & (1 << 16))
#define	IS_H3()			(PTD->PDIR & (1 << 2))
#define	IS_H4()			(PTC->PDIR & (1 << 5))
#define	IS_H5()			(PTD->PDIR & (1 << 4))
#define	IS_H6()			(PTC->PDIR & (1 << 7))
#define	IS_H7()			(PTD->PDIR & (1 << 5))
#define	IS_H8()			(PTC->PDIR & (1 << 6))

// Prototypy funkci definovanych nize
void delay(void);
void cli_mixer(void);	// ovladani modelu pres UART



int main(void) {
	// 1. Povolime hodinovy signal pro porty
	SIM->SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTC_MASK |
	SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK);

	// 2. Nastavime funkci pinu na GPIO
	PORTE->PCR[0] = PORT_PCR_MUX(1);	// E0 = SV4
	PORTE->PCR[1] = PORT_PCR_MUX(1);	// E1 = SV5
	PORTE->PCR[4] = PORT_PCR_MUX(1);	// E4 = SV3
	PORTE->PCR[5] = PORT_PCR_MUX(1);	// E5 = SV1
	PORTC->PCR[1] = PORT_PCR_MUX(1);	// C1 = SV2
	PORTD->PCR[0] = PORT_PCR_MUX(1);	// D0 = Michadlo

	PORTC->PCR[16] = PORT_PCR_MUX(1);	// C16 = Hladina H2
	PORTD->PCR[5] = PORT_PCR_MUX(1);	// D5 = H7
	PORTD->PCR[4] = PORT_PCR_MUX(1);	// D4 = H5
	PORTD->PCR[2] = PORT_PCR_MUX(1);	// D2 = H3
	PORTD->PCR[3] = PORT_PCR_MUX(1);	// D3 = H1
	PORTC->PCR[7] = PORT_PCR_MUX(1);	// C7 = H6
	PORTC->PCR[6] = PORT_PCR_MUX(1);	// C6 = H8
	PORTC->PCR[5] = PORT_PCR_MUX(1);	// C5 = H4

	// 3. Nastavime prislusne piny jako vystupy a vstupy
	// vystupy
	SV1_OFF();
	SV2_OFF();
	SV3_OFF();
	SV4_OFF();
	SV5_OFF();
	MICHADLO_OFF();
	PTE->PDDR |= (1 << 0);
	PTE->PDDR |= (1 << 1);
	PTE->PDDR |= (1 << 4);
	PTE->PDDR |= (1 << 5);
	PTC->PDDR |= (1 << 1);
	PTD->PDDR |= (1 << 0);

	// vstupy
	PTC->PDDR &= ~(1 << 16);
	PTD->PDDR &= ~(1 << 4);
	PTD->PDDR &= ~(1 << 5);
	PTD->PDDR &= ~(1 << 2);
	PTD->PDDR &= ~(1 << 3);
	PTC->PDDR &= ~(1 << 7);
	PTC->PDDR &= ~(1 << 6);
	PTC->PDDR &= ~(1 << 5);

	/* Zakomentovanim cli_mixer() se pouzije verze 2 */

	/* VERZE 1 s ovladanim pres seriovou linku*/
	cli_mixer();

	/* VERZE 2 s napustenim a vypustenim */
	while(1) {

		// nadrz 1
		SV1_ON();
		while( !IS_H3() )
			;
		SV1_OFF();
		delay();

		SV1_ON();
		while( !IS_H2() )
			;
		SV1_OFF();
		delay();

		SV1_ON();
		while (!IS_H1())
			;
		SV1_OFF();
		delay();

		// Nadrz 2
		SV2_ON();
		while (!IS_H5())
			;
		SV2_OFF();
		delay();

		SV2_ON();
		while (!IS_H4())
			;
		SV2_OFF();
		delay();

		// Nadrz 3
		SV3_ON();
		while (!IS_H8())
			;
		SV3_OFF();
		delay();

		SV3_ON();
		while (!IS_H7())
			;
		SV3_OFF();
		delay();

		SV3_ON();
		while (!IS_H6())
			;
		SV3_OFF();
		delay();

		// Napustit mixer
		SV4_ON();
		while ( IS_H3() )
			;
		delay();
		SV4_OFF();

		// Michat
		MICHADLO_ON();
		delay();
		delay();
		MICHADLO_OFF();
		delay();

		// Vypustit
		SV5_ON();
		delay();
		delay();
		delay();
		SV5_OFF();

	}

	/* Never leave main */
	return 0;
}

/* delay
 * Jednoducha cekaci funkce - busy wait
 * */
void delay(void)
{
	unsigned long n = 2000000L;
	while ( n-- )
		;
}

/*
 * ovladani modelu pres textove prikazy pres UART
 * Zavolat z main a toto se pak stava hlavni smyckou programu.
 * */
void cli_mixer(void) {

	char rcvChar;
	bool commandReady;

	// Inicializace serioveho rozhrani UART
	UART0_Initialize(BD9600);

	while (1) {
		// Cekame na prichod znaku
		rcvChar = UART0_getch();

		// Odesleme znak zpet na seriovou linku (echo)
		UART0_putch(rcvChar);

		/* Build a new command. */
		commandReady = cliBuildCommand(rcvChar);

		/* Call the CLI command processing routine to verify the command entered
		 * and execute the command; then output a new prompt. */
		if (commandReady) {
			commandReady = false;
			cliProcessCommand();
			UART0_putch('>');
		}
	}

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
	if (strcmp(G_CommandBuffer, "sv1on") == 0) {

		SV1_ON();
		UART0_puts("\n\rT1 on\n\r");

	} else if (strcmp(G_CommandBuffer, "sv1off") == 0) {

		SV1_OFF();
		UART0_puts("\n\rT1 off\n\r");

	} else if (strcmp(G_CommandBuffer, "sv2on") == 0) {

		SV2_ON();
		UART0_puts("\n\rT2 on\n\r");

	} else if (strcmp(G_CommandBuffer, "sv2off") == 0) {

		SV2_OFF();
		UART0_puts("\n\rT2 off\n\r");

	} else if (strcmp(G_CommandBuffer, "sv3on") == 0) {

		SV3_ON();
		UART0_puts("\n\rT3 on\n\r");

	} else if (strcmp(G_CommandBuffer, "sv3off") == 0) {

		SV3_OFF();
		UART0_puts("\n\rT3 off\n\r");

	} else if (strcmp(G_CommandBuffer, "sv4on") == 0) {

		SV4_ON();
		UART0_puts("\n\rSV4 on\n\r");

	} else if (strcmp(G_CommandBuffer, "sv4off") == 0) {

		SV4_OFF();
		UART0_puts("\n\rSV4 off\n\r");

	} else if (strcmp(G_CommandBuffer, "sv5on") == 0) {

		SV5_ON();
		UART0_puts("\n\rSV5 on\n\r");

	} else if (strcmp(G_CommandBuffer, "sv5off") == 0) {

		SV5_OFF();
		UART0_puts("\n\rSV5 off\n\r");

	} else if (strcmp(G_CommandBuffer, "mixon") == 0) {

		MICHADLO_ON();
		UART0_puts("\n\rMix on\n\r");

	} else if (strcmp(G_CommandBuffer, "mixoff") == 0) {

		MICHADLO_OFF();
		UART0_puts("\n\rMix off\n\r");

	} else if (strcmp(G_CommandBuffer, "ver") == 0) {
		/* send our version info */
		UART0_puts("\n\rMixer tester version 1.0\n\r");

	} else {
		UART0_puts("\n\rUnknown command\n\r");
	}

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



////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
