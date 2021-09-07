// Ukazka odezvy programu
//
// Ocekavane chovani:
// Pri stisku tlacitka se spusti efekt, kdy blikaji postupne LED,
// Po uvolneni tlacitka LED zhasnou. Tedy blikani trva po dobu stisku tlacitka.
//
// Ukazano je postupne vylepsovani odezvy programu - aby program rychleji reagoval na
// uvolneni tlacitka. Verzi programu zvolte nize v #define	VERSION
//
// Posledni zmena: 8.9.2021
///////////////////////////////////////////////
#include "MKL25Z4.h"
#include "drv_gpio.h"
#include "drv_systick.h"
#include <stdbool.h>	// kvuli typu bool

// Vyber verze kodu.
// 1. stav blikani a nesviti; kontrola tlacitka vzdy az po bliknuti vsech LED.
// 2. kontrola tlacitka po bliknuti kazde LED.
// 3. nepouziva se delay - polling casu.
// 4. rozdeleni kodu na ulohy - tasky.
#define VERSION 1

// Spolecne definice
#define SWITCH_PRESSED  	(1)
#define SWITCH_NOT_PRESSED  (0)

#define KEY		SW1
#define LED1    LD1
#define LED2    LD2
#define LED3    LD3
#define BLINK_DELAY   500

// Prototypy funkci
void init(void);
int switch1_read(void);
void delay_debounce(void);
void LED_control(bool d1, bool d2, bool d3);

///////////////////////////////////////////////////////////////////////////////////////
// Kod programu v nekolika verzich.
// Aktivni verze se vybere pomoci #define VERSION nahore.
#if VERSION == 1
// Verze 1
// Odezva je spatna, protoze stav tlacitka se testuje az po bliknuti vsech LED.
// Pri uvolneni tlacitka program zareaguje az na konci celeho efektu, kdyz zhasne
// treti LED a prvni uz se nerozsviti.

// Stavy programu
#define ST_EFFECT	1
#define ST_OFF		2

int state = ST_OFF;


int main(void) {
	// inicializace ovladace pinu a delay
	init();

	while (1) {
		if (switch1_read() == SWITCH_PRESSED)
			state = ST_EFFECT;
		else
			state = ST_OFF;

		switch (state) {
		case ST_OFF:
			LED_control(false, false, false);
			break;

		case ST_EFFECT:
			LED_control(true, false, false);
			SYSTICK_delay_ms(BLINK_DELAY);
			LED_control(false, true, false);
			SYSTICK_delay_ms(BLINK_DELAY);
			LED_control(false, false, true);
			SYSTICK_delay_ms(BLINK_DELAY);
			break;
		}	// switch

	}	// while

	/* Never leave main */
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////

#elif VERSION == 2
// Verze 2
// Odezva je vylepsena tim, ze kontrola tlacitka probiha po bliknuti kazde LED.
// Po uvolneni tlacitka pak LED sice "dosviti" svou dobu svitu,
// ale dalsi LED uz se nerozsviti.

// Stavy programu
#define ST_LED1_ON	1
#define ST_LED2_ON	2
#define ST_LED3_ON	3
#define ST_OFF		4

int state = ST_OFF;

int main(void) {
	// inicializace ovladace pinu a delay
	init();

	while (1) {
		if (switch1_read() == SWITCH_PRESSED) {
			// Jen pokud je stisknuto tlacitko a soucasny stav je vypnuto,
			// prejdeme na stav rozsviceni prvni LED, jinak uz nektera LED
			// sviti a stavy se meni ve switch.
			if ( state == ST_OFF )
				state = ST_LED1_ON;
		}
		else
			state = ST_OFF;

		switch (state) {

		case ST_OFF:
			LED_control(false, false, false);
			break;

		case ST_LED1_ON:
			// Bliknout LED1 a prejit na stav dalsi LED2
			LED_control(true, false, false);
			SYSTICK_delay_ms(BLINK_DELAY);
			state = ST_LED2_ON;
			break;

		case ST_LED2_ON:
			LED_control(false, true, false);
			SYSTICK_delay_ms(BLINK_DELAY);
			state = ST_LED3_ON;
			break;

		case ST_LED3_ON:
			LED_control(false, false, true);
			SYSTICK_delay_ms(BLINK_DELAY);
			state = ST_LED1_ON;
			break;
		}	// switch

	}	// while

	/* Never leave main */
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////

#elif VERSION == 3
// Verze 3
// Odezva je vylepsena tim, ze se nepouziva cekani (busy waiting) ale zjistuje se
// zda uz ubehl potrebny cas - jde o tzv. polling - dotazovani.
// Diky tomu LED zhasne okamzite po uvolneni tlacitka.

// Stavy programu
#define ST_LED1_ON		1
#define ST_LED2_ON		2
#define ST_LED3_ON		3
#define ST_OFF			4
#define ST_LED1_WAIT	5
#define ST_LED2_WAIT    6
#define ST_LED3_WAIT    7

int state = ST_OFF;

int main(void) {
	// inicializace ovladace pinu a delay
	init();

	uint32_t waitStart;		// cas, kdy se rozvitila LED
	uint32_t currentTime;	// aktualni cas, pomocna promenna

	while (1) {
		if (switch1_read() == SWITCH_PRESSED) {
			// Jen pokud je stisknuto tlacitko a soucasny stav je vypnuto,
			// prejdeme na stav rozsviceni prvni LED, jinak uz nektera LED
			// sviti a stavy se meni ve switch.
			if ( state == ST_OFF )
				state = ST_LED1_ON;
		}
		else
			state = ST_OFF;

		switch (state) {

		case ST_OFF:
			LED_control(false, false, false);
			break;

		case ST_LED1_ON:
			// Rozsvitit LED, ulozit aktualni cas a prejit do stavu cekani na
			// uplynuti casu svitu teto LED.
			LED_control(true, false, false);
			waitStart = SYSTICK_millis();
			state = ST_LED1_WAIT;
			break;

		case ST_LED1_WAIT:
			// Kontrola jestli uz ubehlo dost casu abychom rozsvitili dalsi LED
			// a pokud ano, prechod na dalsi stav
			currentTime = SYSTICK_millis();
			if ( currentTime - waitStart >= BLINK_DELAY )
				state = ST_LED2_ON;
			break;


		case ST_LED2_ON:
			LED_control(false, true, false);
			waitStart = SYSTICK_millis();
			state = ST_LED2_WAIT;
			break;

		case ST_LED2_WAIT:
			currentTime = SYSTICK_millis();
			if ( currentTime - waitStart >= BLINK_DELAY )
				state = ST_LED3_ON;
			break;

		case ST_LED3_ON:
			LED_control(false, false, true);
			waitStart = SYSTICK_millis();
			state = ST_LED3_WAIT;
			break;

		case ST_LED3_WAIT:
			currentTime = SYSTICK_millis();
			if ( currentTime - waitStart >= BLINK_DELAY )
				state = ST_LED1_ON;
			break;

		}	// switch

	}	// while

	/* Never leave main */
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////

#elif VERSION == 4
// Verze 4
// Program je vylepsen rozdelenim na jednotlive ulohy - tasky.
// Z pohledu odezvy je chovani stejne jako u verze 3, ale struktura programu je
// prehlednejsi a snadneji by se rozsiroval o dalsi cinnosti.

// Stavy programu
#define ST_LED1_ON		1
#define ST_LED2_ON		2
#define ST_LED3_ON		3
#define ST_OFF			4
#define ST_LED1_WAIT	5
#define ST_LED2_WAIT    6
#define ST_LED3_WAIT    7

// globalni promenna pro stav tlacitka SW1
bool SW1_pressed;
// Promenna state je nove lokalni uvnitr tasku (funkce) pro blikani

// Prototypy funkci
void TaskSwitches(void);
void TaskEffect(void);
void TaskGreenLed(void);

int main(void) {
	// inicializace ovladace pinu a delay
	init();

	while (1) {

		TaskSwitches();
		TaskEffect();
		TaskGreenLed();

	}	// while

	/* Never leave main */
	return 0;
}

// Uloha, ktera se stara o obsluhu tlacitek
void TaskSwitches(void)
{
	if (switch1_read() == SWITCH_PRESSED)
		SW1_pressed = true;
	else
		SW1_pressed = false;
}

// Uloha, ktera se stara o blikani LED
void TaskEffect(void) {
	// Stav totoho tasku.
	// Promenna je static, aby si uchovala hodnotu mezi volanimi teto funkce,
	// tj. aby nezanikla na konci funkce
	static int state = ST_LED1_ON;

	static uint32_t waitStart;		// cas, kdy se rozvitila LED, musi byt static!
	uint32_t currentTime;	// aktualni cas, pomocna promenna

	// Uloha efekt LED se provadi jen pri stisknutem tlacitku
	if (SW1_pressed) {
		switch (state) {

		case ST_LED1_ON:
			// Rozsvitit LED, ulozit aktualni cas a prejit do stavu cekani na
			// uplynuti casu svitu teto LED.
			LED_control(true, false, false);
			waitStart = SYSTICK_millis();
			state = ST_LED1_WAIT;
			break;

		case ST_LED1_WAIT:
			// Kontrola jestli uz ubehlo dost casu abychom rozsvitili dalsi LED
			// a pokud ano, prechod na dalsi stav
			currentTime = SYSTICK_millis();
			if (currentTime - waitStart >= BLINK_DELAY)
				state = ST_LED2_ON;
			break;

		case ST_LED2_ON:
			LED_control(false, true, false);
			waitStart = SYSTICK_millis();
			state = ST_LED2_WAIT;
			break;

		case ST_LED2_WAIT:
			currentTime = SYSTICK_millis();
			if (currentTime - waitStart >= BLINK_DELAY)
				state = ST_LED3_ON;
			break;

		case ST_LED3_ON:
			LED_control(false, false, true);
			waitStart = SYSTICK_millis();
			state = ST_LED3_WAIT;
			break;

		case ST_LED3_WAIT:
			currentTime = SYSTICK_millis();
			if (currentTime - waitStart >= BLINK_DELAY)
				state = ST_LED1_ON;
			break;

		}	// switch
	} else {
		// zhasnout LED pokud neni stisknuto tlacitko
		LED_control(false, false, false);
		state = ST_LED1_ON;	// reset stavu tasku
	}

}	// TaskEffect


/////////////////////////////////
// Doba svitu/zhasnuti zelene LED
#define		GREEN_ON_DELAY		200
#define		GREEN_OFF_DELAY		700

// Ukazka dalsiho tasku - blika pri stisknutem tlacitku RGB LED
void TaskGreenLed() {
    static enum {
    	ST_LED_ON,
		ST_ON_WAIT,
        ST_LED_OFF,
		ST_OFF_WAIT

    } stav = ST_LED_ON;

    static uint32_t startTime;

    // uloha se provadi jen pri stisknutem tlacitku
	if (SW1_pressed) {

		switch (stav) {

		case ST_LED_ON:
			pinWrite(LED_GREEN, LOW);
			startTime = SYSTICK_millis();
			stav = ST_ON_WAIT;
			break;

		case ST_ON_WAIT:
			if (SYSTICK_millis() - startTime >= GREEN_ON_DELAY)
				stav = ST_LED_OFF;
			break;

		case ST_LED_OFF:
			pinWrite(LED_GREEN, HIGH);
			startTime = SYSTICK_millis();
			stav = ST_OFF_WAIT;
			break;

		case ST_OFF_WAIT:
			if (SYSTICK_millis() - startTime >= GREEN_OFF_DELAY)
				stav = ST_LED_ON;
			break;
		}   // switch
	} else {
		pinWrite(LED_GREEN, HIGH);   // zhasni LED
		stav = ST_LED_ON;	// resetuj stav LED
	}
}


//////////////////////////////////////////////////////////////////////////////////
#endif	/* VERSION == 4*/

/////////////////////////////////////////////////////////
// Pomocne funkce spolecne pro vsechny verze

// inicializace
void init()
{
	SYSTICK_initialize();

	GPIO_Initialize();

	pinMode(SW1, INPUT_PULLUP);
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinWrite(LED1, HIGH);
	pinWrite(LED2, HIGH);
	pinWrite(LED3, HIGH);
	pinWrite(LED_GREEN, HIGH);


}

// Ovladani LED - parametr true znamena rozsvitit prislusnou LED, false zhasnout.
void LED_control(bool d1, bool d2, bool d3)
{
    pinWrite(LED1, !d1);
    pinWrite(LED2, !d2);
    pinWrite(LED3, !d3);
}

/*
 switch1_read
 Cte stav tlacitka SW1 s osetrenim zakmitu.
 Vraci SWITCH_NOT_PRESSED pokud tlacitko neni stisknuto,
 SWITCH_PRESSED pokud je stisknuto.

 Reads and debounces switch SW1 as follows:
 1. If switch is not pressed, return SWITCH_NOT_PRESSED.
 2. If switch is pressed, wait for about 20 ms (debounce),
    then:
    if switch is no longer pressed, return SWITCH_NOT_PRESSED.
    if switch is still pressed, return SWITCH_PRESSED.
*/
int switch1_read(void)
{
    int switch_state = SWITCH_NOT_PRESSED;
    if ( pinRead(SW1) == LOW )
    {
    	// tlacitko je stisknuto

        // debounce = wait
    	SYSTICK_delay_ms(50);

        // znovu zkontrolovat stav tlacitka
        if ( pinRead(SW1) == LOW )
        {
            switch_state = SWITCH_PRESSED;
        }
    }
    // vratime stav tlacitka
    return switch_state;
}

/////////////////////////////////////////////////////////////////////////////
// Spaghetti version of the code
#if VERSION == 5
// Zlepseni odezvy pridanim kontroly tlacitka do stavu pro efekt.
// Odezva se sice zlepsi, ale struktura programu je neprehledna.

// Stavy programu
#define ST_EFFECT	1
#define ST_OFF		2

int state = ST_OFF;


int main(void) {
	// inicializace ovladace pinu a delay
	init();

	while (1) {
		if (switch1_read() == SWITCH_PRESSED)
			state = ST_EFFECT;
		else
			state = ST_OFF;

		switch (state) {
		case ST_OFF:
			LED_control(false, false, false);
			break;

		case ST_EFFECT:
			LED_control(true, false, false);
			SYSTICK_delay_ms(BLINK_DELAY);
			// Pokud neni stisknuto tlacitko prerus sekvenci
			if ( switch1_read() != SWITCH_PRESSED )
				break;
			LED_control(false, true, false);
			SYSTICK_delay_ms(BLINK_DELAY);
			// Pokud neni stisknuto tlacitko prerus sekvenci
			if ( switch1_read() != SWITCH_PRESSED )
				break;
			LED_control(false, false, true);
			SYSTICK_delay_ms(BLINK_DELAY);
			break;
		}	// switch

	}	// while

	/* Never leave main */
	return 0;
}

#endif



////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
