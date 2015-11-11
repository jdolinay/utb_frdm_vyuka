/* -----------------------------------------------------------------------------
 *
 *
 * Project:      SPI Driver Definitions for Freescale MKL25Z4
 * -------------------------------------------------------------------------- */

#ifndef __SPI_MKL25Z4_H
#define __SPI_MKL25Z4_H

#include "MKL25Z4.h"

#include "Driver_SPI.h"


/* SPI Control Information */
typedef struct {
	ARM_SPI_SignalEvent_t cb_event;           // Event callback
	ARM_SPI_STATUS        status;             // Status flags
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
