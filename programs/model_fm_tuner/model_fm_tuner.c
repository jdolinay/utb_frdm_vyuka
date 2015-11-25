/*
 Ukazka prace s modulem FM radioprijimac

 Vyuziva ovladac I2C dle CMSIS
 Postup vytvoreni projektu s ovladacem I2C

 * Postup vytvoreni projektu s ovladacem I2C
 * 1) Pridat do projektu soubor RTE_Devices.h z CMSIS_Driver/Config.
 * Vhodne je zkopirovat (Copy file) do projektu a ne linkovat (Link to file),
 * aby mohl mit kazdy projekt svou konfiguraci ovladacu.
 *
 * 2) vlozit do zdrojoveho kodu #include "RTE_Device.h"
 *
 * 3) Pridat do projektu zdrojove kody ovladace (ovladacu).
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

 */

#include "MKL25Z4.h"
#include "RTE_Device.h"
#include "drv_lcd.h"


static int i = 0;
uint32_t freq;

uint32_t read_freq(void);
void i2c_event(uint32_t event)
{

}

int main(void)
{
	uint32_t freq;

	Driver_I2C1.Initialize(i2c_event);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);

    LCD_initialize();
    LCD_clear();
    LCD_puts("FM Prijimac test");
    LCD_set_cursor(2,1);

    freq = read_freq();


    for (;;) {
        i++;
    }
    /* Never leave main */
    return 0;
}

// TODO: vraci frekvenci * 10 tj. 980 pro 98 MHz
uint32_t read_freq(void)
{
	uint32_t freq = 0;
	uint8_t data[6];
	Driver_I2C1.MasterReceive(0xC0, data, 5, false);
	// todo: mozna nutno volat nejdriv mtransmit s repeated start a pak az receive

	freq=(((((data[0]&0x3F)<<8)+data[1])+1)*32768/4-225000)/100000;
	return freq;

	//IIC_read_byte(0xC1, &read[0]);
	//frequency=(((((read[0]&0x3F)<<8)+read[1])+1)*32768/4-225000)/100000;
	//return frequency;
}


// zvoli frekvenci
// frekvence je v MHz * 10 tj. 95 MHz se zvoli freq = 950
void write_freq(uint32_t freq)
{
	uint8_t buffer[6];
	uint16_t freq14bit;
	uint8_t freqH, freqL;

	if ((freq >= 700) && (freq <= 1080)) {
		//rozlozeni frekvence na dva bajty tzv PLL word (14 bitu)
		freq14bit = (4 * (freq * 100000 + 225000) / 32768) + 1;
		freqH = (freq14bit >> 8);
		freqL = (freq14bit & 0xFF);
		buffer[0] = 0x00;
		buffer[0] = freqH;
		buffer[1] = freqL;
		Driver_I2C1.MasterTransmit(0xC0, buffer, 5, false);
		//IIC_write_block(0xC0, &buffer[0]);
	}



}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
