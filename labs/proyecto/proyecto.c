#include <s3c44b0x.h>
#include <s3cev40.h>
#include <system.h>
#include <uart.h>
#include <timers.h>
#include <pbs.h>
#include <lcd.h>
#include <uda1341ts.h>
#include <iis.h>
#include <keypad.h>

/* Macros */

#define SPS      (16000)                  // Frecuencia de muestreo
#define BUFFER   ((int16 *)0x0c200000)    // Dirección de inicio del buffer      
#define MAXBYTES (640000)                 // Capacidad máxima del buffer = 10 s : (10 s) * (16000 muestras/s) * (2 canales) * (2 B/muestra) = 640000B 
#define MAXRECORDS (10)					  //Capacidad maxima de grabaciones de un bloque
#define MAXBLOCKS  (3)					  //Capacidad maxima de bloques del sistema
#define true		(1)
#define false		(0)
/* Definición de tipos */

typedef enum {
	waiting, recording, playing
} state_t;    // Estados en los que puede estar el sistema

typedef struct {
	uint16 id;
	int16 *record;	// Puntero al inicio de la grabacion
	uint32 size;    // Numero de muestras grabadas
	uint32 index;   // Numero de muestras reproducidas
	uint16 full; 	// Flag para indicar si la grabacion ha sido completamente grabado
	uint16 played; 	// Flag para indicar si la grabacion ha sido completamente reproducido
} record_t;

typedef struct {
	uint16 id;
	int16 *buffer;    				// Puntero al inicio del buffer de muestras
	uint32 maxsize; 				// Tamaño máximo del buffer (en numero de muestras, 1 muestra = 2B)
	uint32 offset;
	uint16 full; 					// Flag para indicar si el buffer ha sido completamente grabado
	uint16 played; 					// Flag para indicar si el buffer ha sido completamente reproducido
	record_t records[MAXRECORDS]; 	//grabaciones contenidas en el buffer
	uint16 numRecs;					// numero de grabaciones del buffer
	uint16 free; 					// Flag para indicar si el buffer esta vacio
} audioBuffer_t;

typedef struct {
	volatile uint16 numSecs; 	// Numero de segundos transcurridos desde el inicio de la grabación/reproducción
	volatile boolean updated; 	// Flag para indicar si numSeconds se ha actualizado
} time_t;

/* Declaración de funciones */

void timer0_tick(void) __attribute__ ((interrupt ("IRQ")));
static void keypad_ISR( void ) __attribute__ ((interrupt ("IRQ")));

/* Declaración de variables globales para comunicación foreground/background  */

state_t state;				//estado actual del sistema
volatile uint16 currentB;	//indice del bloque actual
volatile uint16 key;		//tecla del keypad seleccionada
volatile audioBuffer_t audioBlock[MAXBLOCKS];// superBloque
volatile time_t time;
volatile uint16 playList[MAXRECORDS];	//array de Id's de records
volatile uint16 totalRecs;	//numero total de grabaciones

/************************************************FOREGROUND**********************************************************/
/********************************************************************************************************************/
void main(void) {
	sys_init();         //configuracion arm
	timers_init();		//
	pbs_init();
	keypad_init();
	lcd_init();
	uda1341ts_init();
	iis_init(IIS_POLLING);
	lcd_on();
	lcd_clear();
	totalRecs=0;
	currentB=-1;         	//indice del bloque actual
	key = KEYPAD_FAILURE;	//inicializamos a false
	audioBlock_init();

	while (1) {
		/**************** Menu principal de opciones ********************/

			state = waiting;
			keypad_open( keypad_ISR );
			lcd_puts_x2(0, 50, BLACK, "  Menu      ");
			lcd_puts(0, 82, BLACK, "   0. Grabar.    ");
			lcd_puts(0, 102, BLACK, "   1. Lista de reproduccion.    ");
			lcd_puts(0, 122, BLACK, "   2. Salir.   ");
			//sleep();
			keypad_wait_any_keydown();  //esperamos a una pulsacion de una tecla
			//lcd_puts_x2( 0, 50, BLACK, "  Esperando...      " );
			//lcd_puts( 0, 82, BLACK, "    pulse para empezar la grabacion   " );
			//pb_wait_keyup( PB_LEFT );

			/*
			 * FUNCIONES dentro de nuestras opciones
			 op1:
			 -grabar
			 op2:
			 -reproducir pista()
			 play, pause, stop, speed
			 -speed (adelante y atrás)
			 -combinar pistas de audio
			 op3:
			 */

			/* Inicio del anidamiento (ejemplo NO FUNCIONA) */

			switch (key) {
			//GRABAR:
			case KEYPAD_KEY0:
				state = recording; // empezamos a recorrer
				if(audioBlock[currentB].full== true){
					audioBlock_init();
					record_init();
				}else{
					record_init();
				}
				time.numSecs = 0;
				time.updated = FALSE;

				lcd_clear();
				lcd_puts_x2(0, 50, BLACK, "  GRABANDO:0        ");
				lcd_puts(0, 82, BLACK, "    pulse para parar la grabacion     ");
				timer0_open_tick(timer0_tick, SPS); 								// Inicia la generación de interrupciones periodicas para muestreo del audio codec
				while (!pb_status(PB_LEFT) && !audioBlock[currentB].full) 			// Espera la presión de un pulsador o el llenado completo del buffer de audio
					if (time.updated) 												// Si ha transcurrido un segundo de grabación...
					{
						lcd_puts_x2(176, 50, BLACK, "  "); 							// ...actualiza en pantalla la cuenta de segundos
						lcd_putint_x2(176, 50, BLACK, time.numSecs);
						time.updated = FALSE;
					}
				timer0_close(); 				// Finaliza la generación de interrupciones periódicas
				if (pb_status(PB_LEFT)) 		// Si finalizó la toma de muestras por presión de un pulsador...
					pb_wait_keyup(PB_LEFT);
				lcd_clear();
				break;

			//LISTA DE REPRODUCCION
			case KEYPAD_KEY1:
				lcd_clear();
				int row = 50;
				int i;
				for (i = 0; i < totalRecs; i++) {
					//lcd_puts_x2(0, 35, BLACK, "  LISTA DE REPRODUCCION   ");
					int v= playList[i];
					lcd_puts(0, row, BLACK, "Pista");
					lcd_putint(40, row, BLACK, v);
					row += 20;
				}
				keypad_wait_any_keydown();
				keypad_close();
				//key = keypad_scan();
				//sleep();
				//pb_wait_keyup( PB_LEFT );
				state = playing;

				time.numSecs = 0;
				time.updated = FALSE;
				lcd_clear();
				lcd_puts_x2(0, 50, BLACK, "  REPRODUCIENDO:0   ");
				lcd_puts(0, 82, BLACK,"    pulse para parar la reproduccion  ");
				timer0_open_tick(timer0_tick, SPS); // Inicia la generación de interrupciones periodicas para muestreo del audio codec
				while (!pb_status(PB_LEFT) && !audioBlock[currentB].records[key].played) // Espera la presión de un pulsador o la reproducción completa del buffer de audio
					if (time.updated) // Si ha transcurrido un segundo de reproducción...
					{
						lcd_puts_x2(256, 50, BLACK, "  "); // ...actualiza en pantalla la cuenta de segundos
						lcd_putint_x2(256, 50, BLACK, time.numSecs);
						time.updated = FALSE;
					}
				timer0_close(); // Finaliza la generación de interrupciones periódicas
				audioBlock[currentB].records[key].played=false;
				keypad_open(keypad_ISR);
				if (pb_status(PB_LEFT)) // Si finalizó la reproducción de muestras por presión de un pulsador...
					pb_wait_keyup(PB_LEFT);            // ...espera su depresión
				lcd_clear();
				break;

			//FIN
			case KEYPAD_KEY2:

				break;
			default:
				break;
			}
	}/* Fin del anidamiento */
}

/***************************************************************************************************************************/
/*********************************************RUTINAS DE TRATAMIENTO DE EXCEPCIONES*****************************************/
/***************************************************************************************************************************/

void timer0_tick(void) {
	static uint16 numTicks = SPS;
	uint16 i =audioBlock[currentB].numRecs -1;
	int* record = audioBlock[currentB].records[i].record;
	int16 ch0, ch1;

	switch (state) {
	case waiting:
		break;
	case recording:

		if (audioBlock[currentB].records[i].size < audioBlock[currentB].maxsize) { // Si el sistema esta grabando sonido y el buffer de audio no esta completo...
			iis_getSample(&ch0, &ch1);      // ...recibe una muestra por canal y
			//iis_rec(&record, MAXBYTES/2);
			//audioBlock[currentB].records[i].record[audioBlock[currentB].records[i].size++] = ch0; // ...las almacena en el audio buffer
			record[audioBlock[currentB].records[i].size++]= ch0;
			//audioBlock[currentB].records[i].record[audioBlock[currentB].records[i].size++] = ch0; // ...las almacena en el audio buffer
			record[audioBlock[currentB].records[i].size++]= ch0;
			audioBlock[currentB].offset++;
			audioBlock[currentB].offset++;
			if (!--numTicks) { // Decrementa el contador de ticks, y si llega a cero...
				numTicks = SPS;
				time.numSecs++; // ...actualiza y señaliza el número de segundos de grabación
				time.updated = TRUE;
			}
		} else {
			audioBlock[currentB].full = TRUE; // Si el buffer esta completo lo señaliza
			numTicks = SPS;
		}
		break;
	case playing:
		if (audioBlock[currentB].records[key].index < audioBlock[currentB].records[key].size) { // Si el sistema esta reproducciendo sonido y quedan muestras por enviar...
			ch0 = audioBlock[currentB].records[key].record[audioBlock[currentB].records[key].index++];
			//ch0 = record[audioBlock[currentB].records[i].index++]; // ...lee las muestras del audio buffer y
			ch1 = audioBlock[currentB].records[key].record[audioBlock[currentB].records[key].index++];
			//ch1 = record[audioBlock[currentB].records[i].index++];
			iis_putSample(ch0, ch1);                             // ...las envia
			if (!--numTicks) { // Decrementa el contador de ticks, y si llega a cero...
				numTicks = SPS;
				time.numSecs++; // ...actualiza y señaliza el número de segundos de reproducci´pn
				time.updated = TRUE;
			}
			break;
		} else {
			audioBlock[currentB].records[key].played = TRUE; // Si se ha finalizado la reproducción de la grabacion lo señaliza
			numTicks = SPS;
			audioBlock[currentB].records[key].index=0;
		}

	}
	I_ISPC = BIT_TIMER0;
}

void keypad_ISR( void ) {
	key = keypad_scan();
	while( !(PDATG & (1<<1)));
	sw_delay_ms(KEYPAD_KEYUP_DELAY);
	I_ISPC = BIT_KEYPAD;
}

/**************************************************************************************************************************/
/********************************************INICIALIZACION DEL GESTOR DE DATOS********************************************/
/**************************************************************************************************************************/

void audioBlock_init() {
	currentB++;

	audioBlock[currentB].buffer = BUFFER + ((MAXBYTES/4) * currentB);//(sizeof(audioBuffer_t) * currentB);
	audioBlock[currentB].maxsize = MAXBYTES / 2;
	audioBlock[currentB].offset = 0;
	audioBlock[currentB].full = FALSE;
	audioBlock[currentB].played = FALSE;
	audioBlock[currentB].numRecs=0;
	audioBlock[currentB].id=currentB;
	audioBlock[currentB].free=false;
}

void record_init() {
	uint16 i =audioBlock[currentB].numRecs;
	audioBlock[currentB].numRecs++;

	audioBlock[currentB].records[i].record = ((audioBlock[currentB].buffer) + (audioBlock[currentB].offset)); /*(2 B/muestra)*/
	audioBlock[currentB].records[i].id = (currentB*10)+i;
	audioBlock[currentB].records[i].size=0;
	audioBlock[currentB].records[i].index=0;
	audioBlock[currentB].records[i].full=FALSE;
	audioBlock[currentB].records[i].played=FALSE;

	playList[totalRecs]=audioBlock[currentB].records[i].id;
	totalRecs++;
}
/**************************************************************************************************************************/
