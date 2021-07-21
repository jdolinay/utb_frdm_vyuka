/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Komunikace pres I2C
 * Program ukazuje cteni dat z teplomeru a vlhkometu pres I2C.
 * Pouziva se modul I2C1, pro akcelerometr I2C0
 * Namerene hodnoty zobrazuje na LCD displeji.
 *
 * Projekt i2c_demo_v2 je verze s jednoduchym ovladacem I2C.
 * Pro vypis na displej kitu je treba vlozit take ovladac LCD.
 * Pro funkci delay_ms je vlozen ovladac systick.
 *
 * Prehled ovladacu (zavislosti)
 * - drv_lcd - Ovladac LCD displeje, vlozen jako link z ../drivers/
 * - drv_systick - Ovladac pro casove funkce, vlozen jako link z ../drivers/
 * - i2c.h a i2c.c - Ovladac pro I2C komunikaci, vlozen primo v projektu, v ../programs
 * Do nastaveni projektu
 * C/C++ Build - Settings - Cross ARM C Compiler - Includes
 *  je treba pridat odkaz na slozky ovladacu lcd a systick:
 *  "../../../drivers/lcd"
 *  "../../../drivers/systick"
 * (V teto ukazce relativni cesta vztazena k umisteni projektu v utb_frdm_vyuka)
 *
 *
 * LOG:
 * 21.7.21 - problem mohl byt i v clock ktery byl nastaven na cca 450 kHz misto 100 kHz. Dle datasheet
 *  je frekvence max 400 kHz pro oba senzory, pro vlhkomer navic i min frekvence 100 kHz.
 * - zkusit humidity s repeated read verzi, pripadne odkomentovanou puvodni verzi, ta jeste nezkousena
 *  a mohl byt problem v lock detect, ktery resetuje v repeated read.
 *   pripadne rozsvitit led pri lock.
 *  - teplota - podle datasheet se zda ze je spravne.
 *  - jaka je frekvence iic nastavena v kodu?
 *
 */

#include "MKL25Z4.h"
#include <stdio.h>
#include <math.h>
#include "i2c.h"			// nas ovladac i2c ve slozce sources
#include "drv_lcd.h"		// ovladac displeje
#include "drv_systick.h"	// ovladac systick pro delay_ms

// Adresy obvodu na I2C sbernici
#define I2C_ADR_RTC (0b1010000)
#define I2C_ADR_TEMP_SENSOR (0b1001000)
#define I2C_ADR_HMDT_SENSOR (0b0100111)

// Registry teplotniho snimace LM75
#define LM75_REG_TEMP  (0)


// Prototypy
void delay(void);
uint32_t MeasureHumidity(void);		// zmereni vlhkosti
uint32_t MeasureTemperature(void);

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void)
{

	uint32_t temperature, humidity;
	char buffer[16];

	// Inicializace ovladace displeje
	LCD_initialize();

	// Inicializace ovladace pro cekani
	SYSTICK_initialize();

	// Inicializace ovladace I2C
	i2c_init();

	// Mereni
	while(1) {
		humidity = MeasureHumidity();
		//temperature = MeasureTemperature();
		SYSTICK_delay_ms(1000);
		LCD_clear();
		sprintf(buffer, "H: %d %%", humidity);
		LCD_puts(buffer);
		//sprintf(buffer, "T: %d.%d C", temperature/10, temperature%10);
		//LCD_set_cursor(2,1);
		//LCD_puts(buffer);
	}

	return 0;
}

/*
 * Zmeri vlhkost.
 * Vraci vlhkost v procentech.
 */
uint32_t MeasureHumidity(void)
{
	uint8_t hih_measure_status;
	uint8_t dataSend[2] = { 0 };	// data to send
	uint8_t sendSize = 0;		// number of bytes to send
	uint8_t dataReceive[4];
	uint32_t humidity;


	//i2c_start();
	i2c_write_nobyte(I2C_ADR_HMDT_SENSOR);
	// Senzoru trva 37 ms nez dokonci prevod.
	// Muzeme take zjistovat status bity v prijatych datech, ale
	// to je komplikovanejsi.
	SYSTICK_delay_ms(40);

	i2c_read_bytes(I2C_ADR_HMDT_SENSOR, dataReceive, 4 );

/*
		// Zahajime mereni poslanim slave adresy a R/W bitu 0 (prikaz write bez dat)
		Driver_I2C1.MasterTransmit(I2C_ADR_HMDT_SENSOR, dataSend, 0, false);
		status = Driver_I2C1.GetStatus();
		while (status.busy) {
			status = Driver_I2C1.GetStatus();
		}
		// Senzoru trva 37 ms nez dokonci prevod.
		// Muzeme take zjistovat status bity v prijatych datech, ale
		// to je komplikovanejsi.
		SYSTICK_delay_ms(40);


		Driver_I2C1.MasterReceive(I2C_ADR_HMDT_SENSOR, dataReceive, 4, false);
		// Cekame na prijem dat
		status = Driver_I2C1.GetStatus();
		while (status.busy) {
			status = Driver_I2C1.GetStatus();
		}
*/
	humidity = 256 * (dataReceive[0] & 0b00111111) + dataReceive[1];

	// Vypocet vlhkosti
	humidity = ((int) humidity * 100) / 16383;
	return humidity;
}

/*
 Cteni teploty z LM75A
 Vraci teplotu ve stupnich x 10, napr. 205 znamena 20.5 C

 Poznamky:
 Obvod ma "pointer" ktery urcuje, co chceme cist nebo zapisovat.
 adresa 0 = teplota (2 B)
 adresa 1 = config (1B)
 a dalsi nepodstatne adresy :)
 V config registru je defaultne hodnota 0 a ta vyhovuje pro mereni teploty.
 Presto se zda nutne do nej zapsat nejprve tuto 0.
*/
uint32_t MeasureTemperature(void)
{
	// Inicializacni prikaz pro teplotni senzor LM75
	const uint8_t cmd_lm75_init[] = {0b00000001, 0b00000000};		// zapis defaultnich hodnot do config registru

	uint8_t dataSend[2] = { 0 };	// data to send
	uint8_t sendSize = 0;		// number of bytes to send
	uint8_t dataReceive[2];
	uint32_t temperature;

	// Inicializace teplotniho snimace
	//i2c_start();
	i2c_write_byte(I2C_ADR_TEMP_SENSOR, cmd_lm75_init[0], cmd_lm75_init[1]);
	//Driver_I2C1.MasterTransmit(I2C_ADR_TEMP_SENSOR, cmd_lm75_init, 2, false);


#if 1
	// Varianta a) - cteni teploty s poslanim pointer hodnoty
	// tj. posilam prikaz "precti registr s teplotou":
	// pointer = 0 (teplota;
	// cti(pointer)

	//i2c_write_reg(I2C_ADR_TEMP_SENSOR, LM75_REG_TEMP);
	//dataSend[0] = LM75_REG_TEMP;		// hodnota = 0 = pointer
	//sendSize = 1;
	//Driver_I2C1.MasterTransmit(I2C_ADR_TEMP_SENSOR, dataSend, sendSize, true);
	i2c_start();
	i2c_read_setup(I2C_ADR_TEMP_SENSOR, LM75_REG_TEMP);
	// Read bytes in repeated mode
	dataReceive[0] = i2c_repeated_read(0);
	// Read last byte ending repeated mode
	dataReceive[1] = i2c_repeated_read(1);

	//i2c_read_bytes(I2C_ADR_TEMP_SENSOR, dataReceive, 2 );
	//Driver_I2C1.MasterReceive(I2C_ADR_TEMP_SENSOR, dataReceive, 2, false);
	// Cekame na odeslani dat

	temperature = (256 * dataReceive[0] + dataReceive[1]) >> 5;
	temperature = (temperature * 1270) / 0x3f8;		// prepocet na st.C
	return temperature;
#endif


#if 0
	//
	// Varianta b) - cteni teploty with preset pointer - funguje taky
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
	Driver_I2C1.MasterReceive(I2C_ADR_TEMP_SENSOR, dataReceive, 2, false);
	status = Driver_I2C1.GetStatus();
	while (status.busy)
	status = Driver_I2C1.GetStatus();

	temperature = (256 * dataReceive[0] + dataReceive[1]) >> 5;
	temperature = (temperature * 1270) / 0x3f8;// prepocet na st.C
	return temperature;

#endif



}



////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
