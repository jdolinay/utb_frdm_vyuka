/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Casovac TPM, cekani s presne danou delkou (polling).
 * Program ukazuje blikani LED s vyuzitim cekaci funkce, ktera pouziva
 * casovac TPM0 k odmereni casu.
 * LED blika: 1 s sviti a 1s nesviti.
 *
 * POZOR: v nastaveni projektu > compiler > preprocesor musi byt CLOCK_SETUP=1
 * aby byl CPU clock 48 MHz!
 *
 * Uzitecne informace:
 *
 */

#include "MKL25Z4.h"

// ovladac GPIO pro praci s LED
#include "drv_gpio.h"

// Prototypy funkci
void TPMDelay1s(void);
void TPMInitialize(void);


int main(void)
{
	// Pro ovladani LED pouzijeme ovladac GPIO
	GPIO_initialize();
	pinMode(LD1, OUTPUT);
	pinWrite(LD1, HIGH);

	// Blikani LED
	while(1) {
		pinWrite(LD1, LOW);
		TPMDelay1s();
		pinWrite(LD1, HIGH);
		TPMDelay1s();
	}


    /* Never leave main */
    return 0;
}

/*
 Cekani 1 s s vyuzitim casovace TPM0.
 */
void TPMDelay1s(void)
{
	// Inicializace casovace
	TPMInitialize();

	// Cekame na priznak TOF
	while ( !(TPM0->SC & TPM_SC_TOF_MASK) )
		;

	// Vynulujeme priznak TOF, coz se dela zapisem 1 do TOF
	TPM0->SC |= TPM_SC_TOF_MASK;

}

/*
 Inicializuje casovac TPM1 pro pouziti v TPMDelay1s.
 Tuto funkci neni treba volat pred pouzitim cekaci funkce,
 je volana interne z funkce TPMDelay1s().
 */
void TPMInitialize(void)
{
	// Povolit clock pro TPM0
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;

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
	TPM0->SC = TPM_SC_CMOD(0);

	// ...pockat az se zmena projevi (acknowledged in the LPTPM clock domain)
	while (TPM0->SC & TPM_SC_CMOD_MASK)
		;

	// ... pri zakazanem citaci provest nastaveni modulo
	// Pri clock = 8 MHz / 128 = 62500 Hz,
	// tedy citac napocita do 62500 za 1 sekundu.
	// Pro cekani 1 ms potrebujeme, aby citac pretekal pri hodnote 62500
	TPM0->CNT = 0;	// manual doporucuje vynulovat citac
	TPM0->MOD = 62500;

	// ... a nakonec nastavit pozadovane hodnoty:
	// ... delicka (prescale) = 128
	// ... interni zdroj hodinoveho signalu
	TPM0->SC = ( TPM_SC_CMOD(1) | TPM_SC_PS(7));

}








////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
