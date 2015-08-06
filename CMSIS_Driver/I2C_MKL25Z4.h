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
 * Project:      I2C Driver Definitions for Freescale MKL25Z4
 * -------------------------------------------------------------------------- */

#ifndef __I2C_LPC18XX_H
#define __I2C_LPC18XX_H

#include "MKL25Z4.h"

#include "Driver_I2C.h"

/* jd TODO: howto declare the driver instances?*/
#if 0
#if (RTE_I2C0)
extern ARM_DRIVER_I2C Driver_I2C0;
#endif

#if (RTE_I2C1)
extern ARM_DRIVER_I2C Driver_I2C1;
#endif
#endif

// todo: jen pro testy:
bool I2C1_DRV_MasterReceive(	uint32_t slaveAddress,
                                          const uint8_t * cmdBuff,
                                          uint32_t cmdSize,
                                          uint8_t * rxBuff,
                                          uint32_t rxSize
                                         );

bool I2C1_DRV_MasterSend(uint32_t slaveAddress,
                                       const uint8_t * cmdBuff,
                                       uint32_t cmdSize,
                                       const uint8_t * txBuff,
                                       uint32_t txSize
                                       );

/* Clock Control Unit register */
/* jd: not used
#define CCU_CLK_CFG_RUN     (1 << 0)
#define CCU_CLK_CFG_AUTO    (1 << 1)
#define CCU_CLK_STAT_RUN    (1 << 0)

#define CLK_SRC_PLL1        0x09            // I2C clock source
*/

/* I2C reset value for RGU */
#define RGU_RESET_I2C0      (1 << 16)       // I2C0 reset
#define RGU_RESET_I2C1      (1 << 17)       // I2C1 reset

/* I2C Driver state flags */
#define I2C_FLAG_INIT       (1 << 0)        // Driver initialized
#define I2C_FLAG_POWER      (1 << 1)        // Driver power on
#define I2C_FLAG_SETUP      (1 << 2)        // Master configured, clock set
#define I2C_FLAG_SLAVE_RX   (1 << 3)        // Slave receive registered

/* I2C Common Control flags */
/* jd: we are using <device>.h flags
#define I2C_CON_AA          (1 << 2)        // Assert acknowledge bit
#define I2C_CON_SI          (1 << 3)        // I2C interrupt bit
#define I2C_CON_STO         (1 << 4)        // STOP bit
#define I2C_CON_STA         (1 << 5)        // START bit
#define I2C_CON_I2EN        (1 << 6)        // I2C interface enable
#define I2C_CON_FLAGS       (I2C_CON_AA | I2C_CON_SI | I2C_CON_STO | I2C_CON_STA)
*/

/* I2C Stalled Status flags */
#define I2C_MASTER          (1 << 0)        // Master stalled
#define I2C_SLAVE_TX        (1 << 1)        // Slave stalled on transmit
#define I2C_SLAVE_RX        (1 << 2)        // Slave stalled on receive
#define I2C_SLAVE           (I2C_SLAVE_TX | I2C_SLAVE_RX)

/* I2C Status Miscellaneous states */
#define I2C_STAT_BUSERR      0x00           // I2C Bus error

/* I2C Status Master mode */
#define I2C_STAT_MA_START    0x08           // START transmitted
#define I2C_STAT_MA_RSTART   0x10           // Repeated START transmitted
#define I2C_STAT_MA_SLAW_A   0x18           // SLA+W transmitted, ACK received
#define I2C_STAT_MA_SLAW_NA  0x20           // SLA+W transmitted, no ACK recvd
#define I2C_STAT_MA_DT_A     0x28           // Data transmitted, ACK received
#define I2C_STAT_MA_DT_NA    0x30           // Data transmitted, no ACK recvd
#define I2C_STAT_MA_ALOST    0x38           // Arbitration lost SLA+W or data
#define I2C_STAT_MA_SLAR_A   0x40           // SLA+R transmitted, ACK received
#define I2C_STAT_MA_SLAR_NA  0x48           // SLA+R transmitted, no ACK recvd
#define I2C_STAT_MA_DR_A     0x50           // Data received, ACK returned
#define I2C_STAT_MA_DR_NA    0x58           // Data received, no ACK returned

/* I2C Status Slave mode */
#define I2C_STAT_SL_SLAW_A   0x60           // SLA+W received, ACK returned
#define I2C_STAT_SL_ALOST_MW 0x68           // Arbitration lost SLA+W in Master mode
#define I2C_STAT_SL_GCA_A    0x70           // General address recvd, ACK returned
#define I2C_STAT_SL_ALOST_GC 0x78           // Arbitration lost in General call
#define I2C_STAT_SL_DR_A     0x80           // Data received, ACK returned
#define I2C_STAT_SL_DR_NA    0x88           // Data received, no ACK returned
#define I2C_STAT_SL_DRGC_A   0x90           // Data recvd General call, ACK returned
#define I2C_STAT_SL_DRGC_NA  0x98           // Data recvd General call, no ACK returned
#define I2C_STAT_SL_STOP     0xA0           // STOP received while addressed
#define I2C_STAT_SL_SLAR_A   0xA8           // SLA+R received, ACK returned
#define I2C_STAT_SL_ALOST_MR 0xB0           // Arbitration lost SLA+R in Master mode
#define I2C_STAT_SL_DT_A     0xB8           // Data transmitted, ACK received
#define I2C_STAT_SL_DT_NA    0xC0           // Data transmitted, no ACK received
#define I2C_STAT_SL_LDT_A    0xC8           // Last data transmitted, ACK received

/* I2C Control Information */
typedef struct {
  ARM_I2C_SignalEvent_t cb_event;           // Event callback
  ARM_I2C_STATUS        status;             // Status flags
  uint8_t               flags;              // Control and state flags
  uint8_t               sla_rw;             // Slave address and RW bit
  bool                  pending;            // Transfer pending (no STOP)
  uint8_t               stalled;            // Stall mode status flags;
  // jd: stalled means after MasterTransmit a MasterReceive will follow, this is used
  // when sending command to slave to which slave sends data in response. After
  // MasterTransmit we are "stalled" and wait for call to MasterReceive which then
  // sends RE-STAR etc. on the bus

 // jd: not used: uint8_t               con_aa;             // I2C slave CON flag
  int32_t               cnt;                // Master transfer count
  uint8_t              *data;               // Master data to transfer
  uint32_t              num;                // Number of bytes to transfer
  uint8_t              *sdata;              // Slave data to transfer
  uint32_t              snum;               // Number of bytes to transfer
} I2C_CTRL;

/* I2C Resource Configuration */
typedef struct {
  I2C_Type        	   *reg;                // I2C register interface
  IRQn_Type             i2c_ev_irq;         // I2C Event IRQ Number
  // jd: volatile uint32_t    *base_clk_reg;       // Base clock register
  volatile uint32_t    *pclk_cfg_reg;       // Peripheral clock config register
  //jd: const volatile uint32_t *pclk_stat_reg;   // Peripheral clock status register
  uint32_t              rgu_val;            // Peripheral reset value
  I2C_CTRL             *ctrl;               // Run-Time control information
} const I2C_RESOURCES;

#endif /* __I2C_LPC18XX_H */
