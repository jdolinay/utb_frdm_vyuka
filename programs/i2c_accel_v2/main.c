/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Komunikace pres I2C
 * Program ukazuje cteni dat z akcelerometru na desce FRDM-KL25Z
 * Namerene hodnoty zobrazuje na LCD displeji.
 * Zobrazuje raw hodnoty 8-bit jako cisla se znamenkem.
 * Rozsah akcelerometru je +/- 2 g, takže rozsah hodnot je +/- cca 127
 * Na displeji jsou cisla v rozsahu +/-65 coz je 1g.
 *
 * Pouziva se modul I2C0, pro teplomer a vlhkomer je modul I2C1
 *
 * Projekt i2c_accel_v2 je verze s jednoduchym ovladacem I2C.
 * Pro vypis na displej kitu je treba vlozit take ovladac LCD.
 * Pro funkci delay_ms je vlozen ovladac systick.
 *
 * Prehled ovladacu (zavislosti)
 * - drv_lcd - Ovladac LCD displeje, vlozen jako link z ../drivers/
 * - drv_systick - Ovladac pro casove funkce, vlozen jako link z ../drivers/
 * - i2c.h a i2c.c - Ovladac pro I2C komunikaci, vlozen primo v projektu, v ../programs
 *
 *  Info o akcelerometru:
 *  MMA8451Q
 *  SCL pin - PTE24
 *  SDA pin - PTE25
 *  Adresa je 0x1D (SA0 pulled high)
 *
 */

#include "MKL25Z4.h"
#include <stdio.h>
#include <math.h>
#include "i2c.h"
#include "drv_lcd.h"		// ovladac displeje
#include "drv_systick.h"	// pro delay_ms

// Definice pro akcelerometr
#define MMA_ADDR 0x3A

#define REG_XHI 0x01
#define REG_XLO 0x02
#define REG_YHI 0x03
#define REG_YLO 0x04
#define REG_ZHI	0x05
#define REG_ZLO 0x06
#define REG_WHOAMI 0x0D
#define REG_CTRL1  0x2A
#define REG_CTRL4  0x2D

#define WHOAMI 0x1A

/* MMA8451 3-axis accelerometer control register bit masks */
#define MMA8451_ACTIVE_BIT_MASK 0x01
#define MMA8451_F_READ_BIT_MASK 0x02

// Prototypy
uint8_t MMA8451_GetRaw8XYZ(int8_t xyz[3]);
uint8_t MMA8451_Init(void);

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
	char buffer[32];
	int8_t xyz[3];


	// Inicializace ovladace displeje
	LCD_initialize();

	// Inicializace ovladace pro cekani
	SYSTICK_initialize();

	// Inicializace akcelerometru
	i2c_init();
	if ( !MMA8451_Init() ) {
		while (1)
			;	// Error initializing accellerometer
	}

	// Mereni
	while (1) {

		MMA8451_GetRaw8XYZ(xyz);

		LCD_clear();
		sprintf(buffer, "x= %d ", xyz[0]);
		LCD_puts(buffer);
		sprintf(buffer, "y= %d ", xyz[1]);
		LCD_set_cursor(2, 1);
		LCD_puts(buffer);
		sprintf(buffer, "z= %d ", xyz[2]);
		LCD_set_cursor(3, 1);
		LCD_puts(buffer);

		SYSTICK_delay_ms(200);
	}

}

/* Inicializuje akcelerometr.
 * Predpoklada, ze I2C driver je uz inicializovan
 * */
uint8_t MMA8451_Init(void)
{
	//set active mode, 8 bit samples (by F_READ mask) and 800 Hz ODR
	i2c_write_byte(MMA_ADDR, REG_CTRL1, (MMA8451_F_READ_BIT_MASK | MMA8451_ACTIVE_BIT_MASK) );
	return 1;
}

uint8_t MMA8451_GetRaw8XYZ(int8_t xyz[3])
{
	int i;

	i2c_start();
	i2c_read_setup(MMA_ADDR , REG_XHI);

	// Read bytes in repeated mode
	for( i=0; i<2; i++)	{
		xyz[i] = i2c_repeated_read(0);
	}
	// Read last byte ending repeated mode
	xyz[i] = i2c_repeated_read(1);

	return 1;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
