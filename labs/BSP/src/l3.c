
#include <s3c44b0x.h>
#include <l3.h>
#include <leds.h>

#define SHORT_DELAY    { int8 j; for( j=0; j<4; j++ ); }

/*Deja en reposo el bus L3: L3CLOCK=1 y L3MODE=1*/
void L3_init( void )
{
	 uint8 rled, lled;

	rled = !led_status(RIGHT_LED);
	lled = !led_status(LEFT_LED);
	/* PDATB[10] led derecho
	 * PDATB[9] led izquierdo
	 * PDATB[5] L3CLOCK
	 * PDATB[4] L3MODE
	 * */
	PDATB = (rled << 10) | (lled << 9) | (1 << 5) | (1 << 4);
}

/*
** Envía un byte por el interfaz L3 en el modo (ADDR/DATA) indicado
*/
void L3_putByte( uint8 byte, uint8 mode )
{
    uint8 i;
    uint8 rled, lled;
    
    rled = !led_status( RIGHT_LED );
    lled = !led_status( LEFT_LED );    
   
    PDATB = (1 << 5) | (mode << 4);/* Modo transmision*/
    SHORT_DELAY;

    for( i=0; i<8; i++ )
    {
        PDATB = (0 << 5) | (mode << 4);	//Baja la señal de reloj
        PDATA = (((byte>>i)& 0x1) << 9);	//pone el bit a transmitir L3DATA = byte[i]
        SHORT_DELAY;    				// espera ciclo de reoj
        PDATB = (1 << 5) | (mode << 4);	//Sube señal de reloj
        SHORT_DELAY;					// espera ciclo de reloj
    }
    PDATB = (rled << 10) | (lled << 9) | (1 << 5) | (1 << 4);   //deja los leds inalterados, BUS L3 en modo reposo.
}

