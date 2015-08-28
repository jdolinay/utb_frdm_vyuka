/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Operacni system FreeRTOS, cvicny ukol na synchronizace uloh pri
 * pristupu ke sdilene promenne.
 *
 * Simuluje se výbìr 5 krát 500 Euro a vklad 5 krát 1000 Euro.
 * Problém je, ze výsledek není správný.
 *
 * Poèáteèní stav úètu je 10000 a tak po simulaci má být zùstatek:
 * 10000 + 5x1000 - 5x500 = 12500.
 * Ale není.
 * Doplòte program o synchronizaci, která zajistí správné provedeni operaci tak,
 * aby na úètu byl správný zùstatek.
 *
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


// Prototypy funkci pro procesy (tasky)
void prikazy( void * pvParameters );
void vyber( void * pvParameters );
void vklad( void * pvParameters );

// Dalsi funkce
int BankaOperace(int castka);
void VypisVysledek(void);
uint8_t VkladBezi, VyberBezi;
int nVyberu, nVkladu;
char buff[20];

int main(void) {

	// Inicializace displeje
	LCD_initialize();

	// Pocatecni stav uctu
	BankaOperace(10000);
	nVyberu = nVkladu = 0;

	LCD_puts("Spoustim procesy...");



	// Vytvoreni ulohy 1 - obsluha prikazu
	xTaskCreate(prikazy, /* ukazatel na task */
			"GUI", /* jmeno tasku pro ladeni - kernel awareness debugging */
			configMINIMAL_STACK_SIZE, /* velikost zasobniku = task stack size */
			(void*)NULL, /* pripadny parametr pro task = optional task startup argument */
			tskIDLE_PRIORITY + 3, /* priorita tasku */
			(xTaskHandle*)NULL /* pripadne handle na task, pokud ma byt vytvoreno */
		);

	// Vytvoreni ulohy 2 - vklad
	xTaskCreate(vklad, /* ukazatel na task */
			"vklad", /* jmeno tasku pro ladeni - kernel awareness debugging */
			configMINIMAL_STACK_SIZE, /* velikost zasobniku = task stack size */
			(void*)NULL, /* pripadny parametr pro task = optional task startup argument */
			tskIDLE_PRIORITY + 2, /* priorita tasku, vyssi nez u tasku 1 */
			(xTaskHandle*)NULL /* pripadne handle na task, pokud ma byt vytvoreno */
		);

	// Vytvoreni ulohy 2 - vyber
	xTaskCreate(vyber, /* ukazatel na task */
				"vyber", /* jmeno tasku pro ladeni - kernel awareness debugging */
				configMINIMAL_STACK_SIZE, /* velikost zasobniku = task stack size */
				(void*)NULL, /* pripadny parametr pro task = optional task startup argument */
				tskIDLE_PRIORITY + 1, /* priorita tasku, vyssi nez u tasku 1 */
				(xTaskHandle*)NULL /* pripadne handle na task, pokud ma byt vytvoreno */
			);

	// Spusteni jadra FreeRTOS
	vTaskStartScheduler(); /* does not return */

	// Sem bychom se nikdy nemeli dostat
	while (1)
		;

	/* Never leave main */
	return 0;
}

// proces zajistujici zpracovani prikazu od uzivatele
// Bezi v nekonecne smycce a testuje zda byla stisknuta klavesa
// Pokud ano, provede prislusnou akci
void prikazy( void * pvParameters )
{
	int n = 0;
	(void) pvParameters; /* parameter not used */

	for( ;; ) {

		// Zapis se provadi kazdych n ms
		vTaskDelay(400 / portTICK_RATE_MS);


	}

	int sluzba;

while(1) {

	LCD_clear();
	LCD_puts("Na uctu:");
	sprintf(buff, "%d", BankaOperace(0) );
	LCD_puts(buff);
	setcursor(2,1);
	LCD_puts("SW4=START");
	vTaskDelay(2000 / portTICK_RATE_MS);
	// TODO: cekani na tlacitko...
	while ( PTAD_PTAD7 != 0 )
	  __RESET_WATCHDOG();



	VkladBezi = 1;
	VyberBezi = 1;

	// spustime procesy simulujici vklad a vyber...
	// TODO: jak spustit...
	rtm_start_p (proces_2_id, 0, 14);
	rtm_start_p (proces_3_id, 0, 17);


	while(VkladBezi || VyberBezi)
	{
	  /*if ( PTAD_PTAD7 == 0 )
	  {
				rtm_continue_p(init_id);
				break;
		} */
	}	// while
		// Po zastaveni procesu vkladu i vyberu pokracujeme zde..
	// umoznime opakovat simulaci
	VypisVysledek();
	ZapisUcet(10000);   // nastavit konto na pocatecni castku
	nVyberu = nVkladu = 0;
	while ( PTAD_PTAD7 != 0 )
	  __RESET_WATCHDOG();
}

/* Toto je uloha (task, proces) s nazvem TaskReader (funkce TaskReader)
 Simuluje zpracovani namerenych dat z ulohy TaskWriter
*/
void vklad( void * pvParameters )
{
	(void) pvParameters; /* parameter not used */
	for ( ;; ) {

		LCD_clear();
		LCD_puts("VKLAD: 1000");
		BankaOperace(+1000);

		// Po 5-ti probìhnutích se ukonèíme
		nVkladu++;
		if ( nVkladu >= 5 )
		{
			//TODO: rtm_ch_period_p(proces_2_id, 0);
			VyberBezi = 0;
		}

		// Akce se provadi kazdych 600 ms
		vTaskDelay(800 / portTICK_RATE_MS);
	}

}

void vyber( void * pvParameters )
{
	(void) pvParameters; /* parameter not used */
	for ( ;; ) {

		LCD_clear();
		LCD_puts("VYBER: 500");
		BankaOperace(-500);

		nVyberu++;
		// Po 5-ti probìhnutích se ukonèíme
		if (nVyberu >= 5) {
			// TODO: rtm_ch_period_p(proces_3_id, 0);
			VkladBezi = 0;
		}

		// Akce se provadi kazdych 900 ms
		vTaskDelay(900 / portTICK_RATE_MS);
	}


}


//
// BankaOperace
//
// Pro praci s uctem ve vasem programu vyuzivate tuto funkci
// Provede operaci na úètu, pøi kladné èástce = vklad, pøi záporné výbìr.
// Vrací nový zùstatek úètu
// TIP: lze volat s castka = 0 pro zjisteni zustatku na uctu.
int BankaOperace(int castka)
{
	int sum;

	// Bankovni operace trva dlouho...

	// Hledáme stav úètu...
	sum = NactiUcet();
	if ( castka != 0 )
	{	// pokud opravdu meni stav uctu, a ne jen zjistuje...
		sum = sum + castka;
		ZapisUcet(sum);
	}

	return sum;
}


/////////////////////////////////////////////////////////////////
// TYTO FUNKCE NESMITE MENIT
// Toto jsou funkce pro obsluhu databaze s castami na uctu.
// Kod je dodavan firmou SuperBank Software, certifikován ministerstvem financi
// a nesmi byt menen!
// Vas program je smi pouze vyuzivat, ale nikoliv menit!
int NactiUcet( void )
{
  unsigned int i;
	// Hledani uctu v databazi trva dlouho...
	for ( i = 0; i<300000; i++ )
		  ;
	return g_Ucet;
}

void ZapisUcet(int castka )
{
	unsigned int i;
	// Hledani uctu v databazi trva dlouho...
	for ( i = 0; i<400000; i++ )
		  ;
	g_Ucet = castka;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
