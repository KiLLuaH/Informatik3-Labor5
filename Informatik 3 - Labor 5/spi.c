#include <lpc11xx.h>                        /* LPC11xx definitions */

void Waitms(const unsigned int msWait);


/* ----------------------------------------------------------------
   Initialisierung des SPI0 Interfaces für eine 8-Bit Kommunikation 
   als Master. Es erfolgt nur ein unidirektionaler Datenverkehr vom
   Master zum Slave => kein MISO
   ---------------------------------------------------------------- */ 
void SPIInit8BitMaster(void) {
	
		//I/O-Konfiguration:
		LPC_IOCON->SCK_LOC |= 0x1;						//SCK0 auf PIO2_11 legen										
    LPC_IOCON->PIO2_11 |= 0x1;						//Portpin PIO2_11 als SCK0 für SPI0 verwenden	
		LPC_IOCON->PIO0_9  |= 0x1;						//Portpin PIO0_9 als MOSI0 für SPI0 verwenden
	
		//System-Konfiguration:
	  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<11);	//Enables clock for SPI0; wäre hier nicht notwendig, da default gesetzt
		LPC_SYSCON->SSP0CLKDIV		|= 0x1 ;		//Teiler für SPI0 peripheral clock; 0 bis 255 möglich; 0 => SPI0_PCLK deaktiviert
	  LPC_SYSCON->PRESETCTRL 		|= 0x1;			//deaktiviert SPI0 reset
	
		//SPP0-Einstellungen:
		LPC_SSP0->CR0					 		|= 0x07;		//8-Bit-Übertragung
		LPC_SSP0->CPSR 						 = 0x4;			//SCK-Übertragungstakt einstellen			
		LPC_SSP0->CR1					 		|= 0x02;		//enable SPI0, SPI im Master-Modus
}


/* ----------------------------------------------------------------
   Senden von 8-Bit über das SPI-Interface 
   Achtung: Der Slave muss vorher über Slave-Select (SSEL) 
            aktiviert werden. Nach der Uebertragung sollte SSEL
						wieder inaktiv werden.
   ---------------------------------------------------------------- */
void SPISend8Bit(unsigned char data)
{
		LPC_GPIO2->DATA &= ~0x400;  						//SSEL aktivieren fuer SPI Uebertragung
	  LPC_SSP0->DR 	= data;	 // Eintragen der Daten ins Senderegister und Start der Uebertragung
		while ((LPC_SSP0->SR & 0x14)!=0x04);		// warten bis Daten-Transfer beendet: 0x14 BSY und RNE
		LPC_GPIO2->DATA |= 0x400; 							//SSEL deaktivieren
}  
