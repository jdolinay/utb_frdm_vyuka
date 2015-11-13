/* -----------------------------------------------------------------------------
 *
 *
 * $Date:        31. July 2015
 * $Revision:    V1.00
 *
 * Driver:       Driver_SPI0,
 * Configured:   via RTE_Device.h configuration file
 * Project:      SPI Driver for Freescale MKL25Z4
 * Limitations: This is very limited, poorly tested version!
 * 				Only Master mode is supported, slave mode is not implemented!
 * 				All send/receive is blocking, not interrupt based.
 * 				Only Send with 1 byte value was tested.
 * 				Receive not tested, slave select not tested.
 *
 * Notes: 				KL25Z SPI only supports 8 data bits.
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
 * IMPORTANT: CMSIS functions are non blocking; here everything is blocking for now!

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
  \fn          uint32_t SPI_GetClockFreq (uint32_t clk_src)
  \brief       Return the clock for SPI; in manual it is always bus clock on KL25Z4.
  BUT  in KSDK it is bus clock for SPI0 and system clock (core) for SPI1! see CLOCK_SYS_GetSpiFreq().
  \note  TODO: This could be moved to general file, e.g. clock_mkl25z4.h and use enum for clock src instead of
  SPI_Resources
   We rely on values of bus clock defined in system_MKL24Z4.h file. Hope it will not change.
*/
static uint32_t SPI_GetClockFreq (SPI_RESOURCES* spi)
{
	uint32_t clk = 20971520u;	// preset default clock
#if (CLOCK_SETUP == 0)
	clk = 20971520u;
#elif (CLOCK_SETUP == 1 || CLOCK_SETUP == 4)
	if ( spi->reg == I2C0 )
		clk = 24000000;	// 24 MHz
	else
		clk = 48000000;
#elif (CLOCK_SETUP == 2)
	if ( spi->reg == I2C0 )
		clk = 800000;	// 0.8 MHz
	else
		clk = 4000000;
#elif (CLOCK_SETUP == 3)
	if ( spi->reg == I2C0 )
		clk = 1000000;	// 1 MHz
	else
		clk = 4000000;	// 4 MHz
#else
	#warning CLOCK_SETUP not available for SPI driver. Assuming default bus clock 20.97152MHz.
	// using preset default clock
#endif
	return clk;
}

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
	spi->ctrl->status.busy       = 0;
	spi->ctrl->status.data_lost  = 0;
	spi->ctrl->status.mode_fault = 0;

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

	/* Init the SPI into default state which includes disabling it */
	// Cannot call init because the clock is not enabled;
	//SPI_HAL_Init(spi->reg);

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
	   else if (spi->reg == SPI1) {

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
		SPI_HAL_SetIntMode(spi->reg, kSpiRxFullAndModfInt, false);
		SPI_HAL_SetIntMode(spi->reg, kSpiTxEmptyInt, false);


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

		/* Enable SPI Operation */
		// KSDK version:
		SPI_HAL_Init(spi->reg);


		/* Enable I2C interrupts */
		//NVIC_ClearPendingIRQ(spi->spi_ev_irq);
		//NVIC_EnableIRQ(spi->spi_ev_irq);

		SPI_HAL_Enable(spi->reg);

		spi->ctrl->flags |= SPI_FLAG_POWER;
		break;

	default:
		return ARM_DRIVER_ERROR_UNSUPPORTED;
	}

	return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SPIx_Send (const void *data, uint32_t num, SSP_RESOURCES *ssp)
  \brief       Start sending data to SSP transmitter.
  NOTE: blocking for now!
  \param[in]   data  Pointer to buffer with data to send to SSP transmitter
  \param[in]   num   Number of data items to send
  \param[in]   ssp   Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPIx_Send(const void *data, uint32_t num, SPI_RESOURCES *spi) {

	// speed is set in control()
	// frame format configured in control
	/* In order to flush any remaining data in the shift register, disable then enable the SPI */
	SPI_HAL_Disable(spi->reg);
	SPI_HAL_Enable(spi->reg);

	/* Make sure TX data register (or FIFO) is empty. If not, return appropriate
	 * error status. This also causes a read of the status
	 * register which is required before writing to the data register below.
	 */
	if (SPI_HAL_IsTxBuffEmptyPending(spi->reg) != 1) {
		// TODO: better error code
		return  ARM_DRIVER_ERROR_SPECIFIC;	//kStatus_SPI_TxBufferNotEmpty;
	}

	/* Start the transfer by writing the first byte. If a send buffer was provided, the byte
	 * comes from there. Otherwise we just send a zero byte. Note that before writing to the
	 * data register, the status register must first be read, which was already performed above.
	 */
	uint8_t byteReceived;
	for (uint32_t i = 0; i < num; i++) {
		SPI_HAL_WriteData(spi->reg, ((uint8_t*) data)[i]);
		// wait for transmit complete
		// we must first read received data
		while (!SPI_HAL_IsReadBuffFullPending(spi->reg))
			;
		byteReceived = SPI_HAL_ReadData(spi->reg);

		// then wait for Tx buffer empty flag
		while (!SPI_HAL_IsTxBuffEmptyPending(spi->reg))
			;

	}

	// end transfer, Disable interrupts
	SPI_HAL_SetIntMode(spi->reg, kSpiRxFullAndModfInt, false);
	SPI_HAL_SetIntMode(spi->reg, kSpiTxEmptyInt, false);

}

// Send 0s and read received data
int32_t SPIx_Receive(void *data, uint32_t num, SPI_RESOURCES *spi) {
	/* In order to flush any remaining data in the shift register, disable then enable the SPI */
		SPI_HAL_Disable(spi->reg);
		SPI_HAL_Enable(spi->reg);

		/* Make sure TX data register (or FIFO) is empty. If not, return appropriate
		 * error status. This also causes a read of the status
		 * register which is required before writing to the data register below.
		 */
		if (SPI_HAL_IsTxBuffEmptyPending(spi->reg) != 1) {
			// TODO: better error code
			return  ARM_DRIVER_ERROR_SPECIFIC;	//kStatus_SPI_TxBufferNotEmpty;
		}

		/* Start the transfer by writing the first byte. If a send buffer was provided, the byte
		 * comes from there. Otherwise we just send a zero byte. Note that before writing to the
		 * data register, the status register must first be read, which was already performed above.
		 */
		uint8_t byteReceived;
		for (uint32_t i = 0; i < num; i++) {
			SPI_HAL_WriteData(spi->reg, 0);
			// wait for transmit complete
			// we must first read received data
			while (!SPI_HAL_IsReadBuffFullPending(spi->reg))
				;
			byteReceived = SPI_HAL_ReadData(spi->reg);
			((uint8_t*)data)[i] = byteReceived;

			// then wait for Tx buffer empty flag
			while (!SPI_HAL_IsTxBuffEmptyPending(spi->reg))
				;
		}

		// end transfer, Disable interrupts
		SPI_HAL_SetIntMode(spi->reg, kSpiRxFullAndModfInt, false);
		SPI_HAL_SetIntMode(spi->reg, kSpiTxEmptyInt, false);
}

// send and receive at the same time
int32_t SPIx_Transfer(const void *data_out, void *data_in, uint32_t num, SPI_RESOURCES *spi) {
		/* In order to flush any remaining data in the shift register, disable then enable the SPI */
		SPI_HAL_Disable(spi->reg);
		SPI_HAL_Enable(spi->reg);

		/* Make sure TX data register (or FIFO) is empty. If not, return appropriate
		 * error status. This also causes a read of the status
		 * register which is required before writing to the data register below.
		 */
		if (SPI_HAL_IsTxBuffEmptyPending(spi->reg) != 1) {
			// TODO: better error code
			return  ARM_DRIVER_ERROR_SPECIFIC;	//kStatus_SPI_TxBufferNotEmpty;
		}

		/* Start the transfer by writing the first byte. If a send buffer was provided, the byte
		 * comes from there. Otherwise we just send a zero byte. Note that before writing to the
		 * data register, the status register must first be read, which was already performed above.
		 */
		uint8_t byteReceived;
		for (uint32_t i = 0; i < num; i++) {
			SPI_HAL_WriteData(spi->reg, ((uint8_t*) data_out)[i]);
			// wait for transmit complete
			// we must first read received data
			while (!SPI_HAL_IsReadBuffFullPending(spi->reg))
				;
			byteReceived = SPI_HAL_ReadData(spi->reg);
			((uint8_t*)data_in)[i] = byteReceived;

			// then wait for Tx buffer empty flag
			while (!SPI_HAL_IsTxBuffEmptyPending(spi->reg))
				;

		}

		// end transfer, Disable interrupts
		SPI_HAL_SetIntMode(spi->reg, kSpiRxFullAndModfInt, false);
		SPI_HAL_SetIntMode(spi->reg, kSpiTxEmptyInt, false);
}

uint32_t SPIx_GetDataCount(SPI_RESOURCES *spi) {
	return 0;
}

// will set baudrate to closest lower possible
int32_t SPIx_Control(uint32_t control, uint32_t arg, SPI_RESOURCES *spi) {
	uint32_t calculatedBaudRate;
	uint32_t data_bits;

	if (spi->ctrl->status.busy)
		return ARM_DRIVER_ERROR_BUSY;

	switch (control & ARM_SPI_CONTROL_Msk)
    {
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_SPI_MODE_INACTIVE:             // SPI Inactive
        return ARM_DRIVER_OK;

    case ARM_SPI_MODE_MASTER:               // SPI Master (Output on MOSI, Input on MISO); arg = Bus Speed in bps

    	 /* Set SPI to master mode */
		SPI_HAL_SetMasterSlave(spi->reg, kSpiMaster);

		/* Set slave select to automatic output mode */
		SPI_HAL_SetSlaveSelectOutputMode(spi->reg, kSpiSlaveSelect_AutomaticOutput);

		// Need to also set speed, see arg
		calculatedBaudRate = SPI_HAL_SetBaud(spi->reg, arg,
				SPI_GetClockFreq(spi));
		// Check if the configuration is correct
		if (calculatedBaudRate > arg) {
			return ARM_SPI_ERROR_MODE;
		}
		spi->ctrl->baud = calculatedBaudRate;
        break;

    case ARM_SPI_MODE_SLAVE:                // SPI Slave  (Output on MISO, Input on MOSI)
    	return ARM_DRIVER_ERROR_UNSUPPORTED;
        break;

    case ARM_SPI_MODE_MASTER_SIMPLEX:       // SPI Master (Output/Input on MOSI); arg = Bus Speed in bps
    case ARM_SPI_MODE_SLAVE_SIMPLEX:        // SPI Slave  (Output/Input on MISO)
        return ARM_SPI_ERROR_MODE;

    case ARM_SPI_SET_BUS_SPEED:             // Set Bus Speed in bps; arg = value
		calculatedBaudRate = SPI_HAL_SetBaud(spi->reg, arg, SPI_GetClockFreq(spi));
		// Check if the configuration is correct
		if (calculatedBaudRate > arg) {
			return ARM_SPI_ERROR_MODE;
		}
		spi->ctrl->baud = calculatedBaudRate;
		break;

    case ARM_SPI_GET_BUS_SPEED:             // Get Bus Speed in bps
    	return spi->ctrl->baud;
        break;

    case ARM_SPI_SET_DEFAULT_TX_VALUE:      // Set default Transmit value; arg = value
        break;

    case ARM_SPI_CONTROL_SS:                // Control Slave Select; arg = 0:inactive, 1:active
    	if ( arg == 1 )
    		SPI_HAL_SetSlaveSelectOutputMode(spi->reg, kSpiSlaveSelect_AutomaticOutput);
    	else if (arg == 0 )
    		SPI_HAL_SetSlaveSelectOutputMode(spi->reg, kSpiSlaveSelect_AsGpio);
    	else
    		return ARM_DRIVER_ERROR_UNSUPPORTED;
        break;

    case ARM_SPI_ABORT_TRANSFER:            // Abort current data transfer
    	return ARM_DRIVER_ERROR_UNSUPPORTED;
        break;
    }

	// Preset defaults to frame format
	spi->ctrl->databits = 8;
	spi->ctrl->direction = kSpiMsbFirst;
	spi->ctrl->phase = kSpiClockPhase_FirstEdge;	// or kSpiClockPhase_SecondEdge
	spi->ctrl->polarity = kSpiClockPolarity_ActiveHigh;	// or kSpiClockPolarity_ActiveLow

	// Configure Frame Format
	switch (control & ARM_SPI_FRAME_FORMAT_Msk) {
	case ARM_SPI_CPOL0_CPHA0:		// polarity 0, phase 0
		// TODO: is this correct? based on the value in register bits which equals value in enums from KSDK
		spi->ctrl->polarity = kSpiClockPolarity_ActiveHigh;
		spi->ctrl->phase = kSpiClockPhase_FirstEdge;
		break;

	case ARM_SPI_CPOL0_CPHA1:	// Clock Polarity 0, Clock Phase 1
		spi->ctrl->polarity = kSpiClockPolarity_ActiveHigh;
		spi->ctrl->phase = kSpiClockPhase_SecondEdge;
		break;

	case ARM_SPI_CPOL1_CPHA0:
		spi->ctrl->polarity = kSpiClockPolarity_ActiveLow;
		spi->ctrl->phase = kSpiClockPhase_FirstEdge;
		break;

	case ARM_SPI_CPOL1_CPHA1:
		spi->ctrl->polarity = kSpiClockPolarity_ActiveLow;
		spi->ctrl->phase = kSpiClockPhase_SecondEdge;
		break;

	case ARM_SPI_TI_SSI:
		return ARM_SPI_ERROR_FRAME_FORMAT;
		break;

	case ARM_SPI_MICROWIRE:
		return ARM_SPI_ERROR_FRAME_FORMAT;
		break;

	default:
		return ARM_SPI_ERROR_FRAME_FORMAT;
	}

	// Configure Number of Data Bits
	data_bits = ((control & ARM_SPI_DATA_BITS_Msk) >> ARM_SPI_DATA_BITS_Pos);
	if ( data_bits == 0 )
		data_bits = 8;
	if ( data_bits != 8 )
		return ARM_SPI_ERROR_DATA_BITS;		// only 8 bit supported

	if ((data_bits >= 4) && (data_bits <= 16)) {
		spi->ctrl->databits = data_bits;
	} else {
		return ARM_SPI_ERROR_DATA_BITS;
	}

	// Configure Bit Order
	if ((control & ARM_SPI_BIT_ORDER_Msk) == ARM_SPI_LSB_MSB) {
		// LSB to MSB
		spi->ctrl->direction = kSpiLsbFirst;
	} else {
		spi->ctrl->direction = kSpiMsbFirst;
	}

	SPI_HAL_SetDataFormat(spi->reg, spi->ctrl->polarity, spi->ctrl->phase, spi->ctrl->direction);

	return ARM_DRIVER_OK;
}

ARM_SPI_STATUS SPIx_GetStatus(SPI_RESOURCES *spi) {
	return spi->ctrl->status;
}

void ARM_SPI_SignalEvent(uint32_t event, SPI_RESOURCES *spi) {
    // function body
}

// End SPI Interface




#if (RTE_SPI0)
/* SPI0 Driver wrapper functions */

//TODO zde funkce pro SPI0_...
/* SPI0 Driver wrapper functions */
static int32_t SPI0_Initialize (ARM_SPI_SignalEvent_t cb_event) {
  return (SPIx_Initialize (cb_event, &SPI0_Resources));
}
static int32_t SPI0_Uninitialize (void) {
  return (SPIx_Uninitialize (&SPI0_Resources));
}
static int32_t SPI0_PowerControl (ARM_POWER_STATE state) {
  return (SPIx_PowerControl (state, &SPI0_Resources));
}

static int32_t SPI0_Send(const void *data, uint32_t num) {
	return SPIx_Send(data, num, &SPI0_Resources);
}

static int32_t SPI0_Receive(void *data, uint32_t num) {
	return SPIx_Receive(data, num, &SPI0_Resources);
}

static int32_t SPI0_Transfer(const void *data_out, void *data_in, uint32_t num) {
	SPIx_Transfer(data_out, data_in, num, &SPI0_Resources);
}
static int32_t SPI0_Control(uint32_t control, uint32_t arg) {
	SPIx_Control(control, arg, &SPI0_Resources);
}

static ARM_SPI_STATUS SPI0_GetStatus() {
	return SPIx_GetStatus(&SPI0_Resources);
}

static uint32_t SPI0_GetDataCount() {
	return SPIx_GetDataCount(&SPI0_Resources);
}

/* SPI0 Driver Control Block */
ARM_DRIVER_SPI Driver_SPI0 = {
    SPI_GetVersion,
    SPI_GetCapabilities,
    SPI0_Initialize,
    SPI0_Uninitialize,
    SPI0_PowerControl,
    SPI0_Send,
    SPI0_Receive,
    SPI0_Transfer,
    SPI0_GetDataCount,
    SPI0_Control,
    SPI0_GetStatus
};

#endif
