/*
 Ukazka prace s modulem FM radioprijimac

 Vyuziva ovladac I2C dle CMSIS

 * Postup vytvoreni projektu s ovladacem I2C:
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
#include "drv_systick.h"
#include <stdio.h>


static int i = 0;
uint32_t freq;
char buf[32];

// Adresa obvodu fm tuneru TEA 5767 na sbernici I2C1
// Frekvence max. 400 kHz coz odpovida HIGH_SPEED dle I2C CMSIS
#define	I2C_ADR_FM_TUNER		(0x60)	// Adresa je 0x60
// Kod pro HC08 pouziva 0xC0 protoze adresa je v odesilanem bajtu posunuta vlevo,
// vyuziva se jen 7 bitu pro adresu, 8. bit je RW.


uint32_t read_freq(void);
void write_freq(uint32_t freq);

void i2c_event(uint32_t event) { }

int main(void)
{
	uint32_t freq;

	SYSTICK_initialize();

	Driver_I2C1.Initialize(i2c_event);
	Driver_I2C1.PowerControl(ARM_POWER_FULL);
	Driver_I2C1.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);

    LCD_initialize();
    LCD_clear();
    LCD_puts("FM Prijimac test");



    freq = 917;		// 91.7 radio zlin
    write_freq(freq);

    SYSTICK_delay_ms(2000);

    for (;;) {

    	freq = read_freq();
    	sprintf(buf, "%d", freq);
    	LCD_set_cursor(2,1);
    	LCD_puts(buf);
    	SYSTICK_delay_ms(2000);

        i++;
    }
    /* Never leave main */
    return 0;
}

// Cte frekvenci nastavenou v obvodu FM tuneru
// Vraci: frekvence * 10 tj. napr. pro 98 MHz vrati 980
uint32_t read_freq(void)
{
	ARM_I2C_STATUS status;
	uint8_t data[6];

	// Vzdy cteme 5 bajtu
	Driver_I2C1.MasterReceive(I2C_ADR_FM_TUNER, data, 5, false);
	status = Driver_I2C1.GetStatus();
	while (status.busy)
		status = Driver_I2C1.GetStatus();

	freq=(((((data[0]&0x3F)<<8)+data[1])+1)*32768/4-225000)/100000;
	return freq;

	/* mbed:
	frequency = ((buf_temp[0]&0x3f)<<8) | buf_temp[1];
    return (((((float)frequency*32768)/4)-225000)/1000000);

	/* Arduino:
	freq_available=(((buffer[0]&0x3F)<<8)+buffer[1])*32768/4-225000;
		 lcd.print("FM ");
		 lcd.print((freq_available/1000000));
	freq=(((((data[0]&0x3F)<<8)+data[1])+1)*32768/4-225000)/100000;
	return freq;
	*/

}


// Zapise do obvodu FM tuneru novou frekvenci, tj. preladi na danou frekvenci.
// Vstup: frekvence v MHz * 10 tj. pro 95 MHz se funkci preda freq = 950
void write_freq(uint32_t freq)
{
	ARM_I2C_STATUS status;
	uint8_t buffer[6];
	uint16_t freq14bit;
	uint8_t freqH, freqL;

	if ((freq >= 700) && (freq <= 1080)) {
		//rozlozeni frekvence na dva bajty tzv PLL word (14 bitu)
		// vzorec odpovida HIGH side injection
		freq14bit = (4 * (freq * 100000 + 225000) / 32768) + 1;
		freqH = (freq14bit >> 8);
		freqL = (freq14bit & 0xFF);
		//buffer[0] = 0x00;
		buffer[0] = freqH;
		buffer[1] = freqL;
		// Dalsi hodnoty jsou v puvodnim ovladaci uz v promenne buffer,
		// protoze jsou nastaveny v init
		buffer[2]=  0b10110000;		// arduino = hc = 0b10110000;
		// Dle datasheet pro Byte 3: nutno nastavit bit 4 = high side injection
		// tj. 0x10
		// Bit 7 = 1 > search up
		// Bit 6,7 = search stop level; 01 = LOW
		// Hodnota 0xB0 = 0b10110000 je ok :)

		// Hodnoty pro byte 4:
		// Bit 4 (XTAL) = frekvence krystalu spolu s bitem 7 v byte 5 = PLLREF
		// Hodnota PLLREF = 0 a XTAL = 1 odpovida 32,768 kHz
		// Arduino 0x10 nastavuje jen XTAL, HC verze zapina i noise cancel a high cut...
		buffer[3]=  0x10;		// arduino: 0x10; hc: 0b00010110;
		buffer[4] = 0;			// 0 je ok (PLLREF = 0)

		// Odesilame 5 bajtu a cekame na odeslani
		Driver_I2C1.MasterTransmit(I2C_ADR_FM_TUNER, buffer, 5, false);
		status = Driver_I2C1.GetStatus();
		while (status.busy) {
			status = Driver_I2C1.GetStatus();
		}

	}

	/* Arduino:
		 frequency=87.5; //starting frequency
		 frequencyB=4*(frequency*1000000+225000)/32768; //calculating PLL word
		 frequencyH=frequencyB>>8;
		 frequencyL=frequencyB&0XFF;
		 Wire.beginTransmission(0x60);   //writing TEA5767
		 Wire.write(frequencyH);
		 Wire.write(frequencyL);
		 Wire.write(0xB0);
		 Wire.write(0x10);
		 Wire.write(0x00);
		 Wire.endTransmission();
		 */
}





////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
