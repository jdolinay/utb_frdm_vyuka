// Ovladac pro Info displej, puvodne pro HCS08
// Uprava pro KL25Z
// Podrobnejsi info viz InfoDisp.h

//#include <hidef.h> /* for EnableInterrupts macro */
//#include "derivative.h" /* include peripheral declarations */

// Piny:
// Pro HC08
// PTF4 - zapisovy signal 74HC595 = enable
// PTF5 - reset signal  74HC595 = latch
// PTD1 - 3 - vyber radku A,B,C
// Pro KL25Z
// Enable (zapisovy signal) = PTD6
// Latch (reset) = PTD7
// Radek A = PTC16
// Radek B = PTD5
// Radek C = PTD4


#include "MKL25Z4.h"
#include <stdio.h>
#include "RTE_Device.h"

//#define enable PTFD_PTFD4
//#define latch PTFD_PTFD5

#define		RADEKA_PIN	(16)
#define		RADEKB_PIN	(5)
#define		RADEKC_PIN	(4)
#define 	LATCH_PIN	(7)
#define		ENABLE_PIN	(6)
#define		LATCH_HIGH()	PTD->PSOR |= (1 << LATCH_PIN);
#define		LATCH_LOW()		PTD->PCOR |= (1 << LATCH_PIN);
#define		ENABLE_HIGH()	PTD->PSOR |= (1 << ENABLE_PIN);
#define		ENABLE_LOW()	PTD->PCOR |= (1 << ENABLE_PIN);
#define		RADEKA_HIGH()	PTC->PSOR |= (1 << RADEKA_PIN);
#define		RADEKA_LOW()	PTC->PCOR |= (1 << RADEKA_PIN);
#define		RADEKB_HIGH()	PTD->PSOR |= (1 << RADEKB_PIN);
#define		RADEKB_LOW()	PTD->PCOR |= (1 << RADEKB_PIN);
#define		RADEKC_HIGH()	PTD->PSOR |= (1 << RADEKC_PIN);
#define		RADEKC_LOW()	PTD->PCOR |= (1 << RADEKC_PIN);

// makro ktere na KL25Z neni ale je vsude v kodu pro HCS08 :)
#define	__RESET_WATCHDOG()	; ;

void dwait(void);
void timer2_ovf_int(void);
char SetBit(int radek, int sloupec, char zobraz); //parametry - radek, sloupec, a jestli chceme bit zobrazit
void ZobrazZnak(char znak, int radek, int sloupec,char invert);//parametry - radek, sloupec, a pak bude invert = 1 bude pismeno invertovane
void ZobrazText(char *text, int radek, int sloupec,char invert);//parametry - radek, sloupec, a pak bude invert = 1 bude text invertovany
void Clear(void);// smaze displej
void Fill(void);// zaplni displej
char radek=0;
char temp=0;

//void (*const obsluha)(void) @0xFFE2 = timer2_ovf_int;


const unsigned char Abeceda[]={
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, //mezernik
  0b00000000, 0b00000000, 0b01111101, 0b00000000, 0b00000000, //!
  0b00000000, 0b01110000, 0b00000000, 0b01110000, 0b00000000, //"
  0b00010100, 0b00111110, 0b00010100, 0b00111110, 0b00010100, //#
  0b00010000, 0b00101010, 0b01111111, 0b00101010, 0b00000100, //$
  0b01100010, 0b01100100, 0b00001000, 0b00010011, 0b00100011, //%
  0b00110110, 0b01001001, 0b01010101, 0b00100010, 0b00000101, //&
  0b00000000, 0b01010000, 0b01100000, 0b00000000, 0b00000000, //'  
  0b00011100, 0b00100010, 0b01000001, 0b01000001, 0b00000000, //( 
  0b00000000, 0b01000001, 0b01000001, 0b00100010, 0b00011100, //) 
  0b00101010, 0b00011100, 0b11111111, 0b00011100, 0b00101010, //* 
  0b00001000, 0b00001000, 0b00111110, 0b00001000, 0b00001000, //+     
  0b00000000, 0b00000101, 0b00000110, 0b00000000, 0b00000000, //,
  0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, //-
  0b00000000, 0b00000011, 0b00000011, 0b00000000, 0b00000000, //.
  0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, //lomitko
  0b00111110, 0b01000101, 0b01001001, 0b01010001, 0b00111110, //0
  0b00010001, 0b00100001, 0b01111111, 0b00000001, 0b00000001, //1
  0b00100001, 0b01000011, 0b01000101, 0b01001001, 0b00110001, //2
  0b01000010, 0b01000001, 0b01010001, 0b01011001, 0b01100110, //3
  0b00001100, 0b00010100, 0b00100100, 0b11111111, 0b00000100, //4
  0b01110010, 0b01010001, 0b01010001, 0b01010001, 0b01001110, //5
  0b00111110, 0b01001001, 0b01001001, 0b01001001, 0b00100110, //6
  0b01000011, 0b01000100, 0b01001000, 0b01010000, 0b01100000, //7
  0b00110110, 0b01001001, 0b01001001, 0b01001001, 0b00110110, //8
  0b00110010, 0b01001001, 0b01001001, 0b01001001, 0b00111110, //9
  0b00000000, 0b00110110, 0b00000000, 0b00000000, 0b00000000, //:
  0b00000000, 0b00110101, 0b00110110, 0b00000000, 0b00000000, //;
  0b00001000, 0b00010100, 0b00100010, 0b01000001, 0b00000000, //<
  0b00010100, 0b00010100, 0b00010100, 0b00010100, 0b00010100, //=
  0b00000000, 0b01000001, 0b00100010, 0b00010100, 0b00001000, //>
  0b00100000, 0b01000000, 0b01001101, 0b01010000, 0b00110000, //?
  0b00111110, 0b01001001, 0b01010101, 0b01001001, 0b00111010, //@
  0b00011111, 0b00100100, 0b01000100, 0b00100100, 0b00011111, //A
  0b01111111, 0b01001001, 0b01001001, 0b01001001, 0b00110110, //B
  0b00111110, 0b01000001, 0b01000001, 0b01000001, 0b00100000, //C
  0b01111111, 0b01000001, 0b01000001, 0b00100010, 0b00011100, //D
  0b01111111, 0b01001001, 0b01001001, 0b01001001, 0b01000001, //E
  0b01111111, 0b01001000, 0b01001000, 0b01001000, 0b01000000, //F
  0b00111110, 0b01000001, 0b01001001, 0b01001001, 0b00101110, //G
  0b01111111, 0b00001000, 0b00001000, 0b00001000, 0b01111111, //H
  0b01000001, 0b01000001, 0b11111111, 0b01000001, 0b01000001, //I
  0b00000110, 0b01000001, 0b01000001, 0b01111110, 0b01000000, //J
  0b01111111, 0b00001000, 0b00010100, 0b00100010, 0b01000001, //K
  0b01111111, 0b00000001, 0b00000001, 0b00000001, 0b00000001, //L
  0b01111111, 0b00100000, 0b00010000, 0b00100000, 0b01111111, //M
  0b01111111, 0b00010000, 0b00001000, 0b00000100, 0b01111111, //N
  0b00111110, 0b01000001, 0b01000001, 0b01000001, 0b00111110, //O
  0b01111111, 0b01001000, 0b01001000, 0b01001000, 0b00110000, //P
  0b00111110, 0b01000001, 0b01000101, 0b01000010, 0b00111101, //Q
  0b01111111, 0b01001000, 0b01001000, 0b01001000, 0b00110111, //R
  0b00110010, 0b01001001, 0b01001001, 0b01001001, 0b00100110, //S
  0b01000000, 0b01000000, 0b01111111, 0b01000000, 0b01000000, //T
  0b01111110, 0b00000001, 0b00000001, 0b00000001, 0b01111110, //U
  0b01111100, 0b00000010, 0b00000001, 0b00000010, 0b01111110, //V
  0b01111110, 0b00000001, 0b00001110, 0b00000001, 0b01111110, //W
  0b01100011, 0b00010100, 0b00001000, 0b00010100, 0b01100011, //X
  0b01110000, 0b00001000, 0b00000111, 0b00001000, 0b01110000, //Y
  0b01000011, 0b01000101, 0b01001001, 0b01010001, 0b01100001, //Z
  0b00000000, 0b01111111, 0b01000001, 0b01000001, 0b00000000, //[
  0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, //\
  0b00000000, 0b01000001, 0b01000001, 0b01111111, 0b00000000, //]
  0b00000000, 0b01000001, 0b01000001, 0b01111111, 0b00000000, //]
  0b00010000, 0b00100000, 0b01000000, 0b00100000, 0b00010000, //^
  0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001, //_
  0b00000000, 0b01000000, 0b00100000, 0b00010000, 0b00000000, //`
  0b00000010, 0b00010101, 0b00010101, 0b00010101, 0b00001111, //a
  0b01111111, 0b00010001, 0b00010001, 0b00010001, 0b00001110, //b
  0b00001110, 0b00010001, 0b00010001, 0b00010001, 0b00000010, //c
  0b00001110, 0b00010001, 0b00010001, 0b00001001, 0b01111111, //d
  0b00001110, 0b00010101, 0b00010101, 0b00010101, 0b00001100, //e
  0b00000000, 0b00001000, 0b00111111, 0b01001000, 0b00100000, //f
  0b00001100, 0b00010101, 0b00010101, 0b00010101, 0b00011110, //g
  0b01111111, 0b00001000, 0b00010000, 0b00010000, 0b00001111, //h
  0b00000000, 0b00010001, 0b01011111, 0b00000001, 0b00000000, //i
  0b00000010, 0b00000001, 0b00000001, 0b00101110, 0b00000000, //j
  0b01111111, 0b00000100, 0b00001100, 0b00010010, 0b00000001, //k
  0b00000000, 0b01000000, 0b01111111, 0b00000000, 0b00000000, //l
  0b00001111, 0b00010000, 0b00001100, 0b00010000, 0b00001111, //m
  0b00011111, 0b00001000, 0b00010000, 0b00010000, 0b00001111, //n
  0b00001110, 0b00010001, 0b00010001, 0b00010001, 0b00001110, //o
  0b00111111, 0b00010100, 0b00100100, 0b00100100, 0b00011000, //p
  0b00011000, 0b00100100, 0b00100100, 0b00101000, 0b00111111, //q
  0b00011111, 0b00001000, 0b00010000, 0b00010000, 0b00001000, //r
  0b00001001, 0b00010101, 0b00010101, 0b00010101, 0b00000010, //s
  0b00010000, 0b01111110, 0b00010001, 0b00000001, 0b00000010, //t
  0b00011110, 0b00000001, 0b00000001, 0b00000010, 0b00011111, //u
  0b00011100, 0b00000010, 0b00000001, 0b00000010, 0b00011100, //v
  0b00011110, 0b00000001, 0b00000110, 0b00000001, 0b00011110, //w
  0b00010001, 0b00001010, 0b00000100, 0b00001010, 0b00010001, //x
  0b00011000, 0b00000101, 0b00000101, 0b00000101, 0b00011110, //y
  0b00010001, 0b00010011, 0b00010101, 0b00011001, 0b00010001, //z
  0b00000000, 0b00001000, 0b00110110, 0b01000001, 0b00000000, //{
  0b00000000, 0b00000000, 0b01111111, 0b00000000, 0b00000000, //|
  0b00000000, 0b01000001, 0b00110110, 0b00001000, 0b00000000, //}
  0b00000100, 0b00001000, 0b00000100, 0b00000010, 0b00000100, //~
  0b00011110, 0b00100001, 0b01010001, 0b00100010, 0b00011111, //ù=249
  0b00001110, 0b01010001, 0b00110001, 0b01010001, 0b00000010, //è=232
  0b01001110, 0b00101001, 0b01001001, 0b00000110, 0b01111111, //ï=239
  0b00001110, 0b01010101, 0b00110101, 0b01010101, 0b00001100, //ì=236
  0b00011111, 0b01001000, 0b00110000, 0b01010000, 0b00001111, //ò=242
  0b00011111, 0b01001000, 0b00110000, 0b01010000, 0b00001000, //ø=248
  0b00001001, 0b01010101, 0b00110101, 0b01010101, 0b00000010, //š=154
  0b00010000, 0b01111110, 0b00010001, 0b01010001, 0b01100010, //=157
  0b00010001, 0b01010011, 0b00110101, 0b01011001, 0b00010001, //ž=158
  0b00000010, 0b00010101, 0b00110101, 0b01010101, 0b00001111, //á=225
  0b00001110, 0b00010101, 0b00110101, 0b01010101, 0b00001100, //é=233
  0b00000000, 0b00001001, 0b00101111, 0b01000001, 0b00000000, //í=237
  0b00001110, 0b00010001, 0b00110001, 0b01010001, 0b00001110, //ó=243
  0b00011110, 0b00000001, 0b00100001, 0b01000010, 0b00011111, //ú=250
  
};



 
unsigned char pole [7][7] = 
{
  {0b10101010,0b10101010,0b10101010,0b10101010,0b10101010,0b10101010,0b10101010}, 
  {0b01010101,0b01010101,0b01010101,0b01010101,0b01010101,0b01010101,0b01010101},
  {0b10101010,0b10101010,0b10101010,0b10101010,0b10101010,0b10101010,0b10101010}, 
  {0b01010101,0b01010101,0b01010101,0b01010101,0b01010101,0b01010101,0b01010101},
  {0b10101010,0b10101010,0b10101010,0b10101010,0b10101010,0b10101010,0b10101010}, 
  {0b01010101,0b01010101,0b01010101,0b01010101,0b01010101,0b01010101,0b01010101},
  {0b10101010,0b10101010,0b10101010,0b10101010,0b10101010,0b10101010,0b10101010}, 
  
};


              


void DispInit(void) {

   
 /* PTDDD = 0x0E;    	    //  jako vystup
  PTDPE = 0;		        // pull-up vypnuty
  
  enable = 1;
  latch = 0;
  PTFDD = 0x30;    	    //  jako vystup
  PTFPE = 0;		        // pull-up vypnuty
  */
	// 1. Povolime hodinovy signal pro porty
	SIM->SCGC5 |= (SIM_SCGC5_PORTC_MASK | SIM_SCGC5_PORTD_MASK);

	// 2. Nastavime funkci pinu na GPIO
	PORTC->PCR[RADEKA_PIN] = PORT_PCR_MUX(1);
	PORTD->PCR[RADEKB_PIN] = PORT_PCR_MUX(1);
	PORTD->PCR[RADEKC_PIN] = PORT_PCR_MUX(1);
	PORTD->PCR[LATCH_PIN] = PORT_PCR_MUX(1);
	PORTD->PCR[ENABLE_PIN] = PORT_PCR_MUX(1);

	// 3. Nastavime smer pinu na vystupni
	PTC->PDDR |= (1 << RADEKA_PIN);
	PTD->PDDR |= (1 << RADEKB_PIN);
	PTD->PDDR |= (1 << RADEKC_PIN);
	PTD->PDDR |= (1 << LATCH_PIN);
	PTD->PDDR |= (1 << ENABLE_PIN);

	ENABLE_HIGH();
	LATCH_LOW();

	// Nastavit casovac
	// HCS08:
  //TPM2SC = 0x4A;  		  // source fbus, delicka 4
  //TPM2MOD = 7140;  	    // modulo registr, TOF perioda 10 ms

	// KL25Z
	// Povolit clock pro TPM2
	SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK;
	// Nastavit zdroj hodin pro casovac TPM (sdileno vsemi moduly TPM)
	// Dostupne zdroje hodinoveho signalu zavisi na CLOCK_SETUP
	// Pro CLOCK_SETUP = 1 nebo 4 je mozno pouzit OSCERCLK (8 MHz)
	// Pro CLOCK_SETUP = 0 (vychozi v novem projektu) PLLFLLCLK (20.97152 MHz)
	// Mozne hodnoty:
	// 0 - clock vypnut
	// 1 - MCGFLLCLK nebo MCGFLLCLK/2
	// 2 - OSCERCLK
	// 3 - MCGIRCLK  (interni generator, 32 kHz nebo 4 MHz)
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1);

	// SPI pro komunikaci s displejem
	Driver_SPI0.Initialize(0);
	Driver_SPI0.PowerControl(ARM_POWER_FULL);
	Driver_SPI0.Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0, 5000000L);

	// Nastavit casovac
	// Pole PS (prescale) muze byt zmeneno pouze, kdyz je
	// citac casovace zakazan (counter disabled), tj. pokud SC[CMOD] = 0
	// ...nejprve zakazat counter
	TPM2->SC = TPM_SC_CMOD(0);

	// ...pockat az se zmena projevi (acknowledged in the LPTPM clock domain)
	while (TPM2->SC & TPM_SC_CMOD_MASK)
		;

	// ... pri zakazanem citaci provest nastaveni modulo
	// Pro clock = 20 MHz / 128; chceme 10 ms periodu > modulo = 1563 Hz
	TPM2->CNT = 0;	// manual doporucuje vynulovat citac
	TPM2->MOD = 500;	// pri 1500 hodne blika

	// ... a nakonec nastavit pozadovane hodnoty vcetne delicky (prescale)
	TPM2->SC = (TPM_SC_TOIE_MASK	// povolit preruseni
			| TPM_SC_TOF_MASK	// smazat pripadny priznak preruseni
			| TPM_SC_CMOD(1)	// vyber interniho zdroje hodinoveho signalu
			| TPM_SC_PS(7));	// delicka = 128

	// Preruseni je treba povolit take v NVIC
	// ...smazat pripadny priznak cekajiciho preruseni
	NVIC_ClearPendingIRQ(TPM2_IRQn);
	// ...povolit preruseni od TPM0
	NVIC_EnableIRQ(TPM2_IRQn);
	// ...nastavit prioritu preruseni: 0 je nejvyssi, 3 nejnizsi
	NVIC_SetPriority(TPM2_IRQn, 2);

  // SPI HC08
	/*
  SPI1BR = 0x01;        // spi clk = 20MHz / 4 = 5 MHz
  SPI1C1 = 0b00010000;  // 2.CPHA = 0; 3.CPOL = 0; 4.MSTR = 1; 
  SPI1C2 = 0x00;
  SPI1C1_SPE = 1;
  */

	// SPI init je vyse aby nedoslo k TOF preruseni pred jeho inicializaci

  
     
  //EnableInterrupts; /* enable interrupts */
  
  
  Fill();//funkce na vymazání displeje
  
  


}


char SetBit(int radek, int sloupec, char zobraz) //vytvoøení funkce na zobrazení bitu
{ 
  // nic neprovede 
  if (radek < 0) return 0; 
  if (radek >6) return 0; 
  if (sloupec < 0) return 0;
  if (sloupec > 49) return 0;
  
  sloupec=49-sloupec; //aby se zaèínalo vypisovat z leve strany
  
  // zobraz = 0 zhasne displej -> musi se zapsat 1
  if(zobraz==0) {
    
    if (sloupec < 8) { // prvni registr
     pole [radek][6] |= (1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak 
    } 
    else if (sloupec < 16) { // druhy registr
      sloupec-=8;  
      pole [radek][5] |= (1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    
    else if (sloupec < 24) { // treti registr
      sloupec-=16;
      pole [radek][4] |= (1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    
    else if (sloupec < 32) { // ctvrty registr
      sloupec-=24;
      pole [radek][3] |= (1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    
    else if (sloupec < 40) { // paty registr
      sloupec-=32; 
      pole [radek][2] |= (1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    else if (sloupec < 48) { // sesty registr
      sloupec-=40; 
      pole [radek][1] |= (1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    
    else if (sloupec < 56) { // sedmy registr
      sloupec-=48;  
      pole [radek][0] |= (1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
  
  } 
  
  // zobraz = 1 rozsviti displej -> musi se zapsat 0
  else{
  
    if (sloupec < 8) { // prvni registr
     pole [radek][6] &= ~(1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak 
    }
  
  
   else  if (sloupec < 16) { // druhy registr
      sloupec-=8;  
      pole [radek][5] &= ~(1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    
   else  if (sloupec < 24) { // treti registr
      sloupec-=16;
      pole [radek][4] &= ~(1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    
   else  if (sloupec < 32) { // ctvrty registr
      sloupec-=24;
      pole [radek][3] &= ~(1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    
   else  if (sloupec < 40) { // paty registr
      sloupec-=32; 
      pole [radek][2] &= ~(1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
   else  if (sloupec < 48) { // sesty registr
      sloupec-=40; 
      pole [radek][1] &= ~(1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
    
   else  if (sloupec < 56) { // sedmy registr
      sloupec-=48;  
      pole [radek][0] &= ~(1 << sloupec); // nastaví bit v sloupci a radku na 1 ostatni necha tak
    }
  
  
  }
  
}

//vymaze displej 
void Clear(void){
  int i;
  int j;
  for(i=0; i<7; i++){
    for(j=0; j<7; j++) pole [i][j] = 0b11111111; 
    
  }
}

//vymaze displej 
void Fill(void){
  int i;
  int j;
  for(i=0; i<7; i++){
    for(j=0; j<7; j++) pole [i][j] = 0b00000000; 
    
  }
}

void ZobrazZnak(char znak, int radek, int sloupec, char invert)
    {   
        char x;
        char y;
        char data;
        
        
        //pro háèky, èárky abecedy
        if(znak==249)znak=127;
        if(znak==232)znak=128;
        if(znak==239)znak=129;
        if(znak==236)znak=130;
        if(znak==242)znak=131;
        if(znak==248)znak=132;
        if(znak==154)znak=133;
        if(znak==157)znak=134;
        if(znak==158)znak=135;
        if(znak==225)znak=136;
        if(znak==233)znak=137;
        if(znak==237)znak=138;
        if(znak==243)znak=139;
        if(znak==250)znak=140;
        znak-=32;//abych zacal od nuly
        
        for (x=0; x<5; ++x) { // bere sloupce znaku
            data =  Abeceda[x+znak*5];// *5 offset na další znak je pìt ( jedno pismeno ma pet znaku)  
            for (y=0; y<7; ++y) { // nastavuje bity v sloupci
                if (data & (1<<(6-y))) {
                    // rozsviti led na daném umístìní (x,y)
                    if(invert==0) SetBit(y+radek,x+sloupec,1);     //pokud chci invertovat do 0
                    else SetBit(y+radek,x+sloupec,0);
                } else {
                    // zhasne led na daném umístìní (x,y)
                    if (invert==0)SetBit(y+radek,x+sloupec,0);    //pokud chci invertovat do 1
                    else SetBit(y+radek,x+sloupec,1);
                }
            }
        }
    }
  


void ZobrazText(char *text, int radek, int sloupec,char invert) 
{
  char pismeno=0;
  int i;
  while(text[pismeno]!='\0') //pojede dokud nebude ukoncovaci znak pole
  {
    ZobrazZnak(text[pismeno],radek,sloupec+(pismeno*6),invert); //Zobrazi znak na displeji
    for(i=0;i<7;i++)SetBit(i,sloupec+(pismeno*6+5),invert);//vykresli volny sloupec mezi pismeny za pismenem, smer 0, pokud chci invertovat do 1
    for(i=0;i<7;i++)SetBit(i,sloupec+(pismeno*6-1),invert);//vykresli volny sloupec mezi pismeny za pismenem, smer 1, pokud chci invertovat do 1
    for(i=0;i<50;i++)SetBit(radek-1,i,invert);//vykresli volny radek, smer 2, pokud chci invertovat do 1
    for(i=0;i<50;i++)SetBit(radek+7,i,invert);//vykresli volny radek, smer 3, pokud chci invertovat do 1
    
    pismeno ++;
  }
  
}



void TPM2_IRQHandler(void) {
	uint8_t data[2];
	// Pokud je zdrojem preruseni TOF
	if (TPM2->SC & TPM_SC_TOF_MASK) {

		// vymazat priznak preruseni
		TPM2->SC |= TPM_SC_TOF_MASK;
		ENABLE_HIGH();	//enable = 1;

		if (radek == 7)
			radek = 0;

		data[0] = pole[radek][0];
		Driver_SPI0.Send(data, 1 );
		/*while (SPI1S_SPTEF == 0)
			__RESET_WATCHDOG();
		SPI1D = pole[radek][0];
		while (SPI1S_SPRF == 0)
			__RESET_WATCHDOG();
		temp = SPI1D;*/

		data[0] = pole[radek][1];
		Driver_SPI0.Send(data, 1 );
		/*
		while (SPI1S_SPTEF == 0)
			__RESET_WATCHDOG();
		SPI1D = pole[radek][1];
		while (SPI1S_SPRF == 0)
			__RESET_WATCHDOG();
		temp = SPI1D;*/

		data[0] = pole[radek][2];
		Driver_SPI0.Send(data, 1 );
		/*while (SPI1S_SPTEF == 0)
			__RESET_WATCHDOG();
		SPI1D = pole[radek][2];
		while (SPI1S_SPRF == 0)
			__RESET_WATCHDOG();
		temp = SPI1D;*/

		data[0] = pole[radek][3];
		Driver_SPI0.Send(data, 1 );
		/*
		while (SPI1S_SPTEF == 0)
			__RESET_WATCHDOG();
		SPI1D = pole[radek][3];
		while (SPI1S_SPRF == 0)
			__RESET_WATCHDOG();
		temp = SPI1D;*/

		data[0] = pole[radek][4];
		Driver_SPI0.Send(data, 1 );
		/*
		while (SPI1S_SPTEF == 0)
			__RESET_WATCHDOG();
		SPI1D = pole[radek][4];
		while (SPI1S_SPRF == 0)
			__RESET_WATCHDOG();
		temp = SPI1D;*/

		data[0] = pole[radek][5];
		Driver_SPI0.Send(data, 1 );
		/*
		while (SPI1S_SPTEF == 0)
			__RESET_WATCHDOG();
		SPI1D = pole[radek][5];
		while (SPI1S_SPRF == 0)
			__RESET_WATCHDOG();
		temp = SPI1D;*/

		data[0] = pole[radek][6];
		Driver_SPI0.Send(data, 1 );
		/*
		while (SPI1S_SPTEF == 0)
			__RESET_WATCHDOG();
		SPI1D = pole[radek][6];
		while (SPI1S_SPRF == 0)
			__RESET_WATCHDOG();
		temp = SPI1D;*/

		// Aktivace prislusneho radku
		// je to v binarnim kodu A, B, C
		//PTDD = (PTDD & 0b11110001) | (radek << 1);
		switch(radek) {
		case 0:
			RADEKA_LOW();
			RADEKB_LOW();
			RADEKC_LOW();
			break;
		case 1:
			RADEKA_HIGH();
			RADEKB_LOW();
			RADEKC_LOW();
			break;
		case 2:
			RADEKA_LOW();
			RADEKB_HIGH();
			RADEKC_LOW();
			break;
		case 3:
			RADEKA_HIGH();
			RADEKB_HIGH();
			RADEKC_LOW();
			break;
		case 4:
			RADEKA_LOW();
			RADEKB_LOW();
			RADEKC_HIGH();
			break;
		case 5:
			RADEKA_HIGH();
			RADEKB_LOW();
			RADEKC_HIGH();
			break;
		case 6:
			RADEKA_LOW();
			RADEKB_HIGH();
			RADEKC_HIGH();
			break;
		}

		LATCH_HIGH();	//latch = 1;
		__RESET_WATCHDOG();
		LATCH_LOW();	//latch = 0;

		ENABLE_LOW();	//enable = 0;

		radek++;
		// TODO: vhodne by bylo prenastavit aby dalsi preruseni nastalo za delsi dobu?
		// aby byly LED delsi dobu rozsviceny?
		// spise ne, zhasnuty jsou jen v dobe od zacatku do konce tohoto kodu, jinak vzdy 1 radek sviti.
		//
	}

}


/* HCS08
interrupt void timer2_ovf_int(void)
{ 
	// nuluj priznak preruseni
 	
  TPM2SC &= 0x7F; 
  
  enable = 1;

  if(radek ==7 ) radek=0;
  
  while(SPI1S_SPTEF==0) __RESET_WATCHDOG(); 
  SPI1D = pole[radek][0]; 
  while(SPI1S_SPRF==0) __RESET_WATCHDOG();
  temp = SPI1D;
  
  while(SPI1S_SPTEF==0) __RESET_WATCHDOG();
  SPI1D = pole[radek][1];
  while(SPI1S_SPRF==0) __RESET_WATCHDOG();
  temp = SPI1D;
  
  while(SPI1S_SPTEF==0) __RESET_WATCHDOG();
  SPI1D = pole[radek][2];
  while(SPI1S_SPRF==0) __RESET_WATCHDOG();
  temp = SPI1D;
  
  while(SPI1S_SPTEF==0) __RESET_WATCHDOG();
  SPI1D = pole[radek][3];
  while(SPI1S_SPRF==0) __RESET_WATCHDOG();
  temp = SPI1D;
  
  while(SPI1S_SPTEF==0) __RESET_WATCHDOG();
  SPI1D = pole[radek][4];
  while(SPI1S_SPRF==0) __RESET_WATCHDOG();
  temp = SPI1D;
  
  while(SPI1S_SPTEF==0) __RESET_WATCHDOG();
  SPI1D = pole[radek][5];
  while(SPI1S_SPRF==0) __RESET_WATCHDOG();
  temp = SPI1D;
  
  while(SPI1S_SPTEF==0) __RESET_WATCHDOG();
  SPI1D = pole[radek][6];
  while(SPI1S_SPRF==0) __RESET_WATCHDOG();
  temp = SPI1D;
  
  // aktivace prislusneho radku
 	PTDD = (PTDD & 0b11110001) | (radek << 1);
 	  
  latch = 1;
  __RESET_WATCHDOG();
  latch = 0;
 	
 	enable = 0; 
 
 	radek++;
}
*/



//zpozdeni
void dwait(void)
{
    int i;
    for(i=0;i<714;i++) 
    {
    __RESET_WATCHDOG();
    }
}
