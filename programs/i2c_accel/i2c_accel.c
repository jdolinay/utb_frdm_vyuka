/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Komunikace pres I2C
 * Program ukazuje cteni dat z akcelerometru na desce FRDM-KL25Z
 * Namerene hodnoty zobrazuje na LCD displeji.
 * Zobrazuje raw hodnoty 8-bit.
 *
 *
 * Postup vytvoreni projektu s ovladacem I2C
 * 1) Pridat do projektu (Copy file) soubor RTE_Devices.h z CMSIS_Driver/Config.
 * Zkopirova (Copy file) do projektu a ne linkovat (Link to file),
 * aby mohl mit kazdy projekt svou konfiguraci ovladacu.
 *
 * 2) vlozit do zdrojoveho kodu #include "RTE_Device.h"
 *
 * 3) Pridat do projektu zdrojove kody ovladace I2C z CMSIS_Driver:
 * Propojit (Link to files), nevytvaret kopii v projektu.
 * I2C: I2C_MKL25Z4.c
 *
 * 4) Pridat slozku KSDK do projektu, pretazenim z Pruzkumnika na projekt
 * a volbou "Link to files and folders". Vznikne tak slozka "KSDK" v projektu.
 *
 * 4) Pridat cesty k nasledujicim umistenim do nastaveni C Compiler Includes:
 *  CMSIS_Driver
 *  KSDK/hal
 *  KSDK/mkl25z4
 *  Muzeme pridat absolutni cesty. Pro cesty v KSDK muzeme take pridat odkazy
 *  pres tlacitko Workspace.
 *  Priklad konkretnich cest v seznamu Includes:
 *  "../../../CMSIS_Driver"
 *  "${workspace_loc:/${ProjName}/KSDK/hal}"
 *  "${workspace_loc:/${ProjName}/KSDK/mkl25z4}"
 *
 *  Pro vypis na displej kitu je treba vlozit take ovladac LCD.
 *
 *  Info o akcelerometru:
 *  MMA8451Q
 *  SCL pin - PTE24
 *  SDA pin - PTE25
 *  Adresa je 0x1D (SA0 pulled high)
 *
 */

#include "MKL25Z4.h"
#include "RTE_Device.h"
#include <stdio.h>
#include "drv_lcd.h"		// ovladac displeje
#include "drv_systick.h"	// pro delay_ms

//
// Kod pro praci s akcelerometem podle Erich Styger
//
// Prototypy
uint8_t MMA8451_GetRaw8XYZ(uint8_t xyz[3]);
uint8_t MMA8451_Init(void);

/* External 3-axis accelerometer control register addresses */
#define MMA8451_CTRL_REG_1 0x2A
/* MMA8451 3-axis accelerometer control register bit masks */
#define MMA8451_ACTIVE_BIT_MASK 0x01
#define MMA8451_F_READ_BIT_MASK 0x02

/* External 3-axis accelerometer data register addresses */
#define MMA8451_OUT_X_MSB 0x01
#define MMA8451_OUT_X_LSB 0x02
#define MMA8451_OUT_Y_MSB 0x03
#define MMA8451_OUT_Y_LSB 0x04
#define MMA8451_OUT_Z_MSB 0x05
#define MMA8451_OUT_Z_LSB 0x06

// Adresa obvodu akcelerometru na I2C sbernici
#define MMA8451_I2C_ADDR   (0x1D) /* SA0=1 */
//#define MMA8451_I2C_ADDR   (0x1C) /* SA0=0 */


int main(void)
{
	ARM_I2C_STATUS status;
	char buffer[16];
	uint8_t xyz[3];


    // Inicializace a konfigurace ovladace I2C
	Driver_I2C0.Initialize(0);
	Driver_I2C0.PowerControl(ARM_POWER_FULL);
	Driver_I2C0.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);

	// Inicializace ovladace displeje
	LCD_initialize();

	// Inicializace ovladace pro cekani
	SYSTICK_initialize();

	// Inicializace akcelerometru
	MMA8451_Init();

	// Mereni
	while(1) {

		MMA8451_GetRaw8XYZ(xyz);

		LCD_clear();
		sprintf(buffer, "x: %d ", xyz[0]);
		LCD_puts(buffer);
		sprintf(buffer, "y: %d ", xyz[1]);
		LCD_set_cursor(2,1);
		LCD_puts(buffer);
		sprintf(buffer, "z: %d ", xyz[2]);
		LCD_set_cursor(3,1);
		LCD_puts(buffer);

		SYSTICK_delay_ms(100);
	}


    /* Never leave main */
    return 0;
}

/* Inicializuje akcelerometr.
 * Predpoklada, ze I2C driver je uz inicializovan
 * */
uint8_t MMA8451_Init(void)
{
	ARM_I2C_STATUS status;
	//static const uint8_t addr = MMA8451_CTRL_REG_1;
	//static const uint8_t data = MMA8451_F_READ_BIT_MASK|MMA8451_ACTIVE_BIT_MASK;
	const uint8_t cmd_accel[] = { MMA8451_CTRL_REG_1, (MMA8451_F_READ_BIT_MASK|MMA8451_ACTIVE_BIT_MASK) };
  /* F_READ: Fast read mode, data format limited to single byte (auto increment counter will skip LSB)
   * ACTIVE: Full scale selection
   */
  //return GI2C1_WriteAddress(MMA8451_I2C_ADDR, (uint8_t*)&addr, sizeof(addr), (uint8_t*)&data, sizeof(data));

	Driver_I2C0.MasterTransmit(MMA8451_I2C_ADDR, cmd_accel, 2, false);
	// Cekame na odeslani dat
	status = Driver_I2C0.GetStatus();
	while (status.busy) {
		status = Driver_I2C0.GetStatus();
	}

}

uint8_t MMA8451_GetRaw8XYZ(uint8_t xyz[3])
{
	ARM_I2C_STATUS status;
	static const uint8_t addr = MMA8451_OUT_X_MSB;

	// Zacatek komunikace, s repeated start
	Driver_I2C0.MasterTransmit(MMA8451_I2C_ADDR, &addr, 1, true);
	// Cekame na odeslani dat
	status = Driver_I2C0.GetStatus();
	while (status.busy)
		status = Driver_I2C0.GetStatus();

	// Cteni dat
	Driver_I2C0.MasterReceive(MMA8451_I2C_ADDR, &xyz[0], 3, false);
  	// Cekame na prijem dat
  	status = Driver_I2C0.GetStatus();
  	while (status.busy) {
  		status = Driver_I2C0.GetStatus();
  	}

  	return 0;

  //return GI2C1_ReadAddress(MMA8451_I2C_ADDR, (uint8_t*)&addr, sizeof(addr), &xyz[0], sizeof(xyz));
}











////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
