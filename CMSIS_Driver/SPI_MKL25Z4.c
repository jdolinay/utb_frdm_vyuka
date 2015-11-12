/* -----------------------------------------------------------------------------
 *
 *
 * $Date:        31. July 2015
 * $Revision:    V1.00
 *
 * Driver:       Driver_SPI0,
 * Configured:   via RTE_Device.h configuration file
 * Project:      SPI Driver for Freescale MKL25Z4
 * Limitations: Only Master mode is supported, slave mode is not implemented!
 *
 * -----------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting               Value     SPI Interface
 *   ---------------------               -----     -------------
 *   Connect to hardware via Driver_SPI# = 0       use SPI0
 *   Connect to hardware via Driver_SPI# = 1       use SPI1
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.01
 *    - ...
 *  Version 1.00
 *    - First version.
 *    Only Master mode supported.
 */

/* Notes:
 * Driver uses HAL from Freescale Kinetis SDK for handling device registers.

 */

#include <string.h>

#include "SPI_MKL25Z4.h"
#include "fsl_spi_hal.h"	// jd: KSDK HAL functions for handling SPI registers

#include "Driver_SPI.h"
#include "pins_MKL25Z4.h"

#include "RTE_Device.h"

// code based on ARM CMSIS\Driver\Driver_Templates\Driver_SPI.c
#define ARM_SPI_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,00) /* driver version */


/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_SPI_API_VERSION,
    ARM_SPI_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_SPI_CAPABILITIES DriverCapabilities = {
    1, /* Simplex Mode (Master and Slave) */
    0, /* TI Synchronous Serial Interface */
    0, /* Microwire Interface */
    0  /* Signal Mode Fault event: \ref ARM_SPI_EVENT_MODE_FAULT */
};


#if (RTE_SPI0)
/* SPI0 Control Information */
static SPI_CTRL SPI0_Ctrl = { 0 };

/* SPI0 Resources */
static SPI_RESOURCES SPI0_Resources = {
  SPI0,		/* registers of the SPI module instance for this driver */
  SPI0_IRQn,
  &SIM->SCGC4, 	// register where clock for SPI is enabled
  &SPI0_Ctrl
};
#endif /* RTE_SPI0 */


//
//  Functions
//

/**
  \fn          ARM_DRIVER_VERSION SPI_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION SPI_GetVersion(void) {
	return DriverVersion;
}

/**
  \fn          ARM_I2C_CAPABILITIES SPI_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_SPI_CAPABILITIES
*/
static ARM_SPI_CAPABILITIES SPI_GetCapabilities(void) {
	return DriverCapabilities;
}

/**
  \fn          int32_t SPIx_Initialize (ARM_SPI_SignalEvent_t cb_event, SPI_RESOURCES *spi
  \brief       Initialize SPI Interface.
  \param[in]   cb_event  Pointer to \ref ARM_SPI_SignalEvent
  \param[in]   spi   Pointer to SPI resources
  \return      \ref execution_status
  \note			It enables the clock in SIM for the ports of the pin used by the driver.
*/
static int32_t SPIx_Initialize(ARM_SPI_SignalEvent_t cb_event, SPI_RESOURCES *spi) {
	if (spi->ctrl->flags & SPI_FLAG_POWER) {
		/* Driver initialize not allowed */
		return ARM_DRIVER_ERROR;
	}
	if (spi->ctrl->flags & SPI_FLAG_INIT) {
		/* Driver already initialized */
		return ARM_DRIVER_OK;
	}
	spi->ctrl->flags = SPI_FLAG_INIT;

	/* Register driver callback function */
	spi->ctrl->cb_event = cb_event;

	/* Configure SPI Pins */
	if (spi->reg == SPI0) {
		// Enable clock for the pins port
		// We assume all the pins are on the same port
		PINS_EnablePinPortClock(RTE_SPI0_MISO_PIN);

		// Set pin function to I2C
		PINS_PinConfigure(RTE_SPI0_MISO_PIN, RTE_SPI0_MISO_FUNC);
		PINS_PinConfigure(RTE_SPI0_MOSI_PIN, RTE_SPI0_MOSI_FUNC);
		PINS_PinConfigure(RTE_SPI0_SCK_PIN, RTE_SPI0_SCK_FUNC);
		// TODO: set slave select pin? should be optional!

	} else if (spi->reg == SPI1) {
		/* TODO: for SPI1*/
		//PINS_EnablePinPortClock(RTE_SPI1_MISO_PIN);

	}

	/* Clear driver status */
	memset (&spi->ctrl->status, 0, sizeof(ARM_SPI_STATUS));

	return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SPIx_Uninitialize(SPI_RESOURCES *spi)
  \brief       De-initialize SPI Interface.
  \param[in]   i2c   Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPIx_Uninitialize(SPI_RESOURCES *spi) {

	/* Unconfigure SCL and SDA pins */
	  if (spi->reg == SPI0) {
		  PINS_PinConfigure(RTE_SPI0_MISO_PIN, 0);
		  PINS_PinConfigure(RTE_SPI0_MOSI_PIN, 0);
		  PINS_PinConfigure(RTE_SPI0_SCK_PIN, 0);
	   }
	   else if (i2c->reg == I2C1) {

	   }
	return ARM_DRIVER_OK;
}

/**
  \fn          SPIx_PowerControl(ARM_POWER_STATE state, SPI_RESOURCES *spi)
  \brief       Control SPI Interface Power.
  \param[in]   state  Power state
  \param[in]   i2c    Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPIx_PowerControl(ARM_POWER_STATE state, SPI_RESOURCES *spi) {
	if (!(spi->ctrl->flags & SPI_FLAG_INIT)) {
		/* Driver not initialized */
		return ARM_DRIVER_ERROR;
	}

	switch (state) {
	case ARM_POWER_OFF:
		if (!(spi->ctrl->flags & SPI_FLAG_POWER)) {
			/* Driver not powered */
			break;
		}
		if (spi->ctrl->status.busy) {
			/* Transfer operation in progress */
			return ARM_DRIVER_ERROR_BUSY;
		}
		spi->ctrl->flags = SPI_FLAG_INIT;
		/* Disable SPI interrupts */
		NVIC_DisableIRQ(spi->spi_ev_irq);

		//SPI_HAL_SetIntCmd(i2c->reg, false); /* jd: disable interrupts in i2c module*/

		/* Disable I2C Operation */
		/* Use Kinetis SDK HAL */
		SPI_HAL_Disable(spi->reg);

		/* Disable I2C peripheral clock */
		if (spi->reg == SPI0) {
			*spi->pclk_cfg_reg &= ~SIM_SCGC4_SPI0_MASK;
		} else if (spi->reg == SPI1) {
			*spi->pclk_cfg_reg &= ~SIM_SCGC4_SPI1_MASK;
		}
		break;

	case ARM_POWER_LOW:
		return ARM_DRIVER_ERROR_UNSUPPORTED;
		break;

	case ARM_POWER_FULL:
		if (spi->ctrl->flags & SPI_FLAG_POWER) {
			/* Driver already powered */
			break;
		}

		/* Enable I2C peripheral clock */
		if (spi->reg == SPI0) {
			*spi->pclk_cfg_reg |= SIM_SCGC4_SPI0_MASK;
		} else if (spi->reg == SPI1) {
			*spi->pclk_cfg_reg |= SIM_SCGC4_SPI1_MASK;
		}
		// jd: TODO: wait for clock running?


		/* Enable SPI Operation */
		// KSDK version:
		SPI_HAL_Init(i2c->reg);


		/* Enable I2C interrupts */
		NVIC_ClearPendingIRQ(spi->spi_ev_irq);
		NVIC_EnableIRQ(spi->spi_ev_irq);

		SPI_HAL_Enable(spi->reg);
		//I2C_HAL_SetIntCmd(i2c->reg, true);

		spi->ctrl->flags |= SPI_FLAG_POWER;
		break;

	default:
		return ARM_DRIVER_ERROR_UNSUPPORTED;
	}

	return ARM_DRIVER_OK;
}

int32_t SPIx_Send(const void *data, uint32_t num, SPI_RESOURCES *spi) {

}

int32_t SPIx_Receive(void *data, uint32_t num, SPI_RESOURCES *spi) {
}

int32_t SPIx_Transfer(const void *data_out, void *data_in, uint32_t num, SPI_RESOURCES *spi) {

}

uint32_t SPIx_GetDataCount(SPI_RESOURCES *spi) {

}

int32_t SPIx_Control(uint32_t control, uint32_t arg, SPI_RESOURCES *spi) {
    switch (control & ARM_SPI_CONTROL_Msk)
    {
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_SPI_MODE_INACTIVE:             // SPI Inactive
        return ARM_DRIVER_OK;

    case ARM_SPI_MODE_MASTER:               // SPI Master (Output on MOSI, Input on MISO); arg = Bus Speed in bps
        break;

    case ARM_SPI_MODE_SLAVE:                // SPI Slave  (Output on MISO, Input on MOSI)
        break;

    case ARM_SPI_MODE_MASTER_SIMPLEX:       // SPI Master (Output/Input on MOSI); arg = Bus Speed in bps
    case ARM_SPI_MODE_SLAVE_SIMPLEX:        // SPI Slave  (Output/Input on MISO)
        return ARM_SPI_ERROR_MODE;

    case ARM_SPI_SET_BUS_SPEED:             // Set Bus Speed in bps; arg = value
        break;

    case ARM_SPI_GET_BUS_SPEED:             // Get Bus Speed in bps
        break;

    case ARM_SPI_SET_DEFAULT_TX_VALUE:      // Set default Transmit value; arg = value
        break;

    case ARM_SPI_CONTROL_SS:                // Control Slave Select; arg = 0:inactive, 1:active
        break;

    case ARM_SPI_ABORT_TRANSFER:            // Abort current data transfer
        break;
    }
}

ARM_SPI_STATUS SPIx_GetStatus(SPI_RESOURCES *spi) {
}

void ARM_SPI_SignalEvent(uint32_t event, SPI_RESOURCES *spi) {
    // function body
}

// End SPI Interface




#if (RTE_SPI0)
/* SPI0 Driver wrapper functions */

//TODO zde funkce pro SPI0_...
/* I2C0 Driver wrapper functions */
static int32_t SPI0_Initialize (ARM_SPI_SignalEvent_t cb_event) {
  return (SPIx_Initialize (cb_event, &SPI0_Resources));
}
static int32_t SPI0_Uninitialize (void) {
  return (SPIx_Uninitialize (&SPI0_Resources));
}
static int32_t SPI0_PowerControl (ARM_POWER_STATE state) {
  return (SPIx_PowerControl (state, &SPI0_Resources));
}

/* SPI0 Driver Control Block */
ARM_DRIVER_SPI Driver_SPI0 = {
    SPI_GetVersion,
    SPI_GetCapabilities,
    SPI0_Initialize,
    SPI0_Uninitialize,
    SPI0_PowerControl,
    /*ARM_SPI_Send,
    ARM_SPI_Receive,
    ARM_SPI_Transfer,
    ARM_SPI_GetDataCount,
    ARM_SPI_Control,
    ARM_SPI_GetStatus*/
};

#endif
