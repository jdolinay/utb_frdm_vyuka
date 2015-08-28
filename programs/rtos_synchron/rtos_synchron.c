/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Operacni system FreeRTOS, synchronizace uloh pri
 * pristupu ke sdilene promenne.
 * Program vypisuje na displej kitu stridave radu jednicek a radu devitek
 * z globalni promenne. Pokud neni pristup k teto promenne chranen mutexem,
 * dochazi ve vypisu k chybam.
 *
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
#include "semphr.h"		// FreeRTOS semafory a mutexy
#include "drv_lcd.h"

/*
 * Definujte tuto konstantu jako 1, pokud maji procesy pouzit
 * mutex pro zajisteni vzajemneho vylouceni pri praci s promennou
 * G_SdilenaData.
 *
 * 0 - nesynchronizovany pristup; dochazi k chybam ve vypisu na displej
 * 1 - pristup rizeny mutexem; nedochazi k chybam ve vypisu
 */
#define	 SYNCHRON_POUZIT_MUTEX	0


// Prototypy funkci
void TaskWriter( void * pvParameters );
void TaskReader( void * pvParameters );
char* slowstrcpy(char* buff, const char* src);

// Toto je sdilena promenna, do ktere zapisuje jeden proces
// a cte z ni druhy proces.
char G_SdilenaData[20] = "0000.000";

// Podminena cast kodu; provede se jen pro nenulovou hodnotu makra
#if (SYNCHRON_POUZIT_MUTEX)
// Semafor pouzity pro synchronizaci
SemaphoreHandle_t G_Mutex = NULL;
#endif

int main(void) {
	// Inicializace displeje
	LCD_initialize();

	LCD_puts("Spoustim procesy...");

#if (SYNCHRON_POUZIT_MUTEX)
	// vytvoreni semaforu
	G_Mutex = xSemaphoreCreateMutex();	//xSemaphoreCreateBinary();
	if ( G_Mutex == NULL ) {
		while(1)
			;		// chyba: nedostatek pameti
	}

	// Semafor je vytvoren "prazdny", musime jej "dat" operacnimu systemu
	xSemaphoreGive(G_Mutex);
#endif

	// Vytvoreni ulohy 1 - zapis
	xTaskCreate(TaskWriter, /* ukazatel na task */
			"Writer", /* jmeno tasku pro ladeni - kernel awareness debugging */
			configMINIMAL_STACK_SIZE, /* velikost zasobniku = task stack size */
			(void*)NULL, /* pripadny parametr pro task = optional task startup argument */
			tskIDLE_PRIORITY + 1, /* priorita tasku */
			(xTaskHandle*)NULL /* pripadne handle na task, pokud ma byt vytvoreno */
		);

	// Vytvoreni ulohy 2 - cteni
	xTaskCreate(TaskReader, /* ukazatel na task */
			"Writer", /* jmeno tasku pro ladeni - kernel awareness debugging */
			configMINIMAL_STACK_SIZE, /* velikost zasobniku = task stack size */
			(void*)NULL, /* pripadny parametr pro task = optional task startup argument */
			tskIDLE_PRIORITY + 2, /* priorita tasku, vyssi nez u tasku 1 */
			(xTaskHandle*)NULL /* pripadne handle na task, pokud ma byt vytvoreno */
		);

	vTaskStartScheduler(); /* does not return */

	// Sem bychom se nikdy nemeli dostat
	while (1)
		;

	/* Never leave main */
	return 0;
}

/* Toto je uloha (task, proces) s nazvem TaskWriter (funkce TaskWriter)
 Simuluje proces mereni.
 Namereny udaj se zapisuje do promenne G_SdilenaData.
 */
void TaskWriter( void * pvParameters )
{
	int n = 0;
	(void) pvParameters; /* parameter not used */

	for( ;; ) {

		// Zapis se provadi kazdych n ms
		vTaskDelay(400 / portTICK_RATE_MS);


#if (SYNCHRON_POUZIT_MUTEX)
		// Pozadame RTOS o mutex. Cekame nekonecne dlouho.
		// xSemaphoreTake vraci pdTRUE pokud ziska semafor; pdFALSE pokud vyprsi cas
		xSemaphoreTake(G_Mutex, portMAX_DELAY);
#endif

		// Zapisujeme nejakou hodnotu do sdilene promenne.
		// stridave se zapisuje rada jednicek 111... a rada devitek 999...
		if ( (n % 2) == 0 )
			slowstrcpy(G_SdilenaData, "1111.111");
		else
			slowstrcpy(G_SdilenaData, "9999.999");

#if (SYNCHRON_POUZIT_MUTEX)
		xSemaphoreGive(G_Mutex);
#endif

		n++;
	}
}

/* Toto je uloha (task, proces) s nazvem TaskReader (funkce TaskReader)
 Simuluje zpracovani namerenych dat z ulohy TaskWriter
*/
void TaskReader( void * pvParameters )
{
	(void) pvParameters; /* parameter not used */
	for ( ;; ) {

		// Akce se provadi kazdych 600 ms
		vTaskDelay(600 / portTICK_RATE_MS);

#if (SYNCHRON_POUZIT_MUTEX)
		// Pozadame RTOS o mutex. Cekame nekonecne dlouho.
		// xSemaphoreTake vraci pdTRUE pokud ziska semafor; pdFALSE pokud vyprsi cas
		xSemaphoreTake(G_Mutex, portMAX_DELAY);
#endif

		// Zobrazime sdilena data na displeji
		LCD_clear();
		LCD_puts(G_SdilenaData);

#if (SYNCHRON_POUZIT_MUTEX)
		// Vracime mutex operacnimu systemu
		xSemaphoreGive(G_Mutex);
#endif
	}
}


/*
 * Funkce pro pomale kopirovani retezce, abychom lepe videli problem
 * bez synchronizace.
 * Zkopiruje do"buff" retezec "src".
 */
char* slowstrcpy(char* buff, const char* src)
{
	//strcpy(buff, src);
	//return buff;
	char* pTo;
	uint32_t i;
	const char* pFrom;

	// nez zacneme zapisovat vlozime na konec ukoncovaci znak aby byl cilovy buffer vzdy
	// platny retezec, i kdyz se kopirovani teprve zacina
	buff[16] = '\0';

	pFrom = src;
	pTo = buff;
	while (*pFrom != '\0') {
		*pTo = *pFrom;
		pTo++;
		pFrom++;
		// Hodnota cekani je "vyladena" aby casto dochazelo ke konfliktu
		// pri zapisu se zpozdenim 400 ms a cteni se zpozdenim 600 ms.
		for (i = 0; i < 200000; i++) {
			;
		}

	}

	*pTo = '\0';	// ukoncovaci znak uz nebyl zkopirovan

	return pTo;

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
