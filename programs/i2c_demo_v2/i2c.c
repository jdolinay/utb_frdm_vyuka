/*
 * Ovladac pro I2C komunikaci
 * Vice info viz i2c.c
 */
#include <MKL25Z4.H>
#include "i2c.h"
int lock_detect=0;
int i2c_lock=0;

//init I2C1
void i2c_init(void)
{
	//clock i2c peripheral and port E
	SIM->SCGC4 |= SIM_SCGC4_I2C1_MASK;
	SIM->SCGC5 |= (SIM_SCGC5_PORTE_MASK);
	
	//set pins to I2C function
	PORTE->PCR[1] |= PORT_PCR_MUX(6);	// SCL pin
	PORTE->PCR[0] |= PORT_PCR_MUX(6);	// SDA pin
		
	//set to 100k baud
	//baud = bus freq/(scl_div+mul)
 	//~400k = 24M/(64); icr=0x12 sets scl_div to 64

 	I2C1->F = (I2C_F_ICR(0x11) | I2C_F_MULT(2));
	// jd: pozor vyse ma byt krat a ne plus - spravne dle datasheet:
 	// baud = bus freq/(scl_div x mul)
 	// MULT(0) > mul=1,
 	// SCL divider value se najde v tabulce, pro
 	// ICR=0x10 je SCL divider 48
 	// ICR=0x12 > SCL divider 64
 	// max ICR=0x1f > 240
 	// Vstup pro I2C je "bus clock".
 	// Pro CLOCK_SETUP 0 je bus clock 20.97152 MHz
 	// Pro CLOCK_SETUP 1 je bus clock 24 MHz
 	// baud = 20 971 520 / (48) = 1 310 720
 	// Pro 100 kHz a CLOCK_SETUP = 0 je treba delit bus/210.
 	// pouzit mult=4 a ICR=11 > scl_div=56 tj.
 	// baud = 20 971 520 / (56*4=224) = 93,6 kHz

	//enable i2c and set to master mode
	I2C1->C1 |= (I2C_C1_IICEN_MASK);
	
	// Select high drive mode
	I2C1->C2 |= (I2C_C2_HDRS_MASK);
}


void i2c_busy(void){
	// Start Signal
	lock_detect=0;
	I2C1->C1 &= ~I2C_C1_IICEN_MASK;
	I2C_TRAN;
	I2C_M_START;
	I2C1->C1 |=  I2C_C1_IICEN_MASK;
	// Write to clear line
	I2C1->C1 |= I2C_C1_MST_MASK; /* set MASTER mode */
	I2C1->C1 |= I2C_C1_TX_MASK; /* Set transmit (TX) mode */
	I2C1->D = 0xFF;
	while ((I2C1->S & I2C_S_IICIF_MASK) == 0U) {
	} /* wait interrupt */  
	I2C1->S |= I2C_S_IICIF_MASK; /* clear interrupt bit */
		
		
							/* Clear arbitration error flag*/  
	I2C1->S |= I2C_S_ARBL_MASK;
		
		
							/* Send start */  
	I2C1->C1 &= ~I2C_C1_IICEN_MASK;
	I2C1->C1 |= I2C_C1_TX_MASK; /* Set transmit (TX) mode */
	I2C1->C1 |= I2C_C1_MST_MASK; /* START signal generated */
		
	I2C1->C1 |= I2C_C1_IICEN_MASK;
							/*Wait until start is send*/  

							/* Send stop */  
	I2C1->C1 &= ~I2C_C1_IICEN_MASK;
	I2C1->C1 |= I2C_C1_MST_MASK;
	I2C1->C1 &= ~I2C_C1_MST_MASK; /* set SLAVE mode */
	I2C1->C1 &= ~I2C_C1_TX_MASK; /* Set Rx */
	I2C1->C1 |= I2C_C1_IICEN_MASK;
		
	
								/* wait */  
							/* Clear arbitration error & interrupt flag*/  
	I2C1->S |= I2C_S_IICIF_MASK;
	I2C1->S |= I2C_S_ARBL_MASK;
	lock_detect=0;
	i2c_lock=1;
}

#pragma no_inline 
void i2c_wait(void) {
	lock_detect = 0;
	while(((I2C1->S & I2C_S_IICIF_MASK)==0) & (lock_detect < 200)) {
		lock_detect++;
	} 
	if (lock_detect >= 200)
		i2c_busy();
	I2C1->S |= I2C_S_IICIF_MASK;
}

//send start sequence
void i2c_start()
{
	I2C_TRAN;							/*set to transmit mode */
	I2C_M_START;					/*send start	*/
}

//send device and register addresses
#pragma no_inline 
void i2c_read_setup(uint8_t dev, uint8_t address)
{
	I2C1->D = dev;			  /*send dev address	*/
	I2C_WAIT							/*wait for completion */
	
	I2C1->D =  address;		/*send read address	*/
	I2C_WAIT							/*wait for completion */
		
	I2C_M_RSTART;				   /*repeated start */
	I2C1->D = (dev|0x1);	 /*send dev address (read)	*/
	I2C_WAIT							 /*wait for completion */
	
	I2C_REC;						   /*set to receive mode */

}

//read a byte and ack/nack as appropriate
// #pragma no_inline 
uint8_t i2c_repeated_read(uint8_t isLastRead)
{
	uint8_t data;
	
	lock_detect = 0;
	
	if(isLastRead)	{
		NACK;								/*set NACK after read	*/
	} else	{
		ACK;								/*ACK after read	*/
	}
	
	data = I2C1->D;				/*dummy read	*/
	I2C_WAIT					/*wait for completion */
	
	if(isLastRead)	{
		I2C_M_STOP;					/*send stop	*/
	}
	data = I2C1->D;				/*read data	*/

	return  data;					
}



//////////funcs for reading and writing a single byte
//using 7bit addressing reads a byte from dev:address
// #pragma no_inline 
uint8_t i2c_read_byte(uint8_t dev, uint8_t address)
{
	uint8_t data;
	
	I2C_TRAN;							/*set to transmit mode */
	I2C_M_START;					/*send start	*/
	I2C1->D = dev;			  /*send dev address	*/
	I2C_WAIT							/*wait for completion */
	
	I2C1->D =  address;		/*send read address	*/
	I2C_WAIT							/*wait for completion */
		
	I2C_M_RSTART;				   /*repeated start */
	I2C1->D = (dev|0x1);	 /*send dev address (read)	*/
	I2C_WAIT							 /*wait for completion */
	
	I2C_REC;						   /*set to recieve mode */
	NACK;									 /*set NACK after read	*/
	
	data = I2C1->D;					/*dummy read	*/
	I2C_WAIT								/*wait for completion */
	
	I2C_M_STOP;							/*send stop	*/
	data = I2C1->D;					/*read data	*/

	return data;
}



//using 7bit addressing writes a byte data to dev:address
#pragma no_inline 
void i2c_write_byte(uint8_t dev, uint8_t address, uint8_t data)
{
	
	I2C_TRAN;							/*set to transmit mode */
	I2C_M_START;					/*send start	*/
	I2C1->D = dev;			  /*send dev address	*/
	I2C_WAIT						  /*wait for ack */
	
	I2C1->D =  address;		/*send write address	*/
	I2C_WAIT
		
	I2C1->D = data;				/*send data	*/
	I2C_WAIT
	I2C_M_STOP;
	
}

// jd: write address with no data
// - for humidity sensor Measurement request command
void i2c_write_nobyte(uint8_t dev)
{

	I2C_TRAN;					/*set to transmit mode */
	I2C_M_START;				/*send start	*/
	I2C1->D = dev;			  /*send dev address	*/
	I2C_WAIT						  /*wait for ack */

	I2C_M_STOP;
}

// jd: write one byte - nepotrebne
void i2c_write_reg(uint8_t dev, uint8_t address)
{

	I2C_TRAN;							/*set to transmit mode */
	I2C_M_START;					/*send start	*/
	I2C1->D = dev;			  /*send dev address	*/
	I2C_WAIT						  /*wait for ack */

	I2C1->D =  address;		/*send write address	*/
	I2C_WAIT

	I2C_M_STOP;

}

// jd: write from humidity sensor
void i2c_read_bytes(uint8_t dev, uint8_t* buff, uint8_t size)
{
	uint8_t data;
	uint8_t index = 0;

	I2C_TRAN;						/*set to transmit mode */
	I2C_M_START;					/*send start	*/
	I2C1->D = (dev|0x1);	 		/*send dev address (read)	*/
	I2C_WAIT						/*wait for completion */

	I2C_REC;						   /*set to recieve mode */

	// verze primo cteni, nefunkcni
#if 0
	//data = I2C1->D;					/*dummy read	*/
	//I2C_WAIT						/*wait for completion */
	while ( size > 1 ) {
		lock_detect = 0;

		//buff[index] = I2C1->D;
		if ( size > 1 )
			ACK;
		else {
			NACK;	// send NACK for last byte
		}

		buff[index] = I2C1->D;
		I2C_WAIT
		//data = I2C1->D;					/*dummy read	*/
		//I2C_WAIT						/*wait for completion */
		index++;
		size--;
	}
	//index++;

	I2C_M_STOP;							/*send stop	*/
	buff[index] = I2C1->D;
#endif

	// VERZE s repeated_read
	for( index=0; index<3; index++)	{
		buff[index] = i2c_repeated_read(0);
	}
	// Read last byte ending repeated mode
	buff[index] = i2c_repeated_read(1);

}
