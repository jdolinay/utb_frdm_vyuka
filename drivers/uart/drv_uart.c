/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Ovladac pro seriovou komunikaci (UART).
 *
 * Detailni dokumentace je v .h souboru.
 *
 */

#include "MKL25Z4.h"
#include "drv_uart.h"


#define		UART_GET_BR(baud_val)		(baud_val & 0x00001FFF)				/* the BR is actually only 13-bit long*/
#define		UART_GET_OSR(baud_val)		((baud_val >> 13) & 0x0000001F)		/* OSR is only 5-bits */
#define		UART_GET_BR_UART1(baud_val)	((baud_val & 0x7FFC0000) >> 18)			/* the BR for UART1/2, 13-bit long*/

/* Internal functions */
static void f_UART0_setbaudrate(UART0_baudrate baudrate);

const char UTB_UART_CR = (const char)0x0D;
const char UTB_UART_LF = (const char)0x0A;

/*
 * Initialize UART0
 */
void UART0_initialize(UART0_baudrate baudrate)
{
	/* Enable clock for PORTA needed for Tx, Rx pins */
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
	/* Enable the UART_RXD function on PTA1 */
	PORTA->PCR[1] = PORT_PCR_MUX(2);
	/* Enable the UART_TXD function on PTA2 */
	PORTA->PCR[2] = PORT_PCR_MUX(2);

	/* set clock for UART0 */
	SIM->SOPT2 |= SIM_SOPT2_UART0SRC(MSF_UART0_CLKSEL);

	SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;		/* Enable clock for UART */

	/* Disable UART0 before changing registers */
	/* uart->reg->C2 &= ~(UART0_C2_TE_MASK | UART0_C2_RE_MASK); */
	UART0->C2 = 0;	/* default values */
	UART0->C1 = 0;	/* default values */
	UART0->C3 = 0;	/* default values */
	UART0->BDH = 0;	/* default value including 1 stop bit */

	/* changes C4 and C5 to default values + sets baudrate preserving the other bits in BDH*/	f_UART0_setbaudrate(baudrate);

	/* Clear Receiver overrun flag, just in case */
	UART0->S1  |= UART0_S1_OR_MASK;

	/* Enable receiver and transmitter */
	UART0->C2 |= (UART0_C2_TE_MASK | UART0_C2_RE_MASK );

}

/* Write single byte to SCI
 * param data byte of data to write
 */
 void UART0_write(uint8_t data)
 {
	 /* Wait until space is available in the FIFO */
	 while(!(UART0->S1 & UART0_S1_TDRE_MASK))
		 ;

	 /* Send the character */
	 UART0->D = data;
 }

 /* Read byte form SCI; wait for data to arrive!
  * return byte received from SCI.
  */
 uint8_t UART0_read(void)
 {
	 /* Wait until character has been received */
	 while (!(UART0->S1 & UART0_S1_RDRF_MASK)) {

		 /* It may happen that new char arrives before we
		  * read the old one and overrun flag is set.
		  * Then clear the flag and return old char; the new char is lost.
		  */
		 if ( UART0->S1 & UART0_S1_OR_MASK ) {
			 UART0->S1  |= UART0_S1_OR_MASK;
			 break;
		 }

	 }

	 /* Return the 8-bit data from the receiver */
	 return UART0->D;
 }

 /* Check if data are available for reading
  * return 1 if there are data, 0 if not
 */
 uint8_t UART0_data_available(void)
 {
	 return ((UART0->S1 & UART0_S1_RDRF_MASK) != 0);
 }

 /* read one character. Blocks the caller until a character is available! */
 char UART0_getch(void)
 {
 	return (char)UART0_read();
 }

 /* send one character to console */
 void UART0_putch(char c)
 {
     if(c == '\n')
     {
     	UART0_write(UTB_UART_CR);
     	UART0_write(UTB_UART_LF);
     }
     else
     {
    	 UART0_write(c);
     }
 }

 /* send null-terminated string */
 void UART0_puts(const char* str)
 {
 	while(*str)
    {
     	if( *str == '\n')
     	{
     		UART0_write(UTB_UART_CR);
     		UART0_write(UTB_UART_LF);
     	}
     	else
     	{
     		UART0_write(*str);
     	}

     	str++;
     }
 }

 /* read string from console. */
 uint32_t UART0_gets(char* str, uint32_t max_chars, char terminator)
 {
     char c;
     uint8_t i;

     for ( i = 0; i < max_chars; i++ )
     {
     	c = UART0_getch();
     	if ( c != terminator )
             str[i] = c;
     	else
     		break;
     }
     str[i] = 0;
     return i;
 }



 /*internal function */
static void f_UART0_setbaudrate(UART0_baudrate baudrate)
{
	uint32_t osr_val;
	uint32_t sbr_val;
	uint32_t reg_temp = 0;

	osr_val = UART_GET_OSR((uint32_t)baudrate);
	sbr_val = UART_GET_BR((uint32_t)baudrate);

	// If the OSR is between 4x and 8x then both
	// edge sampling MUST be turned on.
	if ((osr_val > 3) && (osr_val < 9))
		UART0->C5 |= UART0_C5_BOTHEDGE_MASK;
	else
		UART0->C5 &= ~UART0_C5_BOTHEDGE_MASK;

	// Setup OSR value
	reg_temp = UART0->C4;
	reg_temp &= ~UART0_C4_OSR_MASK;
	reg_temp |= UART0_C4_OSR(osr_val - 1);
	// Write reg_temp to C4 register
	UART0->C4 = reg_temp;

	/* Save current value of uartx_BDH except for the SBR field */
	reg_temp = UART0->BDH & ~(UART0_BDH_SBR(0x1F));
	/* write new value */
	UART0->BDH = reg_temp | UART0_BDH_SBR(((sbr_val & 0x1F00) >> 8));
	UART0->BDL = (uint8_t) (sbr_val & UART0_BDL_SBR_MASK);
}



////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
