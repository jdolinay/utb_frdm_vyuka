/*
 * @file     drv_uart.h
 * @brief    Simple UART0 driver for Freescale KL25Z
 * @version  V1.00
 * @date     17. July 2015
 *
 * @note
 *
 * Ukazkovy program pro Programovani mikropocitacu
 * Ovladac pro seriovou komunikaci (UART).
 *
 * Pouzite piny:
 * RX - A1 (alt. funkce 2)
 * TX - A2 (alt. funkce 2)
 *
 * POZOR: ovladac podporuje pouze CLOCK_SETUP 1 nebo 4 tj. frekvenci
 * CPU 48 MHz a frekvenci UART0 clock 8 MHz (externi krystal)
 *
 * Clocks for UARTs:
 * UART0 - the clock can be MCGIRCLK, OSCERCLK, MCGFLLCLK, MCGPLLCLK/2
 * UART1, UART2 - the clock is always bus clock
 * The UART0 clock source is configured in SIM_SOPT2 register;  *
 *
 * Note that the baud rate calculation is different for UART0 than for UART1/2
 * UART0 Baud rate is:
 * baud = uart_clock / ((OSR+1)xBR)
 * BR is baud rate divisor set in baud register UART_BDH (1 - 8191)
 * OSR is oversampling set in UART_C4 register ( 4 - 32)
 * We define the enum value as the BD in lower 13-bits; OSR in bits 13-17
 * and BR for or UART1/2 in bits 18-30, so that we can use the same enum for all UARTs
 * UART1/2 clock is BUS clock (F_BUS)
 * BR = BUSCLK / (16 x baud)
 *
 */
#ifndef	UTBFRDM_DRV_UART_H
#define UTBFRDM_DRV_UART_H

#ifdef __cplusplus
extern "C" {
#endif


/** \ingroup  UTB_FRDM_Drivers
    \defgroup UTB_FRDM_UARTDriver Simple UART Driver for FRDM-KL25Z
  @{
 */

/* macro which creates 32-bit value by combining OSR and BR values.
 * osr is the real OSR value; the register C4 stores osr-1 (value %11 (3) in OSR means real osr = 4) */
#define		UART_MAKE_BDVAL(osr, br, uart1_br )	((br & 0x00001FFF) | ((osr & 0x1F) << 13) \
														| ((uart1_br & 0x00001FFF) << 18))

/* Valid for CPU clock 48 MHz*/

/**
 * @brief Available baudrates for UART0.
 * @note Valid for  CLOCK_SETUP = 1 or 4 in system_MKL25Z4.c (CMSIS);
  the UART0 must be clocked from OSCERCLK, because
  PLLFLLCLK is disabled; the UART0 clock is 8 MHz (external crystal)
  */
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

 /**
  * @brief Initialize the UART0 for given baudrate and 8 data bits + 1 stop bit mode.
  * @param[in] baudrate Must be one of the values defined in UART0_baudrate enum!
  * @return none
  * @note  It is assumed that the CLOCK_SETUP is 1 or 4 (48 MHz CPU clock). This must
  * be defined in project settings > C compiler > preprocesor > symbols, for example:
  * CLOCK_SETUP=1
  */
void uart0_initialize(UART0_baudrate baudrate);

/**
  * @brief Write 1 byte to UART
  * @param[in] data byte of data to write.
  * @return none
  * @note
  */
void uart0_write(uint8_t data);

/**
  * @brief Read one byte from UART
  * @param
  * @return the byte of data read from UART.
  * @note  If there are no data, the function blocks untill data are received.
  * For non-blocking use, first check if there is anything to read by calling
  * uart0_data_available, then call uart0_read.
  */
uint8_t uart0_read(void);

/**
  * @brief Check whether there are some data received in UART buffer.
  * @param
  * @return zero if no data are available, nonzero if data are available.
  * @note
  */
uint8_t uart0_data_available(void);

/*@} end of UTB_FRDM_UARTDriver */


#ifdef __cplusplus
}
#endif

#endif	/* UTBFRDM_DRV_UART_H */

