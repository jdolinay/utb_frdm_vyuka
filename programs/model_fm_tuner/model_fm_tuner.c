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

// adresa obvodu fm tuneru TEA 5767 na sbernici I2C1
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

    // init FM je v mbed nastaveni frekvence

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

// TODO: vraci frekvenci * 10 tj. 980 pro 98 MHz
uint32_t read_freq(void)
{
	uint8_t data[6];

	// funkce hc08 je zavadejici, cte se pole
	//IIC_read_byte(TEA5767_READ_FREQ, data);
	Driver_I2C1.MasterReceive(I2C_ADR_FM_TUNER, data, 5, false);
	status = Driver_I2C1.GetStatus();
	while (status.busy)
		status = Driver_I2C1.GetStatus();

	frequency=(((((data[0]&0x3F)<<8)+data[1])+1)*32768/4-225000)/100000;
	return frequency;

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
		// Bit 4 = frekvence krystalu, bit XTAL
		// spolu s bitem 7 v byte 5 = PLLREF
		// Hodnota PLLREF = 0 a XTAL = 1 odpovida 32,768 kHz
		// Arduino 0x10 nastavuje jen XTAL, HC verze zapina i noise cancel a high cut...
		buffer[3]=  0x10;		// arduino: 0x10; hc: 0b00010110;
		buffer[4] = 0;			// 0 je ok (PLLREF = 0)
		Driver_I2C1.MasterTransmit(0xC0, buffer, 5, false);
		status = Driver_I2C1.GetStatus();
		while (status.busy) {
			status = Driver_I2C1.GetStatus();
		}
		//IIC_write_block(0xC0, &buffer[0]);
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


#if 0
// Obdoba funkce z ovladace pro HC08
// addr - adresa v obvodu ze ktere cist (toto NENI svale address)
void IIC_read_byte(uint8_t addr, uint8_t *data)
{
	ARM_I2C_STATUS status;

	// TODO: posilat 0 nebo 1 byte nebo vubec nevolat master transmit?
	// Nemelo by byt MAsterTransmit protoze to vzdy posila R/W bit = 0 tj. "write"
	// ale tuner pro cteni dat ocekava RW=1= "read"
	/*
	uint8_t dataSend[2] = { 0, 0 };	// data to send
	dataSend[0] = addr;
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

	/* Arduino:
	 Wire.requestFrom(0x60,5); //reading TEA5767
	 if (Wire.available())
	 {
	 for (int i=0; i<5; i++) {
	 buffer[i]= Wire.read();
	 }

	 freq_available=(((buffer[0]&0x3F)<<8)+buffer[1])*32768/4-225000;
	 lcd.print("FM ");
	 lcd.print((freq_available/1000000));
	 frequencyH=((buffer[0]&0x3F));
	 frequencyL=buffer[1];
	 if (search_mode) {
	 if(buffer[0]&0x80) search_mode=0;
	 }

	 if (search_mode==1) lcd.print(" SCAN");
	 else {
	 lcd.print("       ");
	 }
	 lcd.setCursor(0, 1);
	 lcd.print("Level: ");
	 lcd.print((buffer[3]>>4));
	 lcd.print("/16 ");
	 if (buffer[2]&0x80) lcd.print("STEREO   ");
	 else lcd.print("MONO   ");
	 }

	 */
	/* hc08:
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
#endif


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
