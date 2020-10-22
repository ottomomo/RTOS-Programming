
#include <s3c44b0x.h>
#include <s3cev40.h>
#include <pbs.h>
#include <timers.h>

extern void isr_PB_dummy( void );

void pbs_init( void )
{
    timers_init();
}

uint8 pb_scan( void ) //lee secuencialmente los pulsadores para determinar el código a devolver
{
	if( !(PDATG & (1<<6)))
		return PB_LEFT;
	else if( !(PDATG & (1 << 7)) )
		return PB_RIGHT;
	else
		return PB_FAILURE; //si ninguno esta pulsado devuelve fallo
}

uint8 pb_status( uint8 scancode )
{
    if(PB_RIGHT){
    	return !(PDATG & (1<<7));
    }else{
    	return !(PDATG & (1<<6));
    }
}

//**
void pb_wait_keydown( uint8 scancode )
{
	 while( 1 )
	     {
		  while(pb_scan()!=scancode);
	        sw_delay_ms( PB_KEYDOWN_DELAY );
	        if ( scancode == pb_scan() )
	            return;
		  while(!(PDATG & (1<<1)));
	        sw_delay_ms( PB_KEYUP_DELAY );
	    }
}

//**
void pb_wait_keyup( uint8 scancode )
{
	pb_wait_keydown(scancode);
	while(!(PDATG & (1<<1)));		//mientras sea =0 es decir ESTE PULSADA!!!!!!
	sw_delay_ms(PB_KEYUP_DELAY);
}

//**
//void pb_wait_any_keydown( void )
//{
//	while(1)
//		{
//			while(pb_scan()==PB_FAILURE);
//			sw_delay_ms( KEYPAD_KEYDOWN_DELAY );
//			if( pb_scan()!=PB_FAILURE )
//				return;
//			while( !(PDATG & (1<<1)) );
//			sw_delay_ms( PB_KEYUP_DELAY );
//		}
//}

//**
//void pb_wait_any_keyup( void )
//{
//	pb_wait_any_keydown();
//	while(!(PDATG & (1<<1)));		//mientras sea =0 es decir ESTE PULSADA!!!!!!
//	sw_delay_ms(PB_KEYUP_DELAY);
//}

/*
** Espera la presión y depresión de un pulsador y devuelve su scancode
*/
uint8 pb_getchar( void )
{
	uint8 scancode;
	 while( (PDATG & (1<<6)) != 0 && (PDATG & (1 << 7)) != 0 ); //espera la presion de cualquier pulsador
	 sw_delay_ms( PB_KEYDOWN_DELAY ); //espera SW, fin de rebotes
	 scancode = pb_scan(); //obtiene el codigo del pulsador presionado
	 while(!(PDATG & (1<<6)) || !(PDATG & (1 << 7)) ); //espera la depresion del pulsador
	 sw_delay_ms( PB_KEYUP_DELAY ); //espera SW, fin de rebotes
	return scancode;
}

/*
** Espera la presión y depresión de un pulsador y devuelve su scancode y el intervalo en ms que ha estado pulsado (max. 6553ms)
*/
uint8 pb_getchartime( uint16 *ms )
{
    uint8 scancode;

    while( (PDATG & (1<<6)) != 0 && (PDATG & (1 << 7)) != 0 ); //espera la presion de cualquier pulsador
    timer3_start(); //arranca el timer 3
    sw_delay_ms( PB_KEYDOWN_DELAY ); //espera SW(el timer 3 está ocupado) fin de rebotes

    scancode = pb_scan(); //obtiene el codigo del pulsador presionado

    while(!(PDATG & (1<<6)) || !(PDATG & (1 << 7)) ); //espera la depresion del pulsador
    *ms = timer3_stop() / 10; //detiene el timer 3 y calcula los ms
    sw_delay_ms( PB_KEYUP_DELAY ); //espera SW(el timer 3 está ocupado) fin de rebotes

    return scancode; //devuelve el código del pulsador presionado
}

//uint8 pb_timeout_getchar( uint16 ms )
//{
//    ...
//}

void pbs_open( void (*isr)(void) )
{
	pISR_PB = (uint32)isr;
	EXTINTPND = ~0;
	I_ISPC = BIT_EINT4567;
	INTMSK &= ~(BIT_GLOBAL | BIT_EINT4567);
}

void pbs_close( void )
{
	INTMSK |= (BIT_GLOBAL | BIT_EINT4567);
	pISR_PB = (uint32)isr_PB_dummy;
}
