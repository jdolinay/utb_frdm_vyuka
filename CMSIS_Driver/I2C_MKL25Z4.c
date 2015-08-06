/* -----------------------------------------------------------------------------
 * Copyright (c) 2013-2014 ARM Ltd.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software. Permission is granted to anyone to use this
 * software for any purpose, including commercial applications, and to alter
 * it and redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software in
 *    a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *
 * $Date:        31. July 2015
 * $Revision:    V1.00
 *
 * Driver:       Driver_I2C0, Driver_I2C1
 * Configured:   via RTE_Device.h configuration file
 * Project:      I2C Driver for Freescale MKL25Z4
 * Note: Uses HAL from Freescale Kinetis SDK for handling device registers.
 * -----------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting               Value     I2C Interface
 *   ---------------------               -----     -------------
 *   Connect to hardware via Driver_I2C# = 0       use I2C0
 *   Connect to hardware via Driver_I2C# = 1       use I2C1
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 1.01
 *    - ...
 *  Version 1.00
 *    - First version based on Keil driver for NXP LPC18xx V2.01
 */

/* Poznamky:
 4.8.15 - asi nutno prepracovat master handler protoze hw je jine nez LPC a resit podle jejich
 stavu je komplikovane, spis se drzet KSDK :)
 slave handler zatim neimplementuju.
 TODO: jak resit vyslani prikazu a prijem dat asynchronne, nebyly by lepsi blokujici funkce?
 5.8.2015 pokus s funkcemi z KSDK jako I2C_DRV_MasterSendDataBlocking,
 POZOR: Nejde tak snadno pouzit :(
 On totiz blocking rezim KSDK stejne vyuziva preruseni pro prijem a jen ceka na semafor, ktery
 se pres nej nejak nastavuje asi jak prijme pocet znaku...

 Pokracuj: i kdyz jsem upravil na blocking rezim, nefunguje, zustane vyset pri
 cekani na transf. complete flag pri receive, protoze ten funguje az po ACK/NACK ale to pri
 prijmu musim generovat ja.. > nevim na co cekat pri prijmu dat... asi to bez interruptu
 nejde? ale ani int flag neni generovat pokud neni ack...
 Mozna jednodussi rozchodit existujici reseni, overit, ze spravnce vysila
 repeasted start atd.
 * */

#include <string.h>

#include "I2C_MKL25Z4.h"   //#include "I2C_LPC18xx.h"
//#include "SCU_LPC18xx.h"
#include "fsl_i2c_hal.h"	// jd: KSDK HAL functions for handling I2C registers

#include "Driver_I2C.h"
#include "pins_MKL25Z4.h"

#include "RTE_Device.h"
//#include "RTE_Components.h"  // jd: asi konfigurace ktere drivery jsou enabled v projektu, tj. #define I2C0  1  apod.

#define ARM_I2C_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,00) /* driver version */

// jd: kontrola pokud je i2c driver instance zapnuta v RTE_Components.h ale pritom
// neni nastaveno RTE_I2C0 na 1 v RTE_Device.h. tj. neni nakonfigurovan...
#if ((defined(RTE_Drivers_I2C0) || \
      defined(RTE_Drivers_I2C1))   \
     && !RTE_I2C0                  \
     && !RTE_I2C1)
#error "I2C not configured in RTE_Device.h!"
#endif

/* I2C core clock (system_LPC18xx.c) */
//extern uint32_t GetClockFreq (uint32_t clk_src);


/**
  \fn          uint32_t GetClockFreq (uint32_t clk_src)
  \brief       Return the clock for I2C; in manual it is always bus clock on KL25Z4.
  BUT  in KSDK it is bus clock for I2C0 and system clock (core) for I2C1! see CLOCK_SYS_GetI2cFreq.
  \note  TODO: This could be moved to general file, e.g. clock_mkl25z4.h
   We rely on values of bus clock defined in system_MKL24Z4.h file. Hope it will not change.
*/
static uint32_t GetClockFreq (uint32_t clk_src)
{
	uint32_t clk = 20971520u;	// preset default clock
#if (CLOCK_SETUP == 0)
	clk = 20971520u;
#elif (CLOCK_SETUP == 1)
	// TODO: foa any other clock than default (0) the I2C1 and I2C0 have different clock? see above.
	clk = 24000000;	// 24 MHz
#elif (CLOCK_SETUP == 2)
	clk = 800000;	// 0.8 MHz
#elif (CLOCK_SETUP == 3)
	clk = 1000000;	// 1 MHz
#elif (CLOCK_SETUP == 4)
	clk = 24000000;
#else
	#warning CLOCK_SETUP not available for I2C driver. Assuming default bus clock 20.97152MHz.
	// using preset default clock
#endif
	return clk;
}

/**
  \fn          void CompleteTransfer (void)
  \brief       Send STOP and disable interrupt when error happens or I2C transfer finishes.
*/
static void CompleteTransfer(I2C_RESOURCES *i2c)
{
	I2C_Type * base = i2c->reg;

	/* Disable interrupt. */
	//jd: not needed, handled by the parent code via NVIC:
	// I2C_HAL_SetIntCmd(base, false);

	/* Generate stop signal. */
	I2C_HAL_SendStop(base);
}

#if 0
/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_DRV_MasterSetBaudRate
 * Description   : configures the I2C bus to access a device.
 * This function will set baud rate.
 *
 *END**************************************************************************/
void I2C_DRV_MasterSetBaudRate(I2C_RESOURCES *i2c, uint32_t instance, const i2c_device_t * device)
{
    //assert(device);
    //assert(instance < I2C_INSTANCE_COUNT);

    I2C_Type * base = i2c->reg;	//g_i2cBase[instance];
    uint32_t i2cClockFreq;

    /* Get current runtime structure. */
    //i2c_master_state_t * master = (i2c_master_state_t *)g_i2cStatePtr[instance];

    /* Set baud rate if different.*/
    //if (device->baudRate_kbps != master->lastBaudRate_kbps)
    //{
        /* Get the current bus clock.*/
        i2cClockFreq = CLOCK_SYS_GetI2cFreq(instance);
        I2C_HAL_SetBaudRate(base, i2cClockFreq, device->baudRate_kbps, NULL);

        /* Record baud rate change */
       // master->lastBaudRate_kbps = device->baudRate_kbps;
    //}
}
#endif

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_DRV_MasterWait
 * Description   : Wait transfer to finish.
 * This function is a static function which will be called by other data
 * transaction APIs.
 *
 *END**************************************************************************/
static i2c_status_t I2C_DRV_MasterWait(I2C_RESOURCES *i2c, /*uint32_t instance,*/ uint32_t timeout_ms)
{
	// jd: nemam zadne funkce os...
	// zkusim cekat na TCF flag (transfer complete)
	while ( (i2c->reg->S & I2C_S_TCF_MASK) == 0 )
		;	// transfer in progress

#if 0
    assert(instance < I2C_INSTANCE_COUNT);

    i2c_master_state_t * master = (i2c_master_state_t *)g_i2cStatePtr[instance];
    //osa_status_t syncStatus;

    do
    {
        syncStatus = OSA_SemaWait(&master->irqSync, timeout_ms);
    }while(syncStatus == kStatus_OSA_Idle);

    if (syncStatus != kStatus_OSA_Success)
    {
        master->status = kStatus_I2C_Timeout;
    }

    return master->status;
#endif
}

/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_DRV_SendAddress
 * Description   : Prepare and send out address buffer with interrupt.
 * This function is a static function which will be called by other data
 * transaction APIs.
 *
 *END**************************************************************************/
static i2c_status_t I2C_DRV_SendAddress(I2C_RESOURCES *i2c,
										uint32_t address,

										/*uint32_t instance,
                                        const i2c_device_t * device,*/
                                        const uint8_t * cmdBuff,
                                        uint32_t cmdSize,
                                        i2c_direction_t direction,
                                        uint32_t timeout_ms)
{
    //assert(instance < I2C_INSTANCE_COUNT);

    I2C_Type * base = i2c->reg;	//g_i2cBase[instance];
    /* Get current runtime structure. */
    //i2c_master_state_t * master = (i2c_master_state_t *)g_i2cStatePtr[instance];

    uint8_t addrByte1, addrByte2, directionBit;
    bool is10bitAddr = false;
    uint8_t addrBuff[2] = {0};
    uint8_t addrSize = 0;
    bool isMainXferBlocking = true;	//master->isBlocking;

    /* Send of address and CMD must be blocking without STOP */
    //master->isRequesting = true;
    //master->isBlocking = true;

    /*--------------- Prepare Address Buffer ------------------*/
    /* Get r/w bit according to required direction.
     * read is 1, write is 0. */
    directionBit = (direction == kI2CReceive) ? 0x1U : 0x0U;

    /* Check to see if slave address is 10 bits or not. */
    //is10bitAddr = ((device->address >> 10U) == 0x1EU) ? true : false;

    /* Get address byte 1 and byte 2 according address bit number. */
    /*
    if (is10bitAddr)
    {
        addrByte1 = (uint8_t)(device->address >> 8U);
        addrByte2 = (uint8_t)device->address;
    }
    else
    {
        addrByte1 = (uint8_t)device->address;
    }*/

    addrByte1 = (uint8_t)address;

    /* Get the device address with r/w direction. If we have a sub-address,
      then that is always done as a write transfer prior to transferring
      the actual data.*/
    addrByte1 = addrByte1 << 1U;

    /* First need to write if 10-bit address or has cmd buffer. */
    addrByte1 |= (uint8_t)((is10bitAddr || cmdBuff) ? 0U : directionBit);

    /* Put slave address byte 1 into address buffer. */
    addrBuff[addrSize++] = addrByte1;

    if (is10bitAddr)
    {
        /* Put address byte 2 into address buffer. */
        addrBuff[addrSize++] = addrByte2;
    }

    /*--------------- Send Address Buffer ------------------*/
    //master->txBuff = addrBuff;
    //master->txSize = addrSize;

    /* Send first byte in address buffer to trigger interrupt.*/
    I2C_HAL_WriteByte(base, addrBuff[0]);

    /* Wait for the transfer to finish.*/
    I2C_DRV_MasterWait(i2c, timeout_ms);

    /*--------------------- Send CMD -----------------------*/
    //if ((master->status == kStatus_I2C_Success) && cmdBuff)
    if (cmdBuff)
    {
    	// JD: pozor: not supported cmdSize > 1 !!!

       // master->txBuff = cmdBuff;
       // master->txSize = cmdSize;

        /* Send first byte in address buffer to trigger interrupt.*/
        I2C_HAL_WriteByte(base, *cmdBuff);

        /* Wait for the transfer to finish.*/
        I2C_DRV_MasterWait(i2c, timeout_ms);
    }

    /*--------------- Send Address Again ------------------*/
    /* Send slave address again if receiving data from 10-bit address slave,
       OR conducting a cmd receive */
    if (/*(master->status == kStatus_I2C_Success) && */
    	(direction == kI2CReceive)
          && (is10bitAddr || cmdBuff))
    {
        /* Need to send slave address again. */
       // master->txSize = 1U;
       // master->txBuff = NULL;

        /* Need to generate a repeat start before changing to receive. */
        I2C_HAL_SendStart(base);

        /* Send address byte 1 again. */
        I2C_HAL_WriteByte(base, (uint8_t)(addrByte1 | 1U));

        /* Wait for the transfer to finish.*/
        // jd: tady zustane cekat:
         // I2C_DRV_MasterWait(i2c, timeout_ms);
    }

   // master->isRequesting = false;
   // master->isBlocking = isMainXferBlocking ;

    return kStatus_I2C_Success;	//master->status;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_DRV_MasterSend
 * Description   : Private function to handle blocking/non-blocking send.
 * This function is a static function which will be called by other data
 * transaction APIs.
 *
 *END**************************************************************************/
i2c_status_t I2C_DRV_MasterSend(I2C_RESOURCES *i2c,
										uint32_t slaveAddress,
									   /*uint32_t instance,
                                       const i2c_device_t * device,*/
                                       const uint8_t * cmdBuff,
                                       uint32_t cmdSize,
                                       const uint8_t * txBuff,
                                       uint32_t txSize,
                                       uint32_t timeout_ms,
                                       bool isBlocking)
{
    //assert(instance < I2C_INSTANCE_COUNT);
    //assert(txBuff);

	uint32_t sentCount;
    I2C_Type * base = i2c->reg; //g_i2cBase[instance];
   //i2c_master_state_t * master = (i2c_master_state_t *)g_i2cStatePtr[instance];

    /* Return if current instance is used */
    // TODO:
    /*
    if (!master->i2cIdle)
    {
        return master->status = kStatus_I2C_Busy;
    }*/

    /* Need to assign a pre-defined timeout value for sending address and cmd */
    /*if (!isBlocking)
    {
        timeout_ms = I2C_TIMEOUT_MS;
    }

    master->txBuff = NULL;
    master->txSize = 0;
    master->rxBuff = NULL;
    master->rxBuff = 0;
    master->status = kStatus_I2C_Success;
    master->i2cIdle = false;
    master->isBlocking = isBlocking;
    */

    // neni potreba
    //I2C_DRV_MasterSetBaudRate(instance, device);

    /* Set direction to send for sending of address and data. */
    I2C_HAL_SetDirMode(base, kI2CSend);

    /* Enable i2c interrupt.*/
    I2C_HAL_ClearInt(base);
    I2C_HAL_SetIntCmd(base, true);

    /* Generate start signal. */
    I2C_HAL_SendStart(base);

    /* Send out slave address. */
    I2C_DRV_SendAddress(i2c, slaveAddress, /*instance, device,*/ cmdBuff, cmdSize, kI2CSend, timeout_ms);

    /* Send out data in transmit buffer. */
    //if (master->status == kStatus_I2C_Success)
    {
        /* Fill tx buffer and size to run-time structure. */
       // master->txBuff = txBuff;
       // master->txSize = txSize;

        /* Send first byte in transmit buffer to trigger interrupt.*/
    	// jd: no interrupts
        //I2C_HAL_WriteByte(base, txBuff[0]);

        //if (isBlocking)
        {
        	sentCount = 0;
        	while ( sentCount < txSize )
        	{
        		I2C_HAL_WriteByte(base, txBuff[sentCount]);
        		I2C_DRV_MasterWait(i2c, timeout_ms);
        		sentCount++;
        	}
        }
    }
#if 0
    else if (master->status == kStatus_I2C_Timeout)
    {
        /* Disable interrupt. */
        I2C_HAL_SetIntCmd(base, false);

        if (I2C_HAL_GetStatusFlag(base, kI2CBusBusy))
        {
            /* Generate stop signal. */
            I2C_HAL_SendStop(base);
        }

        /* Indicate I2C bus is idle. */
        //master->i2cIdle = true;
    }
#endif

    return kStatus_I2C_Success;	//master->status;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : I2C_DRV_MasterReceive
 * Description   : Private function to handle blocking/non-blocking receive.
 * This function is a static function which will be called by other data
 * transaction APIs.
 *
 *END**************************************************************************/
i2c_status_t I2C_DRV_MasterReceive(I2C_RESOURCES *i2c,
											uint32_t slaveAddress,
											/*uint32_t instance,
                                          const i2c_device_t * device,*/
                                          const uint8_t * cmdBuff,
                                          uint32_t cmdSize,
                                          uint8_t * rxBuff,
                                          uint32_t rxSize,
                                          uint32_t timeout_ms,
                                          bool isBlocking)
{
   // assert(instance < I2C_INSTANCE_COUNT);
   // assert(rxBuff);
	uint8_t remainingBytes = rxSize;

    I2C_Type * base = i2c->reg;	//g_i2cBase[instance];
    //i2c_master_state_t * master = (i2c_master_state_t *)g_i2cStatePtr[instance];

    /* Return if current instance is used */
   /* if (!master->i2cIdle)
    {
        return master->status = kStatus_I2C_Busy;
    }*/

    /* Need to assign a pre-defined timeout value for sending address and cmd */
   /* if (!isBlocking)
    {
        timeout_ms = I2C_TIMEOUT_MS;
    }

    master->rxBuff = rxBuff;
    master->rxSize = rxSize;
    master->txBuff = NULL;
    master->txSize = 0;
    master->status = kStatus_I2C_Success;
    master->i2cIdle = false;
    master->isBlocking = isBlocking;

    I2C_DRV_MasterSetBaudRate(instance, device);
    */

    /* Set direction to send for sending of address. */
    I2C_HAL_SetDirMode(base, kI2CSend);

    /* Enable i2c interrupt.*/
    I2C_HAL_ClearInt(base);
    // jd: viz nize, nechci pouzit interrupt:
    // I2C_HAL_SetIntCmd(base, true);

    /* Generate start signal. */
    I2C_HAL_SendStart(base);

    /* Send out slave address. */
    I2C_DRV_SendAddress(i2c, slaveAddress, /*instance, device,*/ cmdBuff, cmdSize, kI2CReceive, timeout_ms);

    /* Start to receive data. */
    //if (master->status == kStatus_I2C_Success)
    {
        /* Change direction to receive. */
        I2C_HAL_SetDirMode(base, kI2CReceive);

        /* Send NAK if only one byte to read. */
        if (rxSize == 0x1U)
        {
            I2C_HAL_SendNak(base);
        }
        else
        {
            I2C_HAL_SendAck(base);
        }

        /* Dummy read to trigger receive of next byte in interrupt. */
        //I2C_HAL_ReadByte(base);

        if (isBlocking)
        {
            /* Wait for the transfer to finish.*/
            //I2C_DRV_MasterWait(i2c, timeout_ms);
        	 // JD: toto nejde pouzit, nechci vyuzit interrupt, nemam ho..
        	I2C_HAL_ReadByte(base);	// toto zahaji cteni
        	while ( remainingBytes > 0)
        	{
        		// TODO: asi zde nemuzu cekat na tranf. complete protoze
        		// ten bude az vyslu ACK?
        		  //I2C_DRV_MasterWait(i2c, timeout_ms);	// cekam na 1 bajt
        		 if (remainingBytes == 1)
        		 {
        			 I2C_HAL_SendNak(base);
        		 }
        		 else
        		 {
        			 I2C_HAL_SendAck(base);
        		 }

        		// toto zahaji cteni dalsiho byte
        		*rxBuff++ =	I2C_HAL_ReadByte(base); // i2c->reg->D;
        		remainingBytes--;
        	}
        }
    }
#if 0
    else if (master->status == kStatus_I2C_Timeout)
    {
        /* Disable interrupt. */
        I2C_HAL_SetIntCmd(base, false);

        if (I2C_HAL_GetStatusFlag(base, kI2CBusBusy))
        {
            /* Generate stop signal. */
            I2C_HAL_SendStop(base);
        }

        /* Indicate I2C bus is idle. */
        master->i2cIdle = true;
    }
#endif

    return  kStatus_I2C_Success;	//master->status;
}






/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
  ARM_I2C_API_VERSION,
  ARM_I2C_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_I2C_CAPABILITIES DriverCapabilities = {
  0            /* supports 10-bit addressing */
};


#if (RTE_I2C0)
/* I2C0 Control Information */
static I2C_CTRL I2C0_Ctrl = { 0 };

/* I2C0 Resources */
static I2C_RESOURCES I2C0_Resources = {
  I2C0,		/* registers of the I2C module instance for this driver */
  I2C0_IRQn,
  //&LPC_CGU->BASE_APB1_CLK,
  &SIM->SCGC4, // jd: register where clock for i2c is enabled //&LPC_CCU1->CLK_APB1_I2C0_CFG,
  //&LPC_CCU1->CLK_APB1_I2C0_STAT,
  RGU_RESET_I2C0,
  &I2C0_Ctrl
};
#endif /* RTE_I2C0 */


#if (RTE_I2C1)
/* I2C1 Control Information */
static I2C_CTRL I2C1_Ctrl = { 0 };

/* I2C1 Resources */
static I2C_RESOURCES I2C1_Resources = {
  I2C1,		/* registers of the I2C module instance for this driver */
  I2C1_IRQn,
  //&LPC_CGU->BASE_APB3_CLK,
  &SIM->SCGC4, //&LPC_CCU1->CLK_APB3_I2C1_CFG,
  // &LPC_CCU1->CLK_APB3_I2C1_STAT,
  RGU_RESET_I2C1,
  &I2C1_Ctrl
};
#endif /* RTE_I2C1 */

// todo: temp
bool I2C1_DRV_MasterSend(uint32_t slaveAddress,
                                       const uint8_t * cmdBuff,
                                       uint32_t cmdSize,
                                       const uint8_t * txBuff,
                                       uint32_t txSize)
{
	I2C_DRV_MasterSend(&I2C1_Resources, slaveAddress, cmdBuff, cmdSize,
			txBuff, txSize, 0, true);

	return true;
}



bool I2C1_DRV_MasterReceive(uint32_t slaveAddress,
                                       const uint8_t * cmdBuff,
                                       uint32_t cmdSize,
									   uint8_t * rxBuff,
									   uint32_t rxSize)
{
	I2C_DRV_MasterReceive(&I2C1_Resources, slaveAddress, cmdBuff, cmdSize,
			rxBuff, rxSize, 0, true);

	return true;
}

// end temp
/**
  \fn          ARM_DRIVER_VERSION I2C_GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION I2C_GetVersion (void) {
  return DriverVersion;
}

/**
  \fn          ARM_I2C_CAPABILITIES I2C_GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_I2C_CAPABILITIES
*/
static ARM_I2C_CAPABILITIES I2C_GetCapabilities (void) {
  return DriverCapabilities;
}

/**
  \fn          int32_t I2Cx_Initialize (ARM_I2C_SignalEvent_t cb_event,
                                        I2C_RESOURCES         *i2c)
  \brief       Initialize I2C Interface.
  \param[in]   cb_event  Pointer to \ref ARM_I2C_SignalEvent
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_Initialize (ARM_I2C_SignalEvent_t cb_event, I2C_RESOURCES *i2c) {

  if (i2c->ctrl->flags & I2C_FLAG_POWER) {
    /* Driver initialize not allowed */
    return ARM_DRIVER_ERROR;
  }
  if (i2c->ctrl->flags & I2C_FLAG_INIT) {
    /* Driver already initialized */
    return ARM_DRIVER_OK;
  }
  i2c->ctrl->flags = I2C_FLAG_INIT;

  /* Register driver callback function */
  i2c->ctrl->cb_event = cb_event;

  /* Configure I2C Pins */
  if (i2c->reg == I2C0) {
	  PINS_PinConfigure(RTE_I2C0_SCL_PIN, RTE_I2C0_SCL_FUNC);
	  PINS_PinConfigure(RTE_I2C0_SDA_PIN, RTE_I2C0_SDA_FUNC);
  }
  else if (i2c->reg == I2C1) {
	  PINS_PinConfigure(RTE_I2C1_SCL_PIN, RTE_I2C1_SCL_FUNC);
	  PINS_PinConfigure(RTE_I2C1_SDA_PIN, RTE_I2C1_SDA_FUNC);
  }

  /* Clear driver status */
  memset (&i2c->ctrl->status, 0, sizeof(ARM_I2C_STATUS));

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_Uninitialize (I2C_RESOURCES *i2c)
  \brief       De-initialize I2C Interface.
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_Uninitialize (I2C_RESOURCES *i2c) {

  if (!(i2c->ctrl->flags & I2C_FLAG_INIT)) {
    /* Driver not initialized */
    return ARM_DRIVER_OK;
  }
  if (i2c->ctrl->flags & I2C_FLAG_POWER) {
    /* Driver needs POWER_OFF first */
    return ARM_DRIVER_ERROR;
  }
  i2c->ctrl->flags = 0;

  /* Unconfigure SCL and SDA pins */
  if (i2c->reg == I2C0) {
 	  PINS_PinConfigure(RTE_I2C0_SCL_PIN, 0);
 	  PINS_PinConfigure(RTE_I2C0_SDA_PIN, 0);
   }
   else if (i2c->reg == I2C1) {
 	  PINS_PinConfigure(RTE_I2C1_SCL_PIN, 0);
 	  PINS_PinConfigure(RTE_I2C1_SDA_PIN, 0);
   }


  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_PowerControl (ARM_POWER_STATE state,
                                          I2C_RESOURCES   *i2c)
  \brief       Control I2C Interface Power.
  \param[in]   state  Power state
  \param[in]   i2c    Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_PowerControl (ARM_POWER_STATE state, I2C_RESOURCES *i2c) {

  if (!(i2c->ctrl->flags & I2C_FLAG_INIT)) {
    /* Driver not initialized */
    return ARM_DRIVER_ERROR;
  }

  switch (state) {
    case ARM_POWER_OFF:
      if (!(i2c->ctrl->flags & I2C_FLAG_POWER)) {
        /* Driver not powered */
        break;
      }
      if (i2c->ctrl->status.busy) {
        /* Transfer operation in progress */
        return ARM_DRIVER_ERROR_BUSY;
      }
      i2c->ctrl->flags = I2C_FLAG_INIT;

      /* Disable I2C interrupts */
      NVIC_DisableIRQ (i2c->i2c_ev_irq);

      I2C_HAL_SetIntCmd(i2c->reg, false);	/* jd: disable interrupts in i2c module*/

      /* Disable I2C Operation */
      //i2c->reg->CONCLR = I2C_CON_AA | I2C_CON_SI | I2C_CON_STA | I2C_CON_I2EN;
      // jd: notes on register mapping between MKL25Z and LPC18xx:
      // I2C_C1_IICIE_MASK = I2C_CON_SI > I2C interrupt
      // I2C_C1_MST_MASK = I2C_CON_STA ? > enter master mode and transmit start on LPC
      // I2C_C1_IICEN_MASK = I2C_CON_I2EN > enable I2C interface
      //i2c->reg->C1 &= ~(I2C_C1_IICIE_MASK | I2C_C1_MST_MASK | I2C_C1_IICEN_MASK);
      //i2c->reg->C1 |= I2C_C1_TXAK_MASK;		// == clearing I2C_CON_AA in LPC.
      // Nova verze s KSDK HAL:
      //I2C_HAL_Init(i2c->reg);
      I2C_HAL_Disable(i2c->reg);

      /* Disable I2C peripheral clock */
      //*i2c->pclk_cfg_reg &= ~CCU_CLK_CFG_RUN;
      if (i2c->reg == I2C0) {
    	  *i2c->pclk_cfg_reg &= ~SIM_SCGC4_I2C0_MASK;
      } else if (i2c->reg == I2C1) {
    	  *i2c->pclk_cfg_reg &= ~SIM_SCGC4_I2C1_MASK;
      }

      break;

    case ARM_POWER_FULL:
      if (i2c->ctrl->flags & I2C_FLAG_POWER) {
        /* Driver already powered */
        break;
      }

      /* Connect base clock */
      // *i2c->base_clk_reg = (1    << 11) |   /* Autoblock En               */
      //                     (0x09 << 24) ;   /* PLL1 is APB  clock source  */
      /* Enable I2C peripheral clock */
      //*i2c->pclk_cfg_reg = CCU_CLK_CFG_AUTO | CCU_CLK_CFG_RUN;
      //while (!((*i2c->pclk_stat_reg) & CCU_CLK_STAT_RUN));
      if (i2c->reg == I2C0) {
    	  *i2c->pclk_cfg_reg |= SIM_SCGC4_I2C0_MASK;
      } else if (i2c->reg == I2C1) {
    	 // *i2c->pclk_cfg_reg |= SIM_SCGC4_I2C1_MASK;
    	  // TODO: for tests directly enabling....
    	  SIM->SCGC4 |= SIM_SCGC4_I2C1_MASK;
      }
      // jd: TODO: wait for clock running?



      /* Reset I2C peripheral */
      /*jd: not available on KL25
      LPC_RGU->RESET_CTRL1 = i2c->rgu_val;
      while (!(LPC_RGU->RESET_ACTIVE_STATUS1 & i2c->rgu_val));
      */

      /* Enable I2C Operation */
      //i2c->reg->CONCLR = I2C_CON_FLAGS;	// jd: == (I2C_CON_AA | I2C_CON_SI | I2C_CON_STO | I2C_CON_STA)
      //i2c->reg->CONSET = I2C_CON_I2EN;
      //i2c->reg->C1 &= ~(I2C_C1_IICIE_MASK | I2C_C1_MST_MASK);	// TODO: I2C_CON_STA?
      //i2c->reg->C1 |= I2C_C1_TXAK_MASK;		// == clearing I2C_CON_AA in LPC.
      //i2c->reg->C1 |= I2C_C1_IICEN_MASK;	// enable I2C module
      // KSDK version:
      I2C_HAL_Init(i2c->reg);

      i2c->ctrl->stalled = 0;
      //i2c->ctrl->con_aa  = 0;

      /* Enable I2C interrupts */
      NVIC_ClearPendingIRQ (i2c->i2c_ev_irq);
      NVIC_EnableIRQ (i2c->i2c_ev_irq);

      I2C_HAL_Enable(i2c->reg);
      I2C_HAL_SetIntCmd(i2c->reg, true);	/* jd: enable interrupts in i2c module*/

      i2c->ctrl->flags |= I2C_FLAG_POWER;
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_MasterTransmit (uint32_t       addr,
                                            const uint8_t *data,
                                            uint32_t       num,
                                            bool           xfer_pending,
                                            I2C_RESOURCES *i2c)
  \brief       Start transmitting data as I2C Master.
  \param[in]   addr          Slave address (7-bit or 10-bit)
  \param[in]   data          Pointer to buffer with data to transmit to I2C Slave
  \param[in]   num           Number of data bytes to transmit
  \param[in]   xfer_pending  Transfer operation is pending - Stop condition will not be generated
  \param[in]   i2c           Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_MasterTransmit (uint32_t       addr,
                                    const uint8_t *data,
                                    uint32_t       num,
                                    bool           xfer_pending,
                                    I2C_RESOURCES *i2c) {

  if (!data || !num || (addr > 0x7F)) {
    /* Invalid parameters */
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if (!(i2c->ctrl->flags & I2C_FLAG_SETUP)) {
    /* Driver not yet configured */
    return ARM_DRIVER_ERROR;
  }

  if (i2c->ctrl->status.busy || (i2c->ctrl->stalled & I2C_SLAVE)) {
    /* Transfer operation in progress, or Slave stalled */
    return ARM_DRIVER_ERROR_BUSY;
  }

  NVIC_DisableIRQ (i2c->i2c_ev_irq);

  /* Set control variables */
  i2c->ctrl->sla_rw  = addr << 1;
  i2c->ctrl->pending = xfer_pending;
  i2c->ctrl->data    = (uint8_t *)data;
  i2c->ctrl->num     = num;
  i2c->ctrl->cnt     = -1;

  /* Update driver status */
  i2c->ctrl->status.busy             = 1;
  i2c->ctrl->status.mode             = 1;		/* 1 = master */
  i2c->ctrl->status.direction        = 0;		/* 0 = transmit, 1 = receive */
  i2c->ctrl->status.arbitration_lost = 0;
  i2c->ctrl->status.bus_error        = 0;
  if (!i2c->ctrl->stalled) {
    //i2c->reg->CONSET = I2C_CON_STA | i2c->ctrl->con_aa;
	  //i2c->reg->C1 |= I2C_C1_MST_MASK;
	  //i2c->reg->C1 &= ~I2C_C1_TXAK_MASK;	// enable ACK
	  // jd: from KSDK
	  /* Set direction to send for sending of address and data. */
	  I2C_HAL_SetDirMode(i2c->reg, kI2CSend);
	  I2C_HAL_SendStart(i2c->reg);
	  // jd: TODO: mozna zde odeslat adresu blocking jako v KSDK
  }

  NVIC_EnableIRQ (i2c->i2c_ev_irq);

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_MasterReceive (uint32_t       addr,
                                           uint8_t       *data,
                                           uint32_t       num,
                                           bool           xfer_pending,
                                           I2C_RESOURCES *i2c)
  \brief       Start receiving data as I2C Master.
  \param[in]   addr          Slave address (7-bit or 10-bit)
  \param[out]  data          Pointer to buffer for data to receive from I2C Slave
  \param[in]   num           Number of data bytes to receive
  \param[in]   xfer_pending  Transfer operation is pending - Stop condition will not be generated
  \param[in]   i2c           Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_MasterReceive (uint32_t       addr,
                                   uint8_t       *data,
                                   uint32_t       num,
                                   bool           xfer_pending,
                                   I2C_RESOURCES *i2c) {

  if (!data || !num || (addr > 0x7F)) {
    /* Invalid parameters */ 
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if (!(i2c->ctrl->flags & I2C_FLAG_SETUP)) {
    /* Driver not yet configured */
    return ARM_DRIVER_ERROR;
  }

  if (i2c->ctrl->status.busy || (i2c->ctrl->stalled & I2C_SLAVE)) {
    /* Transfer operation in progress, or Slave stalled */
    return ARM_DRIVER_ERROR_BUSY;
  }

  NVIC_DisableIRQ (i2c->i2c_ev_irq);

  /* Set control variables */
  i2c->ctrl->sla_rw  = (addr << 1) | 0x01;
  i2c->ctrl->pending = xfer_pending;
  i2c->ctrl->data    = data;
  i2c->ctrl->num     = num;
  i2c->ctrl->cnt     = -1;

  /* Update driver status */
  i2c->ctrl->status.busy             = 1;
  i2c->ctrl->status.mode             = 1;
  i2c->ctrl->status.direction        = 0;	/* 0 = transmit, 1 = receive */
  i2c->ctrl->status.arbitration_lost = 0;
  i2c->ctrl->status.bus_error        = 0;
  if (!i2c->ctrl->stalled) {
    //i2c->reg->CONSET = I2C_CON_STA | i2c->ctrl->con_aa;
	  //i2c->reg->C1 |= I2C_C1_MST_MASK;
	  //i2c->reg->C1 &= ~I2C_C1_TXAK_MASK;	// enable ACK
	  I2C_HAL_SetDirMode(i2c->reg, kI2CSend);	/* sending address */
	  I2C_HAL_SendStart(i2c->reg);
  }

  NVIC_EnableIRQ (i2c->i2c_ev_irq);

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_SlaveTransmit (const uint8_t *data,
                                           uint32_t       num,
                                           I2C_RESOURCES *i2c)
  \brief       Start transmitting data as I2C Slave.
  \param[in]   data  Pointer to buffer with data to transmit to I2C Master
  \param[in]   num   Number of data bytes to transmit
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_SlaveTransmit (const uint8_t *data,
                                   uint32_t       num,
                                   I2C_RESOURCES *i2c) {

  if (!data || !num) {
    /* Invalid parameters */
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if (i2c->ctrl->status.busy || (i2c->ctrl->stalled & (I2C_MASTER | I2C_SLAVE_RX))) {
    /* Transfer operation in progress, Master stalled or Slave receive stalled */
    return ARM_DRIVER_ERROR_BUSY;
  }

  NVIC_DisableIRQ (i2c->i2c_ev_irq);

  /* Set control variables */
  i2c->ctrl->flags &= ~I2C_FLAG_SLAVE_RX;
  i2c->ctrl->sdata  = (uint8_t *)data;
  i2c->ctrl->snum   = num;
  i2c->ctrl->cnt    = -1;

  /* Update driver status */
  i2c->ctrl->status.general_call = 0;
  i2c->ctrl->status.bus_error    = 0;

  NVIC_EnableIRQ (i2c->i2c_ev_irq);

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_SlaveReceive (uint8_t       *data,
                                          uint32_t       num,
                                          I2C_RESOURCES *i2c)
  \brief       Start receiving data as I2C Slave.
  \param[out]  data  Pointer to buffer for data to receive from I2C Master
  \param[in]   num   Number of data bytes to receive
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_SlaveReceive (uint8_t       *data,
                                  uint32_t       num,
                                  I2C_RESOURCES *i2c) {

  if (!data || !num) {
    /* Invalid parameters */ 
    return ARM_DRIVER_ERROR_PARAMETER;
  }

  if (i2c->ctrl->status.busy || (i2c->ctrl->stalled & (I2C_MASTER | I2C_SLAVE_TX))) {
    /* Transfer operation in progress, Master stalled or Slave transmit stalled */
    return ARM_DRIVER_ERROR_BUSY;
  }

  NVIC_DisableIRQ (i2c->i2c_ev_irq);

  /* Set control variables */
  i2c->ctrl->flags |= I2C_FLAG_SLAVE_RX;
  i2c->ctrl->sdata  = data;
  i2c->ctrl->snum   = num;
  i2c->ctrl->cnt    = -1;

  /* Update driver status */
  i2c->ctrl->status.general_call = 0;
  i2c->ctrl->status.bus_error    = 0;

  NVIC_EnableIRQ (i2c->i2c_ev_irq);

  return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_GetDataCount (I2C_RESOURCES *i2c)
  \brief       Get transferred data count.
  \return      number of data bytes transferred; -1 when Slave is not addressed by Master
*/
static int32_t I2Cx_GetDataCount (I2C_RESOURCES *i2c) {
  return (i2c->ctrl->cnt);
}

/**
  \fn          int32_t I2Cx_Control (uint32_t       control,
                                     uint32_t       arg,
                                     I2C_RESOURCES *i2c)
  \brief       Control I2C Interface.
  \param[in]   control  operation
  \param[in]   arg      argument of operation (optional)
  \param[in]   i2c      pointer to I2C resources
  \return      \ref execution_status
*/
static int32_t I2Cx_Control (uint32_t control, uint32_t arg, I2C_RESOURCES *i2c) {
  uint32_t val, clk;	// conset;
  //uint8_t mul, icr;

  if (!(i2c->ctrl->flags & I2C_FLAG_POWER)) {
    /* Driver not powered */
    return ARM_DRIVER_ERROR;
  }
  switch (control) {
    case ARM_I2C_OWN_ADDRESS:
      /* Set Own Slave Address */
      val = (arg << 1) & 0xFF;
      if (arg & ARM_I2C_ADDRESS_GC) {
        /* General call enable */
        //val |= 0x01;
    	//i2c->reg->C2 |= I2C_C2_GCAEN_MASK;
    	  I2C_HAL_SetGeneralCallCmd(i2c->reg, true);
      }
      else {
    	  // jd: added else to disable general call
    	  //i2c->reg->C2  &= ~I2C_C2_GCAEN_MASK;
    	  I2C_HAL_SetGeneralCallCmd(i2c->reg, false);
      }
      //i2c->reg->ADR0 = val;
      i2c->reg->A1 = val;		// jd: 7-bit address; same as on LPC18xx
      // or I2C_HAL_SetAddress7bit but without shift above!

      /* Enable assert acknowledge */
      /*if (val) val = I2C_CON_AA;
      i2c->ctrl->con_aa = val;
      i2c->reg->CONSET  = val;*/
      if ( val )
    	  i2c->reg->C1 &= ~I2C_C1_TXAK_MASK;	// enable ACK
      // KSDK has only I2C_HAL_SendAck which does the same
      break;

    case ARM_I2C_BUS_SPEED:
      /* Set Bus Speed */
      clk = GetClockFreq (0);
      switch (arg) {
        case ARM_I2C_BUS_SPEED_STANDARD:
          /* Standard Speed (100kHz) */
          //clk /= 100000;
          I2C_HAL_SetBaudRate(i2c->reg, clk, 100, NULL);
          break;
        case ARM_I2C_BUS_SPEED_FAST:
          /* Fast Speed     (400kHz) */
          //clk /= 400000;
          I2C_HAL_SetBaudRate(i2c->reg, clk, 400, NULL);
          break;
#if 0	/* jd: not supported for now */
        case ARM_I2C_BUS_SPEED_FAST_PLUS:
          /* Fast+ Speed    (  1MHz) */
          if (i2c->reg == LPC_I2C0) {
            clk /= 1000000;
            break;
          }
#endif
        default:
          return ARM_DRIVER_ERROR_UNSUPPORTED;
      }
      /* Improve accuracy */
      //i2c->reg->SCLH = clk / 2;
      //i2c->reg->SCLL = clk - i2c->reg->SCLH;
      /* jd: MUL * ICR = BUS/Baud
       * clk is now BUS/Baud
       * ICR must fit 6 bits -> ICR < 63 (0x3F)
       * Max bus clock is 24 MHz, with min speed 100 kHz this gives:
       * MUL * ICR = 24000000 / 100000 = 240
       * So we need to use MUL "dynamically", it cannot be 1 for all cases.
       * */
#if 0	/* replaced by KSDK HAL I2C_HAL_SetBaudRate */
      mul = 1;
      tmp_clk = clk;
      while ( (tmp_clk > 63) && (mult < 4)  )
      {
    	  tmp_clk = clk;
    	  mul = mul << 1;
    	  tmp_clk = tpm_clk / mul;	// try ICR value with this mult
      }
      icr = (uint8_t)(clk/mul);	// now calculate the real value of ICR
      i2c->reg->F = (((mul-1) < 6) | icr);
#endif


      /* Speed configured, I2C Master active */
      i2c->ctrl->flags |= I2C_FLAG_SETUP;
      break;

    case ARM_I2C_BUS_CLEAR:
      /* Execute Bus clear */
      return ARM_DRIVER_ERROR_UNSUPPORTED;

    case ARM_I2C_ABORT_TRANSFER:
      /* Abort Master/Slave transfer */
      NVIC_DisableIRQ (i2c->i2c_ev_irq);

      i2c->ctrl->status.busy = 0;
      i2c->ctrl->stalled = 0;
      i2c->ctrl->snum    = 0;
      /* Master: send STOP to I2C bus           */
      /* Slave:  enter non-addressed Slave mode */
      // jd: writing STO in master mode sends STOP signal to bus,
      // in slave mode it recovers from error condition.
      //conset = I2C_CON_STO | i2c->ctrl->con_aa;
      //i2c->reg->CONSET = conset;
      //i2c->reg->CONCLR = conset ^ I2C_CON_FLAGS;
      // jd: I2C_CON_FLAGS == (I2C_CON_AA | I2C_CON_SI | I2C_CON_STO | I2C_CON_STA)
      //i2c->reg->C1 &= ~(I2C_C1_IICIE_MASK | I2C_C1_MST_MASK);	// TODO: I2C_CON_STA?
      //i2c->reg->C1 |= I2C_C1_TXAK_MASK;		// == clearing I2C_CON_AA in LPC.
      // new version:
      I2C_HAL_SendStop(i2c->reg);

      NVIC_EnableIRQ (i2c->i2c_ev_irq);
      break;

    default:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
  return ARM_DRIVER_OK;
}

/**
  \fn          ARM_I2C_STATUS I2Cx_GetStatus (I2C_RESOURCES *i2c)
  \brief       Get I2C status.
  \param[in]   i2c      pointer to I2C resources
  \return      I2C status \ref ARM_I2C_STATUS
*/
static ARM_I2C_STATUS I2Cx_GetStatus (I2C_RESOURCES *i2c) {
  return (i2c->ctrl->status);
}

/**
  \fn          void I2Cx_MasterHandler (I2C_RESOURCES *i2c)
  \brief       I2C Master state event handler.
  \param[in]   i2c  Pointer to I2C resources
  \return      I2C event notification flags
*/
static uint32_t I2Cx_MasterHandler (I2C_RESOURCES *i2c) {
  //uint32_t conset = i2c->ctrl->con_aa;
  uint32_t event  = 0;

  /* jd: Clear the interrupt flag*/
  I2C_HAL_ClearInt(i2c->reg);

  if (i2c->ctrl->stalled) {
    /* Master resumes with repeated START here */
    /* Stalled states: I2C_STAT_MA_DT_A        */
    /*                 I2C_STAT_MA_DR_NA       */
    i2c->ctrl->stalled = 0;
    //conset |= I2C_CON_STA;
    //goto write_con;
    I2C_HAL_SendStart(i2c->reg);
    return (event);	// nothing else to do
  }


  /* pseudokod pro moje udalosti
  Pouzit KSDK z fsl_i2c_shared_function.c
  Udalosti, ktere by mely byt osetreny podle LPC kodu nize:
  1) Bus error (timeout? - neni to arbitration lost viz nize!
  2) Byl vysilan start nebo repeated start
  Odeslan prikaz pro slave + W tj. posilam ti data?
   = SLA+W transmitted, no ACK received
   = SLA+W transmitted, ACK received
  Odeslan prikaz pro slave + R tj. cekam na data?
   = SLA+R transmitted, no ACK received
   = SLA+R transmitted, ACK received
  Odesla data ok = Data transmitted, ACK received
  Prisla data ok = Data received, ACK returned
  Prisla data s no ack = Data received, no ACK returned
  Arbitration lost
  */

  /* Get current master transfer direction */
  i2c_direction_t direction = I2C_HAL_GetDirMode(i2c->reg);

  	// Bus error (timeout? - neni to arbitration lost viz nize!
	if (!I2C_HAL_GetStatusFlag(i2c->reg, kI2CBusBusy)) {
		// If no transfer in progress
		// using I2C_STAT_BUSERR code from LPC
		i2c->ctrl->status.bus_error = 1;
		i2c->ctrl->status.busy = 0;
		i2c->ctrl->status.mode = 0;
		event = ARM_I2C_EVENT_BUS_ERROR |
		ARM_I2C_EVENT_TRANSFER_DONE |
		ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
		return (event);	// nothing else to do
	}

	/* Arbitration lost */
	/* Using code for I2C_STAT_MA_ALOST - Arbitration lost */
	if ( I2C_HAL_GetStatusFlag(i2c->reg, kI2CArbitrationLost) )	{
	      i2c->ctrl->status.arbitration_lost = 1;
	      i2c->ctrl->status.busy             = 0;
	      i2c->ctrl->status.mode             = 0;
	      event = ARM_I2C_EVENT_ARBITRATION_LOST |
	              ARM_I2C_EVENT_TRANSFER_DONE    |
	              ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
	      CompleteTransfer(i2c);
	      return (event);
	}

	/* Further handling is dependent on whether we are sending or receiving
	 * Based on KSDK fsl_i2c_master_driver.c IRQ handler */
	if (direction == kI2CSend) {
		/* We are sending data */
		/* Check whether we got an ACK or NAK from the former byte we sent */
		if (I2C_HAL_GetStatusFlag(i2c->reg, kI2CReceivedNak)) {

			/* using code for events:
			 *  I2C_STAT_MA_SLAW_NA - SLA+W transmitted, no ACK received
			 *  I2C_STAT_MA_SLAR_NA - SLA+R transmitted, no ACK received
			 *  I2C_STAT_MA_DT_NA: -  Data transmitted, no ACK received
			 */
			i2c->ctrl->status.busy = 0;
			i2c->ctrl->status.mode = 0;
			event = ARM_I2C_EVENT_ADDRESS_NACK |
			ARM_I2C_EVENT_TRANSFER_DONE |
			ARM_I2C_EVENT_TRANSFER_INCOMPLETE;

			/* Got a NAK, so we're done with this transfer */
			CompleteTransfer(i2c);

		} else {
			/* Asi zde je i event po odeslani adresy...*/
			// I2C_STAT_MA_START - START transmitted */
			// I2C_STAT_MA_RSTART: - Repeated START transmitted */
			//i2c->reg->DAT = i2c->ctrl->sla_rw;	// jd: send slave address and RW bit
			if ( i2c->ctrl->cnt == -1 )
			{
				// jd: send slave address and RW bit
				I2C_HAL_WriteByte(i2c->reg, i2c->ctrl->sla_rw);
				i2c->ctrl->cnt = 0;
				/* jd: it seems the LPC I2C module automatically handles switching Tx/Rx
				 * mode based on the RW bit in slave address and the original LPC code thus
				 * receives different events for write and read command; but in KL25Z we
				 * need to keep track of this manually. */

				/* /* Note:  r/w - read is 1, write is 0. */
				if ( (i2c->ctrl->sla_rw & 0x01) == 0) { /* write: SLA+W */
					i2c->ctrl->status.direction = 0;
				} else { /* read: SLA+R*/
					 i2c->ctrl->status.direction = 1;	/* now we are received */
				}


			/* TODO: handle 10-bit address. KSDK handles it and sends blocking in
			 * I2C_DRV_SendAddress, how is it handled in LPC driver? is it at all? */
			/* KSDK: Send out slave address. */
			//I2C_DRV_SendAddress(instance, device, cmdBuff, cmdSize, kI2CSend, timeout_ms);
			} else if ( i2c->ctrl->cnt == 0 ) {
				if ( i2c->ctrl->status.direction == 0 ) {
					//I2C_STAT_MA_SLAW_A: -  SLA+W transmitted, ACK received */
					/* jd: this means now we start sending data*/
					i2c->reg->D  = i2c->ctrl->data[0];
				} else {
					// I2C_STAT_MA_SLAR_A - SLA+R transmitted, ACK received
					// switch direction to receiving
					 I2C_HAL_SetDirMode(i2c->reg, kI2CReceive);
				}

			} else {	/* transfer of byte 1+ */

				/* Using code for I2C_STAT_MA_DT_A */
				/* Data transmitted, ACK received */
				i2c->ctrl->cnt++;
				i2c->ctrl->num--;
				if (!i2c->ctrl->num) {
					//goto xfer_done;
					i2c->ctrl->status.busy = 0;
					event = ARM_I2C_EVENT_TRANSFER_DONE;
					if (i2c->ctrl->pending) {
						/* Stall I2C transaction */
						NVIC_DisableIRQ(i2c->i2c_ev_irq);
						i2c->ctrl->stalled = I2C_MASTER;
						return (event);
					}
					/* Generate STOP */
					CompleteTransfer(i2c);
				}
			}
		} /* else == ACK received */

	} else  { /* We are receiving data */

		/* Using code for I2C_STAT_MA_DR_NA - Data received, no ACK returned */
		/* jd: This is normal end of transfer - slave sent us all data */
		if (I2C_HAL_GetStatusFlag(i2c->reg, kI2CReceivedNak)) {
			i2c->ctrl->data[i2c->ctrl->cnt++] = I2C_HAL_ReadByte(i2c->reg);
			i2c->ctrl->num--;
			i2c->ctrl->status.busy = 0;
			event = ARM_I2C_EVENT_TRANSFER_DONE;
			if (i2c->ctrl->pending) {
				/* Stall I2C transaction */
				NVIC_DisableIRQ(i2c->i2c_ev_irq);
				i2c->ctrl->stalled = I2C_MASTER;
				return (event);
			}
			/* Generate STOP */
			CompleteTransfer(i2c);

		} else {	/* ACK returned  */

			/* Using code for I2C_STAT_MA_DR_A - Data received, ACK returned */
			//i2c->ctrl->data[i2c->ctrl->cnt++] = i2c->reg->DAT;
			i2c->ctrl->data[i2c->ctrl->cnt++] = I2C_HAL_ReadByte(i2c->reg);
			i2c->ctrl->num--;
			if (i2c->ctrl->num > 1)
				I2C_HAL_SendAck(i2c->reg);
			else if  (i2c->ctrl->num == 1)
				 /* For the byte before last, we need to set NAK */
				 I2C_HAL_SendNak(i2c->reg);
			else	{ /* num == 0*/
				/* Note: KSDK uses this strategy, i.e. if the count of expected data
				  reaches 0, we end transfer.
				  LPC code does not handle this case in event I2C_STAT_MA_DR_A
				  it has special event I2C_STAT_MA_DR_NA - Data received, no ACK returned,
				  which means end of transfer from slave. Because on I2C if we
				  receive data with no ACK, it means "end of transfer" from slave (?)

				  Here I copy the same code as for I2C_STAT_MA_DR_NA
				  TODO: see which version really happens? - this one or the one above for NACK
				  both are possible if slave sends more data than we expect...
				 */
				i2c->ctrl->data[i2c->ctrl->cnt++] = I2C_HAL_ReadByte(i2c->reg);
				i2c->ctrl->num--;
				i2c->ctrl->status.busy = 0;
				event = ARM_I2C_EVENT_TRANSFER_DONE;
				if (i2c->ctrl->pending) {
					/* Stall I2C transaction */
					NVIC_DisableIRQ(i2c->i2c_ev_irq);
					i2c->ctrl->stalled = I2C_MASTER;
					return (event);
				}
				/* Finish receive data, send STOP, disable interrupt */
				CompleteTransfer(i2c);
			}



		}	/* else == ACK returned*/

	}	/* else == receiving */

	return (event);




 /* jd: LPC has status register STAT which contains status codes.
  * KSDK has enum i2c_status_flag_t in fsl_i2c_hal.h with similar codes..*/
#if 0  /* jd: ? */
	switch (i2c->reg->STAT & 0xF8) {

    case I2C_STAT_BUSERR:
      /* I2C Bus error */
      i2c->ctrl->status.bus_error = 1;
      i2c->ctrl->status.busy      = 0;
      i2c->ctrl->status.mode      = 0;
      event = ARM_I2C_EVENT_BUS_ERROR      |
              ARM_I2C_EVENT_TRANSFER_DONE  |
              ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
      conset |= I2C_CON_STO;
      break;
#endif

#if 0
    case I2C_STAT_MA_START:
      /* START transmitted */
    case I2C_STAT_MA_RSTART:
      /* Repeated START transmitted */
      i2c->reg->DAT = i2c->ctrl->sla_rw;	/* jd: send slave address and RW bit */
      break;
#endif

#if 0
    case I2C_STAT_MA_SLAW_NA:
      /* SLA+W transmitted, no ACK received */
    case I2C_STAT_MA_SLAR_NA:
      /* SLA+R transmitted, no ACK received */
      i2c->ctrl->status.busy = 0;
      i2c->ctrl->status.mode = 0;
      event = ARM_I2C_EVENT_ADDRESS_NACK   |
              ARM_I2C_EVENT_TRANSFER_DONE  |
              ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
      conset |= I2C_CON_STO;
      break;
#endif

#if 0
    case I2C_STAT_MA_SLAW_A:
      /* SLA+W transmitted, ACK received */
    	/* jd: this means now we start sending data*/
      i2c->ctrl->cnt = 0;
      i2c->reg->DAT  = i2c->ctrl->data[0];
      break;
#endif

#if 0
    case I2C_STAT_MA_DT_A:
      /* Data transmitted, ACK received */
      i2c->ctrl->cnt++;
      i2c->ctrl->num--;
      if (!i2c->ctrl->num) {
        goto xfer_done;
      }
      /* Send next byte */
      i2c->reg->DAT = i2c->ctrl->data[i2c->ctrl->cnt];
      break;
#endif
#if 0
    case I2C_STAT_MA_DT_NA:
      /* Data transmitted, no ACK received */
      i2c->ctrl->status.busy = 0;
      i2c->ctrl->status.mode = 0;
      event = ARM_I2C_EVENT_TRANSFER_DONE  |
              ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
      conset |= I2C_CON_STO;
      break;
    case I2C_STAT_MA_ALOST:
      /* Arbitration lost */
      i2c->ctrl->status.arbitration_lost = 1;
      i2c->ctrl->status.busy             = 0;
      i2c->ctrl->status.mode             = 0;
      event = ARM_I2C_EVENT_ARBITRATION_LOST |
              ARM_I2C_EVENT_TRANSFER_DONE    |
              ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
      break;
#endif
#if 0
    case I2C_STAT_MA_SLAR_A:
      /* SLA+R transmitted, ACK received */
      i2c->ctrl->cnt = 0;
      i2c->ctrl->status.direction = 1;	/* jd: now we are received */
      goto upd_conset;
#endif
#if 0
   case I2C_STAT_MA_DR_A:
      /* Data received, ACK returned */
      i2c->ctrl->data[i2c->ctrl->cnt++] = i2c->reg->DAT;
      i2c->ctrl->num--;
upd_conset:
      conset = 0;
      if (i2c->ctrl->num > 1) {
        conset = I2C_CON_AA;
      }
      break;
#endif

#if 0
    case I2C_STAT_MA_DR_NA:
      /* Data received, no ACK returned */
      i2c->ctrl->data[i2c->ctrl->cnt++] = i2c->reg->DAT;
      i2c->ctrl->num--;
xfer_done:
      i2c->ctrl->status.busy = 0;
      event = ARM_I2C_EVENT_TRANSFER_DONE;
      if (i2c->ctrl->pending) {
        /* Stall I2C transaction */
        NVIC_DisableIRQ (i2c->i2c_ev_irq);
        i2c->ctrl->stalled = I2C_MASTER;
        return (event);
      }
      /* Generate STOP */
      conset |= I2C_CON_STO;
      break;
#endif
#if 0
  }
write_con:
  /* Set/clear control flags */
  i2c->reg->CONSET = conset;
  i2c->reg->CONCLR = conset ^ I2C_CON_FLAGS;
jd_end:
  return (event);
#endif
}

/**
  \fn          void I2Cx_SlaveHandler (I2C_RESOURCES *i2c)
  \brief       I2C Slave state event handler.
  \param[in]   i2c  Pointer to I2C resources
  \return      I2C event notification flags
*/
static uint32_t I2Cx_SlaveHandler (I2C_RESOURCES *i2c) {

	/* jd: not implemented */
	uint32_t event  = 0;
	event = ARM_I2C_EVENT_TRANSFER_DONE |
	                 ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
	return (event);
}

#if 0
  uint32_t conset = 0;
  uint32_t event  = 0;

  switch (i2c->reg->STAT & 0xF8) {
    case I2C_STAT_SL_ALOST_GC:
      /* Arbitration lost in General call */
      i2c->ctrl->status.arbitration_lost = 1;
    case I2C_STAT_SL_GCA_A:
      /* General address recvd, ACK returned */
      i2c->ctrl->status.general_call     = 1;
      goto slaw_a;

    case I2C_STAT_SL_ALOST_MW:
      /* Arbitration lost SLA+W */
      i2c->ctrl->status.arbitration_lost = 1;
    case I2C_STAT_SL_SLAW_A:
      /* SLA+W received, ACK returned */
slaw_a:
      /* Stalled Slave receiver also resumes here */
      if (!i2c->ctrl->snum || !(i2c->ctrl->flags & I2C_FLAG_SLAVE_RX)) {
        /* Receive buffer unavailable */
        if (i2c->ctrl->stalled) {
          /* Already stalled, abort transaction to prevent dead-loops */
          event = ARM_I2C_EVENT_TRANSFER_DONE |
                  ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
          conset = I2C_CON_STO | i2c->ctrl->con_aa;
          break;
        }
        /* Stall I2C transaction */
        NVIC_DisableIRQ (i2c->i2c_ev_irq);
        i2c->ctrl->stalled = I2C_SLAVE_RX;
        return (ARM_I2C_EVENT_SLAVE_RECEIVE);
      }
      i2c->ctrl->status.direction = 1;
      i2c->ctrl->status.busy      = 1;
      i2c->ctrl->cnt     = 0;
      i2c->ctrl->stalled = 0;
      conset = I2C_CON_AA;
      break;

    case I2C_STAT_SL_DRGC_A:
      /* Data recvd General call, ACK returned */
    case I2C_STAT_SL_DR_A:
      /* Data received, ACK returned */
      i2c->ctrl->sdata[i2c->ctrl->cnt++] = i2c->reg->DAT;
      i2c->ctrl->snum--;
      if (i2c->ctrl->snum) {
        conset = I2C_CON_AA;
      }
      break;

    case I2C_STAT_SL_ALOST_MR:
      /* Arbitration lost SLA+R */
      i2c->ctrl->status.arbitration_lost = 1;
    case I2C_STAT_SL_SLAR_A:
      /* SLA+R received, ACK returned */
      /* Stalled Slave transmitter also resumes here */
      if (!i2c->ctrl->snum || (i2c->ctrl->flags & I2C_FLAG_SLAVE_RX)) {
        /* Transmit buffer unavailable */
        if (i2c->ctrl->stalled) {
          /* Already stalled, abort transaction to prevent dead-loops */
          event = ARM_I2C_EVENT_TRANSFER_DONE |
                  ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
          conset = I2C_CON_STO | i2c->ctrl->con_aa;
          break;
        }
        NVIC_DisableIRQ (i2c->i2c_ev_irq);
        i2c->ctrl->stalled = I2C_SLAVE_TX;
        return (ARM_I2C_EVENT_SLAVE_TRANSMIT);
      }
      i2c->ctrl->status.direction = 0;
      i2c->ctrl->status.busy      = 1;
      i2c->ctrl->cnt     = 0;
      i2c->ctrl->stalled = 0;
    case I2C_STAT_SL_DT_A:
      /* Data transmitted, ACK received */
      i2c->reg->DAT = i2c->ctrl->sdata[i2c->ctrl->cnt++];
      i2c->ctrl->snum--;
      if (i2c->ctrl->snum) {
        conset = I2C_CON_AA;
      }
      break;

    case I2C_STAT_SL_DT_NA:
      /* Data transmitted, no ACK received */
    case I2C_STAT_SL_LDT_A:
      /* Last data transmitted, ACK received */
    case I2C_STAT_SL_DR_NA:
      /* Data received, no ACK returned */
    case I2C_STAT_SL_DRGC_NA:
      /* Data recvd General call, no ACK returned */
    case I2C_STAT_SL_STOP:
      /* STOP received while addressed */
      i2c->ctrl->status.busy = 0;
      /* Slave operation completed, generate events */
      event = ARM_I2C_EVENT_TRANSFER_DONE;
      if (i2c->ctrl->status.arbitration_lost) {
        event |= ARM_I2C_EVENT_ARBITRATION_LOST;
      }
      if (i2c->ctrl->status.general_call) {
        event |= ARM_I2C_EVENT_GENERAL_CALL;
      }
      if (i2c->ctrl->snum) {
        event |= ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
      }
      conset = i2c->ctrl->con_aa;
      break;
  }
  /* Set/clear control flags */
  i2c->reg->CONSET = conset;
  i2c->reg->CONCLR = conset ^ I2C_CON_FLAGS;

  return (event);
}
#endif

/**
  \fn          void I2Cx_IRQHandler (I2C_RESOURCES *i2c)
  \brief       I2C Event Interrupt handler.
  \param[in]   i2c  Pointer to I2C resources
*/
static void I2Cx_IRQHandler (I2C_RESOURCES *i2c) {
  uint32_t event;

  // jd:
  if (I2C_HAL_IsMaster(i2c->reg) )
	  event = I2Cx_MasterHandler (i2c);
  else
	  event = I2Cx_SlaveHandler (i2c);

  /*
  if (i2c->reg->STAT < I2C_STAT_SL_SLAW_A) {
    event = I2Cx_MasterHandler (i2c);
  }
  else {
    event = I2Cx_SlaveHandler (i2c);
  }
  */

  /* Callback event notification */
  if (event && i2c->ctrl->cb_event) {
    i2c->ctrl->cb_event (event);
  }
}

#if (RTE_I2C0)
/* I2C0 Driver wrapper functions */
static int32_t I2C0_Initialize (ARM_I2C_SignalEvent_t cb_event) {
  return (I2Cx_Initialize (cb_event, &I2C0_Resources));
}
static int32_t I2C0_Uninitialize (void) {
  return (I2Cx_Uninitialize (&I2C0_Resources));
}
static int32_t I2C0_PowerControl (ARM_POWER_STATE state) {
  return (I2Cx_PowerControl (state, &I2C0_Resources));
}
static int32_t I2C0_MasterTransmit (uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending) {
  return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, &I2C0_Resources));
}
static int32_t I2C0_MasterReceive (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending) {
  return (I2Cx_MasterReceive (addr, data, num, xfer_pending, &I2C0_Resources));
}
static int32_t I2C0_SlaveTransmit (const uint8_t *data, uint32_t num) {
  return (I2Cx_SlaveTransmit (data, num, &I2C0_Resources));
}
static int32_t I2C0_SlaveReceive (uint8_t *data, uint32_t num) {
  return (I2Cx_SlaveReceive (data, num, &I2C0_Resources));
}
static int32_t I2C0_GetDataCount (void) {
  return (I2Cx_GetDataCount (&I2C0_Resources));
}
static int32_t I2C0_Control (uint32_t control, uint32_t arg) {
  return (I2Cx_Control (control, arg, &I2C0_Resources));
}
static ARM_I2C_STATUS I2C0_GetStatus (void) {
  return (I2Cx_GetStatus (&I2C0_Resources));
}
void I2C0_IRQHandler (void) {
  I2Cx_IRQHandler (&I2C0_Resources);
}

/* I2C0 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C0 = {
  I2C_GetVersion,
  I2C_GetCapabilities,
  I2C0_Initialize,
  I2C0_Uninitialize,
  I2C0_PowerControl,
  I2C0_MasterTransmit,
  I2C0_MasterReceive,
  I2C0_SlaveTransmit,
  I2C0_SlaveReceive,
  I2C0_GetDataCount,
  I2C0_Control,
  I2C0_GetStatus
};
#endif

#if (RTE_I2C1)
/* I2C1 Driver wrapper functions */
static int32_t I2C1_Initialize (ARM_I2C_SignalEvent_t cb_event) {
  return (I2Cx_Initialize (cb_event, &I2C1_Resources));
}
static int32_t I2C1_Uninitialize (void) {
  return (I2Cx_Uninitialize (&I2C1_Resources));
}
static int32_t I2C1_PowerControl (ARM_POWER_STATE state) {
  return (I2Cx_PowerControl (state, &I2C1_Resources));
}
static int32_t I2C1_MasterTransmit (uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending) {
  return (I2Cx_MasterTransmit (addr, data, num, xfer_pending, &I2C1_Resources));
}
static int32_t I2C1_MasterReceive (uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending) {
  return (I2Cx_MasterReceive (addr, data, num, xfer_pending, &I2C1_Resources));
}
static int32_t I2C1_SlaveTransmit (const uint8_t *data, uint32_t num) {
  return (I2Cx_SlaveTransmit (data, num, &I2C1_Resources));
}
static int32_t I2C1_SlaveReceive (uint8_t *data, uint32_t num) {
  return (I2Cx_SlaveReceive (data, num, &I2C1_Resources));
}
static int32_t I2C1_GetDataCount (void) {
  return (I2Cx_GetDataCount (&I2C1_Resources));
}
static int32_t I2C1_Control (uint32_t control, uint32_t arg) {
  return (I2Cx_Control (control, arg, &I2C1_Resources));
}
static ARM_I2C_STATUS I2C1_GetStatus (void) {
  return (I2Cx_GetStatus (&I2C1_Resources));
}
void I2C1_IRQHandler (void) {
  I2Cx_IRQHandler (&I2C1_Resources);
}

/* I2C1 Driver Control Block */
ARM_DRIVER_I2C Driver_I2C1 = {
  I2C_GetVersion,
  I2C_GetCapabilities,
  I2C1_Initialize,
  I2C1_Uninitialize,
  I2C1_PowerControl,
  I2C1_MasterTransmit,
  I2C1_MasterReceive,
  I2C1_SlaveTransmit,
  I2C1_SlaveReceive,
  I2C1_GetDataCount,
  I2C1_Control,
  I2C1_GetStatus
};
#endif
