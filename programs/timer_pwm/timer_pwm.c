/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Casovac TPM, generovani PWM.
 * Program ukazuje rizeni jasu LED s vyuzitim casovace.
 * Cervena LED na FRDM-KL25Z meni jas od 0 do 100% pomoci hardwarove
 * generovaneho PWM casovacem TPM2 s frekvenci 100 Hz.
 *
 * POZOR: v nastaveni projektu > compiler > preprocesor musi byt CLOCK_SETUP=1
 * aby byl CPU clock 48 MHz!
 *
 * Uzitecne informace:
 * Pomoci casovace muzeme ridit LED RGB primo na FRDM-KL25Z desce,
 * LED LD1 az LD3 na vyvojovem kitu nejsou napojeny na kanaly casovace.
 *
 * B18 	- Red LED - TPM2 kanal 0 (ALT3)
 * B19 	- Green LED - TPM2 kanal 1 (ALT3)
 * D1	- Blue LED - TPM0 kanal 1 (ALT4)
 *
 */

#include "MKL25Z4.h"
#include "stdbool.h"

// Cislo kanalu TPM2 na kterem generujeme PWM
// 0 - RED LED
// 1 - GREEN LED
#define	PWM_CHANNEL		(0)

// Cislo pinu, na kterem generujeme PWM
// POZOR: musi byt synchronni s PWM_CHANNEL!
#define	PWM_PINNUMBER	(18)

void delay(void);

int main(void)
{
	// Kolik tiku casovace odpovida 1% sirky pulsu.
	// Nastaveno na spravnou hodnotu nize pri nastaveni casovace
	uint32_t ticksPerPercent = 1;


	// Povolit clock pro TPM0
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
	// !!! Pozor pri zapisu do SOPT2 nespolehat na to, ze oba bity
	// pole TPMSRC jsou vynulovany, nestaci SOPT2[TPMSRC] |= nova_hodnota;
	// je nutno nejprve vynulovat a pak "ORovat" novou hodnotu.
	SIM->SOPT2 &= ~SIM_SOPT2_TPMSRC_MASK;
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(2);


	// Nastavit casovac
	// Pole PS (prescale) muze byt zmeneno pouze, kdyz je
	// citac casovace zakazan (counter disabled), tj. pokud SC[CMOD] = 0
	// ...nejprve zakazat counter
	TPM2->SC = TPM_SC_CMOD(0);

	// ...pockat az se zmena projevi (acknowledged in the LPTPM clock domain)
	while (TPM2->SC & TPM_SC_CMOD_MASK )
		;

	//
	// ... pri zakazanem citaci provest nastaveni
	//
	// Nastavime PWM s pulsy zarovnanymi na zacatek periody (edge aligned).
	// Protoze LED sviti pri log. 0 na pinu, pouzijeme low-true pulses,
	// tj. puls bude hodnota log. 0 a "klid" mezi pulsy bude log. 1.
	// Doba do preteceni citace urcuje periodu PWM.
	// Pozadujeme periodu 100 Hz, tedy doba do preteceni je 0.01 s (10 ms):
	// Modulo = (0.01 * 8000000)/Prescale = 80000/8 = 10000
	// Modulo bude 10 000, coz je 100% sirky pulsu (perioda PWM)
	// Tedy 1% = 100 "tiku" casovace.
	TPM2->CNT = 0;	// manual doporucuje vynulovat citac pred zapisem modulo
	TPM2->MOD = 10000;

	// Nastavime kolik je tiku casovace na 1% sirky pulsu pro lepsi
	// citelnost zbytku programu
	ticksPerPercent = 100;

	// Nastavit rezim casovace na PWM edge aligned, low true pulses
	TPM2->CONTROLS[PWM_CHANNEL].CnSC = (TPM_CnSC_MSB_MASK | TPM_CnSC_ELSA_MASK);

	// Nastavit pin na PWM vystup
	// ... povolit hodinovy signal pro port pinu
	SIM->SCGC5 |=  SIM_SCGC5_PORTB_MASK;
	// ...Kanal casovace je funkce pinu 3 (ALT3)
	PORTB->PCR[PWM_PINNUMBER] = PORT_PCR_MUX(3);


	// Zapiseme pocatecni hodnotu duty, napr. na 10%
	TPM2->CONTROLS[PWM_CHANNEL].CnV = 10 * ticksPerPercent;


	// Nastavime casovac a spustime citani zapisem nenulove delicky (prescale)
	// TPM_SC_PS(3): 3 je hodnota pole PS pro nastaveni delicky na 8
	TPM2->SC = ( TPM_SC_CMOD(1)	| TPM_SC_PS(3) );


	// Budeme plynule menit duty od 0 do 100%
	uint32_t dutyInPercent = 0;
	bool directionUp = true;
	while(1)
	{
		// Zapis nove sirky pulsu do registru casovace
		TPM2->CONTROLS[PWM_CHANNEL].CnV = dutyInPercent * ticksPerPercent;

		if ( dutyInPercent == 0 ) {
			directionUp = true;
		}

		if ( dutyInPercent == 100 ) {
			directionUp = false;
		}

		if ( directionUp )
			dutyInPercent++;
		else
			dutyInPercent--;

		delay();
	}



    /* Never leave main */
    return 0;
}

/* delay
 Jednoducha cekaci funkce - busy wait
*/
void delay(void)
{
	unsigned long n = 50000L;
	while ( n-- )
		;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
