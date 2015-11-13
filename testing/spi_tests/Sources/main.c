/*
 Pokusy s ovladacem pro SPI0 dle CMSIS

 * Postup vytvoreni projektu s ovladacem SPI
 *
 * 1) Pridat do projektu (Copy file) soubor RTE_Devices.h z CMSIS_Driver/Config.
 * Zkopirovat (Copy file) do projektu a ne linkovat (Link to file),
 * aby mohl mit kazdy projekt svou konfiguraci ovladacu.
 *
 * 2) vlozit do zdrojoveho kodu #include "RTE_Device.h"
 *
 * 3) Pridat do projektu zdrojove kody ovladace SPI z CMSIS_Driver:
 * Propojit (Link to files), nevytvaret kopii v projektu.
 * SPI: SPI_MKL25Z4.c
 *
 * 4) Pridat slozku KSDK do projektu, pretazenim z Pruzkumnika na projekt
 * a volbou "Link to files and folders". Vznikne tak slozka "KSDK" v projektu.
 *
 * 4) Pridat cesty k nasledujicim umistenim do nastaveni C Compiler Includes:
 *  CMSIS_Driver
 *  KSDK/hal
 *  KSDK/mkl25z4
 *
 *  Muzeme pridat absolutni cesty. Pro cesty v KSDK muzeme take pridat odkazy
 *  pres tlacitko Workspace.
 *  Priklad konkretnich cest v seznamu Includes:
 *  "../../../CMSIS_Driver"
 *  "${workspace_loc:/${ProjName}/KSDK/hal}"
 *  "${workspace_loc:/${ProjName}/KSDK/mkl25z4}"
 */

#include "MKL25Z4.h"
#include "RTE_Device.h"
#include "InfoDisp.h"

static int i = 0;

int main(void)
{

	DispInit();
	Clear();	// zhasne vsechny body; zobrazovani rozsvicenim znaku vyzaduje vypnute pozadi
	//Fill();

	//ZobrazZnak('A', 0, 10, 0);	// 0 = zhasni, 1 = rozsvit bod
	ZobrazText("Ahoj UTB", 0, 0, 0);	// 0 = zhasni, 1 = rozsvit bod
	//SetBit(2,5,1);

    /* This for loop should be replaced. By default this loop allows a single stepping. */
    for (;;) {
        i++;
    }
    /* Never leave main */
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
