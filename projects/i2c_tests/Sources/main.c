/*
 * Testovaci projekt pro I2C driver.
 * Vyuziva CMSIS driver z /CMSIS_Driver
 *
 * Piny:
 * Teplomer: (I2C1)
 * SDA: PTE0
 * SCL: PTE1
 */

#include "MKL25Z4.h"
#include "I2C_MKL25Z4.H"	// TODO: asi neinkuludovat primo toto ale pres nejaky RTE_components kde
// budou definovany extern objekty driveru a prislusne include podle nastaveni jake drivery chci.

extern ARM_DRIVER_I2C Driver_I2C1;

// adresy obvodu na I2C sbernici
#define I2C_ADR_RTC (0b1010000)
#define I2C_ADR_TEMP_SENSOR (0b1001000)
#define I2C_ADR_HMDT_SENSOR (0b0100111)

// Registry teplotniho snimace LM75
#define LM75_REG_TEMP  (0)
#define LM75_REG_CONF  (1)
#define LM75_REG_THYST (2)
#define LM75_REG_TOS   (3)

/*
 tepl snimac:
 // Inicializace LM75
  status = I2C1_Kit_SendBlock(cmd_lm75_init, 2, &bwr);		// zapis defaultni konfigurace

 // 2) Teplotni snimac LM75A
	  //------------------------------------
	  status = I2C1_Kit_SelectSlave(I2C_ADR_TEMP_SENSOR);
	  // Cteni Temp registru - hodnota Pointer = 0
	  status = I2C1_Kit_SendChar(LM75_REG_TEMP);
	  I2C1_Kit_RecvBlock(buffer,2,&brd);
	  value = (256*buffer[0]+buffer[1]) >> 5;
	  // prepocet na st.C
	  value = (value * 1270) / 0x3f8;

	  disp_set_cursor(4,9);
	  sprintf(buffer,"%02u.%u",value / 10, value % 10);
	  disp_text(buffer);

 * */

// Init prikaz pro RTC obvod
const uint8_t cmd_rtc_init[] = {0b00000000, 0b00000000};		// inicializace 32.768 kHz, citani casu spusteno
// Init prikaz pro teplotni senzor LM75
const uint8_t cmd_lm75_init[] = {0b00000001, 0b00000000};		// zapis defaultnich hodnot do config registru



static int i = 0;
// event from i2c interface
void i2c0_event(uint32_t event);
void delay(void);


int main(void)
{
	uint8_t dataSend[4] = { 0 };	// data to send
	uint8_t sendSize = 0;		// number of bytes to send
	uint8_t dataReceive[8];
	uint32_t temperature;

	// TODO: enable clock for ports used by I2C - should be in driver?
	// 1. Povolime hodinovy signal pro port E
	SIM->SCGC5 |= (SIM_SCGC5_PORTE_MASK );

    /* Write your code here */
	Driver_I2C1.Initialize(i2c0_event);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);


	// Inicializace teplotniho snimace
	// status = I2C1_Kit_SendBlock(cmd_lm75_init, 2, &bwr);
	Driver_I2C1.MasterTransmit(I2C_ADR_TEMP_SENSOR, cmd_lm75_init, 2, false);
	// TODO: wait for transmit to complete
	delay();

	while (1) {
		// Cteni teploty
		// Cteni Temp registru - hodnota Pointer = 0
		//status = I2C1_Kit_SendChar(LM75_REG_TEMP);
		dataSend[0] = LM75_REG_TEMP;
		sendSize = 1;
		Driver_I2C1.MasterTransmit(I2C_ADR_TEMP_SENSOR, dataSend, sendSize, false);
		// TODO: wait for transmit to complete
		delay();

		//I2C1_Kit_RecvBlock(buffer,2,&brd);
		Driver_I2C1.MasterReceive(I2C_ADR_TEMP_SENSOR, dataReceive, 2, false);
		temperature = (256 * dataReceive[0] + dataReceive[1]) >> 5;
		// prepocet na st.C
		temperature = (temperature * 1270) / 0x3f8;
	}


    /* This for loop should be replaced. By default this loop allows a single stepping. */
    for (;;) {
        i++;
    }
    /* Never leave main */
    return 0;
}

void i2c0_event(uint32_t event)
{
	;
}

void delay(void)
{
	uint32_t n = 10000L;
	while(n > 0)
		n--;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
