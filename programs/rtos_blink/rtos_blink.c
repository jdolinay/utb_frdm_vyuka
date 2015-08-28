/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Operacni system FreeRTOS, periodicke blikani LED
 *
 * Jak pridat FreeRTOS do projektu:
 * 1) Pridat do projektu soubor FreeRTOSConfig.h z FreeRTOS/include/.
 * Vhodne je zkopirovat (Copy file) do projektu a ne linkovat (Link to file),
 * aby mohl mit kazdy projekt svou konfiguraci FreeRTOS.
 *
 * 2) V souboru FreeRTOSConfig.h nastavit konstanty podle hodinove
 * frekvence CPU.
 * Tento projekt je nastaven na 48 MHz: V nastaveni projektu > compiler > preprocesor
 * je pridan symbol CLOCK_SETUP=1
 * Popis jednotlivych CLOCK_SETUP hodnot najdete v Includes/system_MKL25Z4.h
 * Konstanty ve FreeRTOSConfig.h pro CLOCK_SETUP=1:
 * #define configCPU_CLOCK_HZ                       48000000U
 * #define configBUS_CLOCK_HZ                       24000000U
 *
 * Priklad pro vychozi nastaveni projektu v KDS 3.0.0 (CLOCK_SETUP nedefinovan nebo = 0):
 * #define configCPU_CLOCK_HZ                        20971530U
 * #define configBUS_CLOCK_HZ                        20971520U
 *
 *
 * 3) Pridat slozku FreeRTOS do projektu, pretazenim z Pruzkumnika na projekt
 * a volbou "Link to files and folders". Vznikne tak slozka "FreeRTOS" v projektu.
 *
 * 4) Pridat cesty k nasledujicim umistenim do nastaveni C Compiler > Includes:
 * (Pomoci tlacitka Workspace pro vyber cesty)
 * "${workspace_loc:/${ProjName}/Sources/FreeRTOS/include}"
 * "${workspace_loc:/${ProjName}/Sources/FreeRTOS/port}"
 * "${workspace_loc:/${ProjName}/Sources/FreeRTOS/src}"
 *
 * 5) Smazat startup_MKL25Z4.s z Project_Settings/Startup_Code
 * (nebo nastavit ve vlastnostech tohoto souboru Exclude from build).
 *
 * Dalsi informace najdete v /doc/freeRTOS.txt.
 *
 */

#include "MKL25Z4.h"
#include "FreeRTOS.h"

/* Pomocna makra pro praci s LED: */
/* Red RGB LED is on PTB18 */
#define RED_LED             (18)
#define	RED_LED_MASK		(1 << RED_LED)
/* Zmena stavu LED. Vyuzivame registr PTOR (toggle) */
#define	RED_LED_TOGGLE()	PTB->PTOR |= RED_LED_MASK
#define	RED_LED_ON()		PTB->PCOR |= RED_LED_MASK
#define	RED_LED_OFF()		PTB->PSOR |= RED_LED_MASK

// Prototyp funkce = ulohy pro blikani LED
void MainTask( void* pvParameters );


int main(void)
{
	// Inicializace LED
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;		// clock pro port B povolit
	PORTB->PCR[RED_LED] = PORT_PCR_MUX(1);	// pin do GPIO rezimu
	RED_LED_OFF();							// pin na log. 1 = LED zhasnuta
	PTB->PDDR |= RED_LED_MASK;				// pin do vystupniho rezimu

	// Vytvoreni procesu Main
	BaseType_t status = xTaskCreate(
	        MainTask,  /* ukazatel na task */
	        "Main", /* jmeno tasku pro ladeni - kernel awareness debugging */
	        configMINIMAL_STACK_SIZE, /* velikost zasobniku = task stack size */
	        (void*)NULL, /* pripadny parametr pro task = optional task startup argument */
	        tskIDLE_PRIORITY,  /* priorita tasku = initial priority */
	        (xTaskHandle*)NULL /* pripadne handle na task, pokud ma byt vytvoreno */
	      );

	if ( status != pdPASS) {

		while(1) {
			; /* error! probably out of memory */
		}
	}

	vTaskStartScheduler(); /* does not return */

	// Sem bychom se nikdy nemeli dostat
	while(1)
		;

    /* Never leave main */
    return 0;
}

// Toto je proces (task) s nazvem Main (funkce MainTask)
void MainTask( void* pvParameters )
{
	(void) pvParameters; /* parameter not used */


	//
	// 1. varianta kodu: pozastaveni na danou dobu (nepresne)
	//
	for (;;) {
		// Prepnuti stavu LED
		RED_LED_TOGGLE();

		// Pozastaveni procesu na dany pocet tiku.
		// Pro vypocet doby pozastaveni v milisekundach se pouziva makro portTICK_RATE_MS
		// coz je asi 1/(pocet tiku za milisekundu)
		// POZOR: vTaskDelay se nedoporucuje ulohy, ktere maji byt spousteny
		// s presnou periodou, protoze doba pozastaveni je relativni - task je
		// pozastaven na dany pocet tiku od okamziku volani.
		// Pro presne periodicke casovani je doporucena vTaskDelayUntil.
		vTaskDelay(1000 / portTICK_RATE_MS);
	}


	//
	// 2. varianta kodu: zajisteni spousteni kodu v presnych intervalech
	//
	/*
	const TickType_t xFrequency = 500 / portTICK_RATE_MS;
	TickType_t xLastWakeTime;

  	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {

		// Prepnuti stavu LED
		RED_LED_TOGGLE();

		// Postaveni do doby dalsiho spusteni s konstantni periodou
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}*/

}





////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
