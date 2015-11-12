/* -----------------------------------------------------------------------------
 *
 *
 * Project:      SPI Driver Definitions for Freescale MKL25Z4
 * -------------------------------------------------------------------------- */

#ifndef __SPI_MKL25Z4_H
#define __SPI_MKL25Z4_H

#include "MKL25Z4.h"
#include "fsl_spi_hal.h"
#include "Driver_SPI.h"

/* SPI Driver state flags */
#define SPI_FLAG_INIT       (1 << 0)        // Driver initialized
#define SPI_FLAG_POWER      (1 << 1)        // Driver power on
//#define SPI_FLAG_SETUP      (1 << 2)        // SPI configured, clock set


/* SPI Control Information */
typedef struct {
	ARM_SPI_SignalEvent_t cb_event;           // Event callback
	ARM_SPI_STATUS        status;             // Status flags
	uint8_t               flags;              // Control and state flags
	uint32_t			  baud;				// current baudrate of the SPI
	spi_clock_polarity_t polarity;
	spi_clock_phase_t phase;
	spi_shift_direction_t direction;
	uint8_t databits;					// number of data bits. only 8 possible?
} SPI_CTRL;

/* SPI Resource Configuration */
typedef struct {
  SPI_Type        	   *reg;                // SPI register interface
  IRQn_Type             spi_ev_irq;         // SPI Event IRQ Number
  volatile uint32_t    *pclk_cfg_reg;       // Peripheral clock config register
  SPI_CTRL             *ctrl;               // Run-Time control information

} const SPI_RESOURCES;



/* Declare the driver instances */
#if (RTE_SPI0)
extern ARM_DRIVER_SPI Driver_SPI0;
#endif

#if (RTE_SPI1)
extern ARM_DRIVER_SPI Driver_SPI1;
#endif


#endif /* __SPI_MKL25Z4_H */
