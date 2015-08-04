/*
 * $Date:        31. July 2015
 * $Revision:    V1.00
 *
 * Project:      Support for FRDM KL25Z kit
 *
 * Toto je abstrakce prace s piny pro MKL25Z (FRDM-KL25Z board) tak, aby mohly
 * byt pouzity jednotne funkce v driverech pronastavovani potrebnych pinu do
 * rezimu pro dany driver, napr. I2C nebo UART.
 *
 * Exported functions and macros intended for use by other modules:
 * void PINS_PinConfigure(uint32_t pinCode, uint32_t pinFunc);
 *
 *
*/

/* History:
 *  Version 1.00
 *    Initial release
 */

/* Poznamka: kodovani pinu prevzato z MSF_lite */
#ifndef __PINS_MKL25Z4_H
#define __PINS_MKL25Z4_H

#ifdef __cplusplus
extern "C" {
#endif


/* Convenience port letter-to-number defines */
#define	PORT_A		(0)
#define	PORT_B		(1)
#define	PORT_C		(2)
#define	PORT_D		(3)
#define	PORT_E		(4)

/* GPIO_MAKE_PINCODE
 @brief Helper macro to create pin code from port number and pin number
 @note Note that this is not used in runtime; just in compile time to create the
 enum with pin codes.
 Pin code is: 8-bit pin number (normally 0 - 31) in lower byte 1 and 8-bit port number in byte 0
 It cannot be defined in gpio.h as this file is included only at the end of
 msf_<device>.h but needed at the beginning. And gpio.h cannot be at the beginning
 of the msf_<device>.h because it uses macros defined in the msf_<device>.h.
*/
#define	GPIO_MAKE_PINCODE(port, pin)  (((uint16_t)port & 0x00FF) | ((uint16_t)(pin) << 8))


/* MKL25Z has ports A through E
 * We encode the pin as port number (A=0, B=1,...) and pin number (0 - 31)
 * */
typedef enum __mcu_pin_type {
	GPIO_INVALID_PIN = 0x0000,
	GPIO_A0 = GPIO_MAKE_PINCODE(PORT_A, 0),
	GPIO_A1 = GPIO_MAKE_PINCODE(PORT_A, 1),
	GPIO_A2 = GPIO_MAKE_PINCODE(PORT_A, 2),
	/* note that PTA0 and PTA2 have non-standard pull resistor reset value,
	 see Signal multiplexing chapter in the data sheet. */
	GPIO_A3 = GPIO_MAKE_PINCODE(PORT_A, 3),
	GPIO_A4 = GPIO_MAKE_PINCODE(PORT_A, 4),
	GPIO_A5 = GPIO_MAKE_PINCODE(PORT_A, 5),
	/* Pins PTA6 - PTA11 are not available in any package */
	GPIO_A12 = GPIO_MAKE_PINCODE(PORT_A, 12),
	GPIO_A13 = GPIO_MAKE_PINCODE(PORT_A, 13),
	GPIO_A14 = GPIO_MAKE_PINCODE(PORT_A, 14),
	GPIO_A15 = GPIO_MAKE_PINCODE(PORT_A, 15),
	GPIO_A16 = GPIO_MAKE_PINCODE(PORT_A, 16),
	GPIO_A17 = GPIO_MAKE_PINCODE(PORT_A, 17),
	GPIO_A18 = GPIO_MAKE_PINCODE(PORT_A, 18),
	GPIO_A19 = GPIO_MAKE_PINCODE(PORT_A, 19),
	GPIO_A20 = GPIO_MAKE_PINCODE(PORT_A, 20),
	/* no more pins on port A */
	GPIO_B0 = GPIO_MAKE_PINCODE(PORT_B, 0),
	GPIO_B1 = GPIO_MAKE_PINCODE(PORT_B, 1),
	GPIO_B2 = GPIO_MAKE_PINCODE(PORT_B, 2),
	GPIO_B3 = GPIO_MAKE_PINCODE(PORT_B, 3),
	/* pins PTB4 - PTB7 N/A... */
	GPIO_B8 = GPIO_MAKE_PINCODE(PORT_B, 8),
	GPIO_B9 = GPIO_MAKE_PINCODE(PORT_B, 9),
	GPIO_B10 = GPIO_MAKE_PINCODE(PORT_B, 10),
	GPIO_B11 = GPIO_MAKE_PINCODE(PORT_B, 11),
	/* pins PTB12 - PTB15 N/A... */
	GPIO_B16 = GPIO_MAKE_PINCODE(PORT_B, 16),
	GPIO_B17 = GPIO_MAKE_PINCODE(PORT_B, 17),
	GPIO_B18 = GPIO_MAKE_PINCODE(PORT_B, 18),	/* this is red LED on FRDM-KL25Z */
	GPIO_B19 = GPIO_MAKE_PINCODE(PORT_B, 19),	/* this is green LED on FRDM-KL25Z */
	/* no more pins on port B */
	GPIO_C1 = GPIO_MAKE_PINCODE(PORT_C, 1),
	GPIO_C2 = GPIO_MAKE_PINCODE(PORT_C, 2),
	GPIO_C3 = GPIO_MAKE_PINCODE(PORT_C, 3),
	GPIO_C4 = GPIO_MAKE_PINCODE(PORT_C, 4),
	GPIO_C5 = GPIO_MAKE_PINCODE(PORT_C, 5),
	GPIO_C6 = GPIO_MAKE_PINCODE(PORT_C, 6),
	GPIO_C7 = GPIO_MAKE_PINCODE(PORT_C, 7),
	GPIO_C8 = GPIO_MAKE_PINCODE(PORT_C, 8),
	GPIO_C9 = GPIO_MAKE_PINCODE(PORT_C, 9),
	GPIO_C10 = GPIO_MAKE_PINCODE(PORT_C, 10),
	GPIO_C11 = GPIO_MAKE_PINCODE(PORT_C, 11),
	GPIO_C12 = GPIO_MAKE_PINCODE(PORT_C, 12),
	GPIO_C13 = GPIO_MAKE_PINCODE(PORT_C, 13),
	/* pins PTC14 - PTC15 N/A... */
	GPIO_C16 = GPIO_MAKE_PINCODE(PORT_C, 16),
	GPIO_C17 = GPIO_MAKE_PINCODE(PORT_C, 17),
	/* no more pins on port C */
	GPIO_D0 = GPIO_MAKE_PINCODE(PORT_D, 0),
	GPIO_D1 = GPIO_MAKE_PINCODE(PORT_D, 1),	/* this is blue LED on FRDM-KL25Z */
	GPIO_D2 = GPIO_MAKE_PINCODE(PORT_D, 2),
	GPIO_D3 = GPIO_MAKE_PINCODE(PORT_D, 3),
	GPIO_D4 = GPIO_MAKE_PINCODE(PORT_D, 4),
	GPIO_D5 = GPIO_MAKE_PINCODE(PORT_D, 5),
	GPIO_D6 = GPIO_MAKE_PINCODE(PORT_D, 6),
	GPIO_D7 = GPIO_MAKE_PINCODE(PORT_D, 7),
	/* no more pins on port D */
	GPIO_E0 = GPIO_MAKE_PINCODE(PORT_E, 0),
	GPIO_E1 = GPIO_MAKE_PINCODE(PORT_E, 1),
	GPIO_E2 = GPIO_MAKE_PINCODE(PORT_E, 2),
	GPIO_E3 = GPIO_MAKE_PINCODE(PORT_E, 3),
	GPIO_E4 = GPIO_MAKE_PINCODE(PORT_E, 4),
	GPIO_E5 = GPIO_MAKE_PINCODE(PORT_E, 5),
	/* pins PTE6 - PTE19 N/A... */
	GPIO_E20 = GPIO_MAKE_PINCODE(PORT_E, 20),
	GPIO_E21 = GPIO_MAKE_PINCODE(PORT_E, 21),
	GPIO_E22 = GPIO_MAKE_PINCODE(PORT_E, 22),
	GPIO_E23 = GPIO_MAKE_PINCODE(PORT_E, 23),
	GPIO_E24 = GPIO_MAKE_PINCODE(PORT_E, 24),
	GPIO_E25 = GPIO_MAKE_PINCODE(PORT_E, 25),
	/* pins PTE26 - PTE28 N/A... */
	GPIO_E29 = GPIO_MAKE_PINCODE(PORT_E, 29),
	GPIO_E30 = GPIO_MAKE_PINCODE(PORT_E, 30),
	GPIO_E31 = GPIO_MAKE_PINCODE(PORT_E, 31),
	/* no more pins */
	GPIO_PINS_END = 0xFFFF,	/* */

} MCU_pin_type;

typedef MCU_pin_type  MCU_pin_t;

/* -------------- Basic I/O function  ---------------- */
/** Macro to obtain pin number from pin code */
#define		GPIO_PIN_NUM(pin)		((uint8_t)((uint16_t)pin >> 8))

/** Macro to obtain bit mask of a pin from its code */
#define		GPIO_PIN_MASK(pin)		(1 << GPIO_PIN_NUM(pin) )

// Macros to obtain the addresses of various I/O registers from pin code
/** Obtain the address of the GPIO_x structure from pin code. Port number (A=0, etc. is encoded
 * as lowest byte in pin code */
#define		GPIO_GPIO_OBJECT(pin)		((GPIO_Type *)(PTA_BASE + (PTB_BASE-PTA_BASE)*(pin & 0x00FF)))

/** Obtain the address of the PORT_x structure from pin code. Port number (A=0, etc. is encoded
 * as lowest byte in pin code */
#define		GPIO_PORT_OBJECT(pin)		((PORT_Type *)(PORTA_BASE + (PORTB_BASE-PORTA_BASE)*(pin & 0x00FF)))

// Macros which allow us to refer to the I/O registers directly
#define		GPIO_DDR_REG(pin)    	GPIO_GPIO_OBJECT(pin)->PDDR
#define		GPIO_TOGGLE_REG(pin)	GPIO_GPIO_OBJECT(pin)->PTOR
#define		GPIO_SET_REG(pin)		GPIO_GPIO_OBJECT(pin)->PSOR
#define		GPIO_CLEAR_REG(pin)		GPIO_GPIO_OBJECT(pin)->PCOR
#define		GPIO_DATAIN_REG(pin)	GPIO_GPIO_OBJECT(pin)->PDIR

#if 0  /* jd_ not used for now */
/** @brief Directions for GPIO pins
 @note only supported modes should be defined.
 */
typedef enum __gpio_pin_direction {
  input = 0,
  output = 1,
} GPIO_pin_direction;

/** @brief Options for pull-up/pull-down resistors
*/
typedef enum __gpio_pin_pulltype{
  pull_none,
  pull_up,
  pull_down,
} GPIO_pin_pullup;
#endif

/**
  \brief       Configure given pin for given function
  \param[in]   pinCode name (code) of the pin as defined in pins_[device].h file.
  \param[in]   pinFunc number of the alt. function to set the pin to
  \return      none
*/
static inline void PINS_PinConfigure(uint32_t pinCode, uint32_t pinFunc)
{
	GPIO_PORT_OBJECT(pinCode)->PCR[GPIO_PIN_NUM(pinCode)] = PORT_PCR_MUX(pinFunc);
}




#ifdef __cplusplus
}
#endif

#endif /* __PINS_MKL25Z4_H */
