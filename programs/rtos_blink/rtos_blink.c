/*
 * Ukazkovy program pro Programovani mikropocitacu
 * Operacni system FreeRTOS, periodicke blikani LED
 *
 * POZOR: V souboru FreeRTOS/include/FreeRTOSConfig.h je nutno nastavit konstanty podle hodinove
 * frekvence CPU. Priklad pro vychozi nastaveni projektu v KDS 3.0.0:
 * #define configCPU_CLOCK_HZ                        20971530U
   #define configBUS_CLOCK_HZ                        20971520U
 * Tento projekt je nastaven na 48 MHz: V nastaveni projektu > compiler > preprocesor
 * je symbol CLOCK_SETUP=1
 * Popis jednotlivych CLOCK_SETUP hodnot najdete v Includes/system_MKL25Z4.h
 *
 * Soubory RTOS pridany pretazenim z file manager na projekt v KDS a volbou
 * "Link to files and folders".
 *
 * V souboru Project_Settings/Startup_Code/system_MKL25Z4.s jsou obsluhy preruseni
 * nasmerovany na RTOS pro tyto preruseni:
 * SVC_Handler 		> vPortSVCHandler
 * PendSV_Handler 	> vPortPendSVHandler
 * SysTick_Handler  > vPortTickHandler
 *
 * Zde je kus zmeneneho souboru (bez komentaru)
    .long   vPortSVCHandler		    //SVC_Handler
    .long   0                       // Reserved
    .long   0                       // Reserved
    .long   vPortPendSVHandler      // PendSV_Handler
    .long   vPortTickHandler        // SysTick_Handler
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


// Toto je proces (task) s nazvem Main (funkce MainTask)
static portTASK_FUNCTION(MainTask, pvParameters) {
  (void)pvParameters; /* parameter not used */
  for(;;)
  {
    // Prepnuti stavu LED
	RED_LED_TOGGLE();
/*
    // Pozastaveni procesu na dany pocet tiku.
	// Pro vypocet doby pozastaveni se pouziva makro portTICK_RATE_MS
	// coz je asi 1/(pocet tiku za milisekundu)
	// POZOR: vTaskDelay se nedoporucuje ulohy, ktere maji byt spousteni
	// s presnou periodou, protoze doba pozastaveni je relativni - task je
	// pozastaven na dany pocet tiku od okamziku volani..
	// Pro presne periodicke casovani je doporucena vTaskDelayUntil.
    vTaskDelay(1000/portTICK_RATE_MS);
  */
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 500/portTICK_RATE_MS;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
}

int main(void)
{
	// Inicializace LED
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;		// clock pro port B povolit
	PORTB->PCR[RED_LED] = PORT_PCR_MUX(1);	// pin do GPIO rezimu
	RED_LED_OFF();							// pin na log. 1 = LED zhasnuta
	PTB->PDDR |= RED_LED_MASK;				// pin do vystupniho rezimu

	// Vytvoreni procesu Main
	if (xTaskCreate(
	        MainTask,  /* pointer to the task */
	        "Main", /* task name for kernel awareness debugging */
	        configMINIMAL_STACK_SIZE, /* task stack size */
	        (void*)NULL, /* optional task startup argument */
	        tskIDLE_PRIORITY,  /* initial priority */
	        (xTaskHandle*)NULL /* optional task handle to create */
	      ) != pdPASS)
	{

		while(1)
			; /* error! probably out of memory */
	}

	vTaskStartScheduler(); /* does not return */

	while(1) ;

    /* Never leave main */
    return 0;
}




////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
