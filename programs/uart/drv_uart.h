/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Ovladac pro seriovou komunikaci (UART).
 *
 */
#ifndef	DRV_UART
#define DRV_UART

#ifdef __cplusplus
extern "C" {
#endif

/* Clocks for UARTs:
 * UART0 - the clock can be MCGIRCLK, OSCERCLK, MCGFLLCLK, MCGPLLCLK/2
 * UART1, UART2 - the clock is always bus clock
 *
 * Note that the baud rate is different for UART0 than for UART1/2
 * UART0 Baud rate is:
 * baud = uart_clock / ((OSR+1)xBR)
 * BR is baud rate divisor set in baud register UART_BDH (1 - 8191)
 * OSR is oversampling set in UART_C4 register ( 4 - 32)
 * We define the enum value as the BD in lower 13-bits; OSR in bits 13-17
 * and BR for or UART1/2 in bits 18-30, so that we can use the same enum for all UARTs
 * UART1/2 clock is BUS clock (F_BUS)
 * BR = BUSCLK / (16 x baud)
 * */
/** macro which creates 32-bit value by combining OSR and BR values.
 * osr is the real OSR value; the register C4 stores osr-1 (value %11 (3) in OSR means real osr = 4) */
#define		UART_MAKE_BDVAL(osr, br, uart1_br )	((br & 0x00001FFF) | ((osr & 0x1F) << 13) \
														| ((uart1_br & 0x00001FFF) << 18))

/* Valid for CPU clock 48 MHz*/

/* CLOCK_SETUP = 1 or 4 in system_MKL25Z4.c (CMSIS);
  the UART0 must be clocked from OSCERCLK, because
  PLLFLLCLK is disabled; the UART0 clock is 8 MHz (external crystal) */
typedef enum _uart0_baudrates
 {
        BD_INVALID = 0,
        BD2400 = UART_MAKE_BDVAL(14, 238, 625),
        BD4800 = UART_MAKE_BDVAL(14, 119, 314),
        BD9600 = UART_MAKE_BDVAL(13, 64, 156),
        BD19200 = UART_MAKE_BDVAL(13, 32, 78),
        BD38400 = UART_MAKE_BDVAL(13, 16, 39),
        BD57600 = UART_MAKE_BDVAL(14, 10, 26),
        BD115200 = UART_MAKE_BDVAL(14, 5, 13),
 } UART0_baudrate;

 /** The value of the UART0SRC bitfield in SIM_SOPT2 */
 #define	MSF_UART0_CLKSEL	(2) /* OSCERCLK as UART0 clock source */


void uart0_initialize(UART0_baudrate baudrate);
void uart0_write(uint8_t data);
uint8_t uart0_read(void);
uint8_t uart0_data_available(void);


#ifdef __cplusplus
}
#endif

#endif

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
