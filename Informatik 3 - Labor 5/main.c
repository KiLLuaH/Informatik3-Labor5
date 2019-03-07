#include <lpc11xx.h>
#include "spi.h"
#include <stdio.h>
#include <stdlib.h>

unsigned int button;		// Button1
unsigned int button2;		// Button2
int button_count = 0;		// Button_press_timer
int button2_count = 0;	// Button_press_timer
int move_timer = 0;			// Taktzähler der Bewegungen der Walls
int running = 1;				// 0 == false; 1 == true;
int rundenanzahl = 0;
int zufallszahl = 1;
int guterBlock = 1;			// Bestimmt, welcher Block passierbar ist; ändert collisionsBereiche
int wallStep = 0;				// Zählt Schritte der Wand, ermöglicht GameFreeze

int x; //globale Variable für die Schleife
int v; //globale Variable für die zweite schleife (Player darstellen)


int farbe_1 = 0x00;
int farbe_2 = 0x00;
int farbe_3 = 0x00;
int farbe_4 = 0x00;



unsigned short player[] =			// Initialisierung vom player
	{
		0xEF08,	0x1800,	0x1237, 0x154A,	0x130F, // initialisierung, Ausrichtung, x1, x2, y1, y2
		0x1622
	};

unsigned short wall_1 [] =		//Initialisierung Wand 1
{
	0xEF08,	0x1800, 0x1200 , 0x151F ,0x1397 ,0x16AF
};

unsigned short wall_2 [] =		//Initialisierung Wand 2
{
	0xEF08,	0x1800, 0x1220 , 0x1540 ,0x1397 ,0x16AF
};

unsigned short wall_3 [] =		//Initialisierung Wand 3
{
	0xEF08,	0x1800, 0x1241 , 0x1561 ,0x1397 ,0x16AF
};

unsigned short wall_4 [] =		//Initialisierung Wand 4
{
	0xEF08,	0x1800, 0x1262 , 0x1583 ,0x1397 ,0x16AF
};


void Waitms(const unsigned int msWait) {
  unsigned int   aktTime, diff;

  aktTime = LPC_TMR32B0->TC; 		// Aktuellen Zählerstand des Timers auslesen

  do {
					 // Bereits vergangene Zeit berechnen
			if (LPC_TMR32B0->TC >= aktTime) diff = LPC_TMR32B0->TC - aktTime;
			else diff = (0xFFFFFFFF - aktTime) + LPC_TMR32B0->TC;
  } while (diff	< msWait);
}



/* ----------------------------------------------------------------
   Senden einer Sequenz von  16-Bit Kommandos an das TFT-Display
   mittels SPI-Interface. Dabei wird die Data/Command Leitung
   entsprechend gesetzt
   ---------------------------------------------------------------- */
void SendCommandSeq(const unsigned short * data, int Anzahl)
{
  int  index;
	unsigned char   SendeByte;

	for (index=0; index<Anzahl; index++)
	{
		LPC_GPIO0->DATA |= 0x10;		  						//Data/Command auf High => Kommando-Modus
			
		SendeByte = (data[index] >> 8) & 0xFF; 	  //High-Byte des Kommandos
		SPISend8Bit(SendeByte);

		SendeByte = data[index] & 0xFF; 	  			//Low-Byte des Kommandos
		SPISend8Bit(SendeByte);

		LPC_GPIO0->DATA &= ~0x10;	          			//Data Command auf Low => Daten-Modus	
	}
}


/* ----------------------------------------------------------------
   Initialisierung des TFT-Displays fuer den 65536-Farben Modus
   entsprechend der in der Vorlesung beschriebenen Schritte einschließlich
   der Uebertragung der Initialisierungs-Kommandos.
   Zur Ueberwachung der Zeitabstaende wird die Funktion Waitms verwendet.
   Zum senden eines Kommandos an das Display wird die Funktion SendCommand
   verwendet. 
   ---------------------------------------------------------------- */
void InitDisplay(void) {
	
    const unsigned short InitData[] = 
	{ 
	  //Initialisierungsdaten fuer 256 Farben Modus
		0xFDFD, 0xFDFD, 
		/* pause  */
		0xEF00, 0xEE04, 0x1B04, 0xFEFE, 0xFEFE, 
		0xEF90, 0x4A04, 0x7F1F, 0xEE04, 0x4306,  	// 1f in [7] für 256 Colors
		/* pause  */
		0xEF90, 0x0983, 0x0800, 0x0BAF, 0x0A00, 
		0x0500, 0x0600, 0x0700, 0xEF00, 0xEE0C, 
		0xEF90, 0x0080, 0xEFB0, 0x4902, 0xEF00, 
		0x7F01, 0xE181, 0xE202, 0xE276, 0xE183, 
		0x8001, 0xEF90, 0x0000,
		// pause
		0xEF08,	0x1800,	0x1200, 0x1583,	0x1300,
		0x16AF 	//Hochformat 132 x 176 Pixel 
	}; 
	
	

	Waitms(300); 
	LPC_GPIO1->DATA &= ~0x80;  //Reset-Eingang des Displays auf Low => Hardware-Reset;
    Waitms(75);
	LPC_GPIO2->DATA |= 0x400;  //SSEL auf High
    Waitms(75);
	LPC_GPIO0->DATA |= 0x10;   //Data/Command auf High
    Waitms(75);
	LPC_GPIO1->DATA |= 0x80;   //Reset-Eingang des Displays auf High => kein Hardware-Reset 
    Waitms(75); 
	
  SendCommandSeq(&InitData[0], 2);
    Waitms(75); 
  SendCommandSeq(&InitData[2], 10);
    Waitms(75); 
  SendCommandSeq(&InitData[12], 23);
		Waitms(75); 
  SendCommandSeq(&InitData[35], 6); 
}


int main() {

	 LPC_SYSCON->SYSAHBCLKCTRL |= (1<<16);	//siehe Hinweis in Getting Started
	 LPC_SYSCON->SYSAHBCLKCTRL |= (1<<9);		//Enables clock for 32-bit counter/timer 0
	 LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);		//Enables Clock for 16-bit-counter/timer0, s. [3.5.14]
  
   LPC_GPIO0->DIR	|= 0x10;				//PIO0_4 als Data/Command, digitaler Ausgang
	 LPC_GPIO1->DIR |= 0x80;				//PIO1_7 als Reset, digitaler Ausgang
	 LPC_GPIO2->DIR |= 0x400;       //PIO2_10 als SSEL, digitaler Ausgang
	 LPC_GPIO3->DIR |= 0x3F;        // PIO3_0 bis PIO3_5 sind Ausgänge
	
	 	
   //Timer32B0 initialisieren. Er liefert die Zeitbasis fuer die Funktion waitms
   LPC_TMR32B0->PR  = 48000; 	//bei P-Clock 48Mhz ergibt sich 1Khz Timer Auflösung
   LPC_TMR32B0->TCR = 0x02;  	//setzt Timer zurueck und haelt ihn an
   LPC_TMR32B0->TCR = 0x01;  	//startet Timer
	 LPC_GPIO2->IS |= 0x00;
	 LPC_GPIO2->IEV |= 0x00;
	 NVIC->ISER[0] |=0x20000000;

	 //Timer16BO initialisieren. Er liefert die Zeitbasis zum entprellen unserer Buttons.
	 LPC_TMR16B0->PR = 48000;	  	//Prescaler setzten für Milisekunden
	 LPC_TMR16B0->TCR = 0x02;	 		//Timer 16B0 reset und anhalten
	 LPC_TMR16B0->MR0 = 1;		    //Match-Register0 = Jede Milisekunde
	 LPC_TMR16B0->MCR = 0x03;	  	//Match-Controll-Register IR und Reset durch MR0
	 LPC_TMR16B0->TCR = 0x1;			//Timer16B0 starten
	 NVIC->ISER[0] |= 0x10000;		//Interrupt Set Enable für Exception 16, Timer16_0, erlaubt Interrupts
	
	 //SPI-Schnittstelle initialisieren:
   SPIInit8BitMaster();
   
   //Display initialisieren:
   InitDisplay();
	 
	 for(x = 0; x<23232; x++){  // Komplettes Display weiß färben
		SPISend8Bit(0xFF);
	 }
	
	 SendCommandSeq(&player[0],6);  // Player erzeugen
			for(v = 0; v<400; v++)
					{
						SPISend8Bit(0x00);		// Farbe schwarz
					 }
	 
					 // Wände:
	 SendCommandSeq(&wall_1[0],6); 													// wall_1 erzeugen
			for(v = 0; v<800; v++)	{	SPISend8Bit(farbe_1); }
		
		SendCommandSeq(&wall_2[0],6);													// wall_2 erzeugen
			for(v = 0; v<825; v++)	{	SPISend8Bit(farbe_2); }
					
		SendCommandSeq(&wall_3[0],6); 												// wall_3 erzeugen
			for(v = 0; v<825; v++)	{	SPISend8Bit(farbe_3); }
					
		 SendCommandSeq(&wall_4[0],6); 												// wall_4 erzeugen
			for(v = 0; v<850; v++)	{	SPISend8Bit(farbe_4); }
							 
	 while (1) {;} 																					//Endlosschleife
}



		void TIMER16_0_IRQHandler(void){		  //Exception_Handler 

			 // button1
			button = LPC_GPIO2->DATA;		        // GPIO2DATA aufnehmen	
			button = (button & 0x200)>>9;				// Button1 maskieren PIO2_9
			
			//  button2
			button2 = LPC_GPIO1->DATA;
			button2 = (button2 & 0x10)>>4;			// Button2 maskieren PIO1_4
			
	

			// Button 2 nach links bewegen
			if (running > 0){														// Solange kein Crash...
			if (!button2) {
					button2_count++;
					}
						else 	{
						button2_count =0;
						}
						if (button2_count == 6){							// Nur wenn Button länger gedrückt wird - Entprellung
							SendCommandSeq(&player[0],6);				// Ursprungskaestchen uebermalen / verschwinden lassen
							for(v = 0; v<400; v++)
								{
									SPISend8Bit(0xFF);
								 }
								
							if (player[2]>0x1200){							// solange Player noch nicht am Rand
								player[2]--;       								// Player ein Pixel naeher an den Rand
								player[3]--; 
							}
									SendCommandSeq(&player[0],6);		// Neu positionierter Player
									for(v = 0; v<400; v++)
									{
										
										SPISend8Bit(0x00);
									}
								button2_count =0;
						}
			
		
		
		
		// Button 1
		if (!button) {
		button_count++;
		
		}
			else 	{
			button_count =0;
			}
			if (button_count == 6){					// Identisch mit Button2
			SendCommandSeq(&player[0],6);
			for(v = 0; v<400; v++)
					{
						SPISend8Bit(0xFF);
					 }
				if (player[2]<0x1270){				// Arraywerte werden erhöht, statt verringert
				player[2]++;       
				player[3]++; 
				}
			SendCommandSeq(&player[0],6);
			for(v = 0; v<400; v++)
					{
						SPISend8Bit(0x00);
					}
			
		button_count =0;
		}
			
	
		// Bewegung der Wände
		move_timer++;																// move Timer zählt durchgehend hoch
		
		if (move_timer == 5){ 											// Sobald Timer == 5
			
			
					// WALL 1
					SendCommandSeq(&wall_1[0],6);  				// wall_1 alte position löschen
					for(v = 0; v<800; v++)		    				// alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);									// mit Hintergrundfarbe füllen (löschen)
					 }
					 wall_1[4]--;													// Blockposition verschieben
					 wall_1[5]--;	
				  SendCommandSeq(&wall_1[0],6); 				// wall_1 an neuer Position malen
					for(v = 0; v<800; v++)
					{
						SPISend8Bit(farbe_1);
					}
				
			
					// WALL 2
					SendCommandSeq(&wall_2[0],6);  				// wall_2 alte position löschen
					for(v = 0; v<825; v++)		    				// alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);									// mit Hintergrundfarbe füllen ( löschen)
					 }
					 wall_2[4]--;													// Blockposition verschieben
					 wall_2[5]--;	
				  SendCommandSeq(&wall_2[0],6); 				// wall_2 an neuer Position malen
					for(v = 0; v<825; v++)
					{
						SPISend8Bit(farbe_2);
					}
				

					// WALL 3
					SendCommandSeq(&wall_3[0],6);  				// wall_3 alte position löschen
					for(v = 0; v<825; v++)		    				// alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);									// mit Hintergrundfarbe füllen (löschen)
					 }
					 wall_3[4]--;													// Blockposition verschieben
					 wall_3[5]--;	
				  SendCommandSeq(&wall_3[0],6); 				// wall_3 an neuer Position malen
					for(v = 0; v<825; v++)
					{
						SPISend8Bit(farbe_3);
					}
				

					// WALL 4
					SendCommandSeq(&wall_4[0],6);  				// wall_4 alte position löschen
					for(v = 0; v<850; v++)		    				// alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);									// mit Hintergrundfarbe füllen (löschen)
					 }
					 wall_4[4]--;													// Blockposition verschieben
					 wall_4[5]--;	
					 wallStep++;
				  SendCommandSeq(&wall_4[0],6); 				// wall_4 an neuer Position malen
					for(v = 0; v<850; v++)
					{
						SPISend8Bit(farbe_4);
					}					
				
					
					SendCommandSeq(&player[0],6); 				// player als letztes erzeugen, damit dieser nicht übermalt wird
						for(v = 0; v<400; v++)
					{
						SPISend8Bit(0x00);
					 }

					 
					move_timer = 0;											// den Move timer reseten, damit er wieder auf 5 zählen kann
					 
					Waitms(50);
				}
		
				
				
				// COLLISONSABFRAGE!
				if(rundenanzahl>0){    // Damit es nicht in der ersten "MatschRunde" freezed.
					if(guterBlock ==1 ){
						if(wallStep>115 && player[3] > wall_1[3]) {
							running=0;
						}
					}
					
						if(guterBlock ==4){
						if(wallStep>115 && player[2] < wall_4[2]) {
							running=0;
						}
					}
					
						if(guterBlock ==2){
						if((wallStep>115 && player[2] < wall_2[2]) || (wallStep>115 && player[3] > wall_2[3])) {
							running=0;
						}
					}
					
					if(guterBlock ==3){
						if((wallStep>115 && player[2] < wall_3[2]) || (wallStep>115 && player[3] > wall_3[3])) {
							running=0;
						}
					}
			}
				
				
				
				
				
					// Runden-Reset: Alle Walls neu positionieren
			
					if(wall_1[4]==0x1300){       				// Wenn Block1 oberen Rand erreicht
							running =0;								    	// Spiel einfrieren
							rundenanzahl++;						    	// Eine Runde zur RundenAnzahl hinzufügen
						
					//Alle 4 Wände weiß malen:
					

						// WALL 4
					SendCommandSeq(&wall_4[0],6);  			// wall_4 alte position löschen
					for(v = 0; v<850; v++)							// Alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// Mit Hintergrundfarbe füllen (löschen)
					 }
					
					 // WALL 3
					SendCommandSeq(&wall_3[0],6);  			// wall_3 alte position löschen
					for(v = 0; v<825; v++)							// Alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// Mit Hintergrundfarbe füllen (löschen)
					 }
					 
					 // WALL 2
					SendCommandSeq(&wall_2[0],6);  			// wall_1 alte position löschen
					for(v = 0; v<825; v++)							// Alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// Mit Hintergrundfarbe füllen (löschen)
					 }
					 
					 // WALL 1
					SendCommandSeq(&wall_1[0],6);  			// wall_1 alte position löschen
					for(v = 0; v<800; v++)							// Alle Pixel durchgehen...
					{
						SPISend8Bit(0xFF);								// Mit Hintergrundfarbe füllen (löschen)
					}
					
					SendCommandSeq(&player[0],6); 			// player nochmal malen, damit er vollständig angezeigt wird
						for(v = 0; v<400; v++)
					{
						
						SPISend8Bit(0x00);
					 }
					 
					 
							move_timer = 0;							// Timer move reset
						
							wall_1[2]=0x1200;						// Alle Wände an Anfangsposition
							wall_1[3]=0x151F;
							wall_1[4]=0x1397;
							wall_1[5]=0x16AF;
							
							wall_2[2]=0x1220;
							wall_2[3]=0x1540;
							wall_2[4]=0x1397;
							wall_2[5]=0x16AF;
					
						  wall_3[2]=0x1241;
							wall_3[3]=0x1561;
							wall_3[4]=0x1397;
							wall_3[5]=0x16AF;
						
						  wall_4[2]=0x1262;
							wall_4[3]=0x1583;
							wall_4[4]=0x1397;
							wall_4[5]=0x16AF;

							
							
							zufallszahl = rand() % 4 + 1;			// Zufällige Zahl auswählen
							
							
							if (zufallszahl == 1) {			// Ist Zufallszahl == 1
									farbe_1 = 0xFF;					// Farbe von wall_1 ist weiß (leer)
									guterBlock = 1;					// wall_1 ist guterBlock, also keine Kollision
							}
							else{
									farbe_1 = 0x00;					// Ansonsten ist Farbe von wall_1 schwarz
							}
							
																					
							if (zufallszahl == 2) {			// Identisch mit wall_1 (s.o.)
									farbe_2 = 0xFF;
									guterBlock = 2;
							}
							else{
									farbe_2 = 0x00;
							}
							
							if (zufallszahl == 3) {			// Identisch mit wall_1 (s.o.)
									farbe_3 = 0xFF;
									guterBlock = 3;
							}
							else{
									farbe_3 = 0x00;
							}
							
							if (zufallszahl == 4) {			// Identisch mit wall_1 (s.o.)
									farbe_4 = 0xFF;
									guterBlock = 4;
							}
							else{
									farbe_4 = 0x00;
							}
							
								
							Waitms(50);
							wallStep = 0;           // reset wallStepCounter
							running=1;	        		// auftauen
					}
				
	LPC_TMR16B0->IR = 0x1;		//IR quittieren für MR0, beenden des interrupts
		return;	
	}  
}