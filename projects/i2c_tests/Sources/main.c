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
uint32_t MeasureHumidity(void);		// zmereni vlhkosti

uint32_t MeasureTemperature(void);

int main(void)
{
	uint8_t dataSend[4] = { 0 };	// data to send
	uint8_t sendSize = 0;		// number of bytes to send
	uint8_t dataReceive[8];
	uint32_t temperature;
	ARM_I2C_STATUS status;

	// TODO: enable clock for ports used by I2C - should be in driver?
	// 1. Povolime hodinovy signal pro port E
	SIM->SCGC5 |= (SIM_SCGC5_PORTE_MASK );

    /* Write your code here */
	Driver_I2C1.Initialize(i2c0_event);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);

	// POZOR: Funkce MasterTransmit nemusi resit smer R/W, ten je vzdy 0,
	// masterReceive posila s r/w nastaveno na R tj. do adresy prida 0
	/* Note:  r/w - read is 1, write is 0. */

	//MeasureHumidity();
	MeasureTemperature();

    /* This for loop should be replaced. By default this loop allows a single stepping. */
    for (;;) {
        i++;
    }
    /* Never leave main */
    return 0;
}

/*
 // 3) Snimac vlhkosti HIH6130
 //------------------------------------
 status = I2C1_Kit_SelectSlave(I2C_ADR_HMDT_SENSOR);
 if (hih_measure_status == 0)
 {
 status = I2C1_Kit_SendChar(0);					// measure request, mereni trva cca 36.65 ms
 }
 // Cteni vlhkosti - 2 bajty
 I2C1_Kit_RecvBlock(buffer,2,&brd);					// cteni vlhkosti
 hih_measure_status = buffer[0] >> 6;					// ulozeni stavu HIH
 value = 256*(buffer[0] & 0b00111111)+buffer[1];

 // Vypocet vlhkosti
 value = ((int)value * 100) / 16383;

 disp_set_cursor(4,17);
 sprintf(buffer,"%u%%",value);
 disp_text(buffer);
 */
uint32_t MeasureHumidity(void)
{
	uint8_t hih_measure_status;
	uint8_t dataSend[2] = { 0 };	// data to send
	uint8_t sendSize = 0;		// number of bytes to send
	uint8_t dataReceive[4];
	uint32_t humidity;
	ARM_I2C_STATUS status;


	while (1) {
		/* Zde je chyba v Dastyho kodu? nebo to PE jinak neumi?
		 * Measurement request neni poslani byte s hodnotou 0
		 * ale jen slave adresy s bitem RW = 0 tj. prikaz write ale bez dat.
		if (hih_measure_status == 0) {
			dataSend[0] = 0;
			sendSize = 1;
			Driver_I2C1.MasterTransmit(I2C_ADR_HMDT_SENSOR, dataSend, sendSize, false);
			status = Driver_I2C1.GetStatus();
			while (status.busy)
				status = Driver_I2C1.GetStatus();
		}*/
		Driver_I2C1.MasterTransmit(I2C_ADR_HMDT_SENSOR, dataSend, 0, false);
		status = Driver_I2C1.GetStatus();
		delay();	// dame senzoru cas na mereni

		Driver_I2C1.MasterReceive(I2C_ADR_HMDT_SENSOR, dataReceive, 4, false);
		status = Driver_I2C1.GetStatus();
		while (status.busy)
			status = Driver_I2C1.GetStatus();
		//hih_measure_status = dataReceive[0] >> 6;			// ulozeni stavu HIH
		humidity = 256 * (dataReceive[0] & 0b00111111) + dataReceive[1];

		// Vypocet vlhkosti
		humidity = ((int) humidity * 100) / 16383;
	}

}

// cteni teploty z LM75A
// Obvod ma "pointer" ktery urcuje, co chceme cist nebo zapisovat.
// adresa 0 = teplota (2 B)
// adresa 1 = config (1B)
// a dalsi nepodstatne adresy :)
// V config registru je defaultne hodnota 0 a ta vyhovuje pro mereni teploty. Presto se zda
// nutne do nej zapsat nejprve tuto 0; nebo je to chyba driveru?
uint32_t MeasureTemperature(void)
{
	uint8_t dataSend[2] = { 0 };	// data to send
	uint8_t sendSize = 0;		// number of bytes to send
	uint8_t dataReceive[2];
	uint32_t temperature;
	ARM_I2C_STATUS status;

	// Inicializace teplotniho snimace
	// status = I2C1_Kit_SendBlock(cmd_lm75_init, 2, &bwr);
	Driver_I2C1.MasterTransmit(I2C_ADR_TEMP_SENSOR, cmd_lm75_init, 2, false);
	status = Driver_I2C1.GetStatus();
	while ( status.busy )
		status = Driver_I2C1.GetStatus();

#if 1
	// Varianta a) - cteni teploty s poslanim pointer hodnoty
	// tj. posilam prikaz "precti registr s teplotou":
	// pointer = 0 (teplota;
	// cti(pointer)
	while (1) {
		dataSend[0] = LM75_REG_TEMP;		// hodnota = 0 = pointer
		sendSize = 1;
		Driver_I2C1.MasterTransmit(I2C_ADR_TEMP_SENSOR, dataSend, sendSize, true);
		status = Driver_I2C1.GetStatus();
		while (status.busy)
			status = Driver_I2C1.GetStatus();

		Driver_I2C1.MasterReceive(I2C_ADR_TEMP_SENSOR, dataReceive, 2, false);
		status = Driver_I2C1.GetStatus();
		while (status.busy)
			status = Driver_I2C1.GetStatus();

		temperature = (256 * dataReceive[0] + dataReceive[1]) >> 5;
		temperature = (temperature * 1270) / 0x3f8;		// prepocet na st.C
	}
#endif


#if 0
	//
	// Varianta b) - cteni teploty with preset pointer
	// tj. posilam prikaz precti posledni registr (a to je registr teploty, pokud
	// jsem na nej predtim nastavil pointer:
	// Stav:
	// nefunguje; arbitration lost po prvnim cteni...
	dataSend[0] = LM75_REG_TEMP;		// hodnota = 0 = pointer
	sendSize = 1;
	Driver_I2C1.MasterTransmit(I2C_ADR_TEMP_SENSOR, dataSend, sendSize, true);
	status = Driver_I2C1.GetStatus();
	while (status.busy)
		status = Driver_I2C1.GetStatus();
	while(1)
	{
		Driver_I2C1.MasterReceive(I2C_ADR_TEMP_SENSOR, dataReceive, 2, false);
		status = Driver_I2C1.GetStatus();
		while (status.busy)
			status = Driver_I2C1.GetStatus();

		temperature = (256 * dataReceive[0] + dataReceive[1]) >> 5;
		temperature = (temperature * 1270) / 0x3f8;		// prepocet na st.C
	}
#endif


}

void i2c0_event(uint32_t event)
{
	;
}

void delay(void)
{
	uint32_t n = 100000L;
	while(n > 0)
		n--;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
