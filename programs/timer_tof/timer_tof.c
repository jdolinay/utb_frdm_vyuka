/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Casovac TPM, preruseni od preteceni casovace.
 * Program ukazuje blikani LED s vyuzitim preruseni TOF (preteceni citace).
 * LED blika 2x za sekundu.
 *
 * POZOR: v nastaveni projektu > compiler > preprocesor musi byt CLOCK_SETUP=1
 * aby byl CPU clock 48 MHz!
 *
 * Uzitecne informace:
 *
 */

#include "MKL25Z4.h"
#include "drv_gpio.h"


int main(void)
{
	// Pro ovladani LED pouzijeme ovladac GPIO
	gpio_initialize();
	// Nastavit pin do vystupniho rezimu
	pinMode(LD1, OUTPUT);
	// a na log. 1 tedy LED zhasnuta
	pinWrite(LD1, HIGH);

	// Povolit clock pro TPM0
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;

	// Nastavit zdroj hodin pro TPM casovace (sdileno vsemi moduly TPM)
	// Pozor, dostupne zdroje hodinoveho signalu zavisi na CLOCK_SETUP
	// Pro CLOCK_SETUP = 1 nebo 4 je mozno pouzit OSCERCLK (8 MHz)
	// Pro CLOCK_SETUP = 0 (vychozi v novem projektu) PLLFLLCLK (20.97152 MHz)
	// Mozne hodnoty:
	// 0 - clock vypnut
	// 1 - MCGFLLCLK nebo MCGFLLCLK/2
	// 2 - OSCERCLK
	// 3 - MCGIRCLK
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(2);

	// Nastavit casovac
	// Pozor, pole PS (prescale) muze byt zmeneno pouze, kdyz je
	// citac casovace zakazan (counter disabled), tj. pokud SC[CMOD] = 0
	// ...nejprve zakazat counter
	TPM0->SC = TPM_SC_CMOD(0);

	// ...pockat az se zmena projevi (acknowledged in the LPTPM clock domain)
	while (TPM0->SC & TPM_SC_CMOD_MASK )
		;

	// ... pri zakazanem citaci provest nastaveni modulo
	// Pri clock = 8 MHz / 128 = 62500 Hz
	// Pro 2 preruseni za sekundu modulo nstavit na 31250
	TPM0->CNT = 0;	// manual doporucuje vynulovat citac
	TPM0->MOD = 31250;

	// ... a nakonec nastavit pozadovane hodnoty vcetne prescaleru
	TPM0->SC = ( TPM_SC_TOIE_MASK	// povolit preruseni
			| TPM_SC_TOF_MASK	// smazat pripadny priznak preruseni
			| TPM_SC_CMOD(1)	// vyber interniho zdroje hodinoveho signalu
			| TPM_SC_PS(7) );	// delicka = 128


	// Nic dalsiho se v hlavni smycce programu nedeje, vse resi obsluha preruseni
	while(1)
		;


    /* Never leave main */
    return 0;
}



/* Obsluha pro TOF interrupt.
   Jmeno obsluzne funkce je preddefinovano.
   Staci v nasem programu vytvorit funkci tohoto jmena a bude
   volana ona misto vychozi prazdne funckce.
*/
void TPM0_IRQHandler(void)
{
	// static promenna si uchova hodnotu i mezi volanimi funkce
	static uint8_t ledSviti = 0;

	// Pokud je zdrojem preruseni TOF
	if (TPM0->SC & TPM_SC_TOF_MASK) {

		// vymazat priznak preruseni
		TPM0->SC |= TPM_SC_TOF_MASK;

		// Zmenit stav LED
		if (ledSviti) {
			pinWrite(LD1, HIGH);	// zhasnout
			ledSviti = 0;
		}
		else {
			pinWrite(LD1, LOW);		// rozsvitit
			ledSviti = 1;
		}

	}

}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
