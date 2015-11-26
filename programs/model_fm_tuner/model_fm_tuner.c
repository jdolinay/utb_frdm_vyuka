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
#include "drv_systick.h"
#include <stdio.h>


static int i = 0;
uint32_t freq;
char buf[32];

// adresa obvodu fm tuneru na sbernici I2C1
// max. 400 kHz
#define	I2C_ADR_FM_TUNER		(0x60)	// nebo 0x60 viz datasheet: IC address: 110 0000b
// a za tim je jeste bit RW, je mozne ze muj I2C driver dela shitf sam, pak adresa je 0x60!

// asi adresa, ze ktere se cte frekvence
// takto to neni! v puvodnim je C0 a c1 protoze to je i write bit I2C!
#define	 TEA5767_READ_FREQ		(0xC1)

uint32_t read_freq(void);
void write_freq(uint32_t freq);
void i2c_event(uint32_t event)
{

}

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
    SYSTICK_delay_ms(500);

    for (;;) {
    	freq = read_freq();
    	sprintf(buf, "%d", freq);
    	LCD_set_cursor(2,1);
    	LCD_puts(buf);
    	SYSTICK_delay_ms(500);

        i++;
    }
    /* Never leave main */
    return 0;
}

// Obdoba funkce z ovladace pro HC08
// addr - adresa v obvodu ze ktere cist (toto NENI svale address)
void IIC_read_byte(uint8_t addr, uint8_t *data)
{
	uint8_t dataSend[2] = { 0, 0 };	// data to send
	ARM_I2C_STATUS status;

	dataSend[0] = addr;
	// TODO: posilat 0 nebo 1 byte nebo vubec nevolat master transmit?
	/*
	Driver_I2C1.MasterTransmit(I2C_ADR_FM_TUNER, dataSend, 0, true);
	// Cekame na odeslani dat
	status = Driver_I2C1.GetStatus();
	while (status.busy)
		status = Driver_I2C1.GetStatus();
	 */
	Driver_I2C1.MasterReceive(I2C_ADR_FM_TUNER, data, 5, false);
	status = Driver_I2C1.GetStatus();
	while (status.busy)
		status = Driver_I2C1.GetStatus();

	/*
	 unsigned int i;
	 int x;
	 byte dummy;
	 IIC1C_TXAK = 0;            // RX/TX = 1; MS/SL = 1; TXAK = 0;
	 IIC1C |= 0x30;             // And generate START condition;

    //-----Start of transmit first byte to IIC bus-----
    IIC1D = 0xC0;                            // Address the slave and set up for master transmit;
    while (!IIC1S_IICIF)__RESET_WATCHDOG();  // wait until IBIF;
    IIC1S_IICIF=1;                           // clear the interrupt event flag;
    while(IIC1S_RXAK)__RESET_WATCHDOG();     // check for RXAK;
    //-----Slave ACK occurred------------
    IIC1C_RSTA = 1;                          // set up for repeated start
    IIC1D = addr;                            // slave adress read
    while(!IIC1S_IICIF)__RESET_WATCHDOG();   // wait until IBIF
    IIC1S_IICIF=1;                           // clear the interrupt event flag;
    while(IIC1S_RXAK)__RESET_WATCHDOG();     // check for RXAK;
    IIC1C_TX = 0;                            // set up to receive;
    dummy = IIC1D;                           // dummy read
    for(x=0;x<4;x++){
        while (!IIC1S_IICIF)__RESET_WATCHDOG();  // wait until IBIF;
        IIC1S_IICIF=1;                           // clear the interrupt event flag;
        data[x]=IIC1D;
    }
    IIC1C_TXAK = 1;                          // acknowledge disable;
    while (!IIC1S_IICIF)__RESET_WATCHDOG();  // wait until IBIF;
    IIC1S_IICIF=1;                           // clear the interrupt event flag;
    IIC1C_MST = 0;                           // generate STOP condition;
    data[4]=IIC1D;
    for(i=0;i<1000;i++)__RESET_WATCHDOG(); */
}

// TODO: vraci frekvenci * 10 tj. 980 pro 98 MHz
uint32_t read_freq(void)
{
	uint8_t data[6];

	IIC_read_byte(TEA5767_READ_FREQ, data);

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
	ARM_I2C_STATUS status;
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
		// Dalsi hodnoty jsou v puvodnim ovladaci v buffer,
		// protoze jsou nastaveny v init
		buffer[2]=  0b10110000;
		buffer[3]=  0b00010110;
		buffer[4] = 0b00000000;
		Driver_I2C1.MasterTransmit(0xC0, buffer, 5, false);
		status = Driver_I2C1.GetStatus();
		while (status.busy) {
			status = Driver_I2C1.GetStatus();
		}
		//IIC_write_block(0xC0, &buffer[0]);
	}



}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
