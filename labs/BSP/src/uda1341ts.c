
#include <l3.h>
#include <uda1341ts.h>

#define ADDRESS (0x05)

#define DATA0  (0x0)
#define DATA1  (0x1)	/*Nemotécnicos para identificar los registros primarios*/
#define STATUS (0x2)

#define EA (0x18 << 3)		/*Nemotécnicos para los prefijos de acceso*/
#define ED (0x7 << 5)		/*a los registros secundarios			*/

static uint8 volume;
static uint8 state;

/*
** Inicializa el interfaz L3
** Resetea el audio codec
** Configura el audio codec según los siguientes parámetros:
**   CODECLK = 256fs
**   Protocolo de trasmisión de audio: iis
**   Volumen de reproducción máximo
**   Selecciona el canal 1 como entrada
**   Habilita el ADC y DAC con 6 dB de ganancia de entrada
**   Fija el volumen máximo
*/
void uda1341ts_init( void )
{
    L3_init();     
    state=0;
    L3_putByte( (ADDRESS << 2) | STATUS, L3_ADDR_MODE );		/*Resetea el Audio Codec*/
    L3_putByte( (1 << 6) | (2 << 4), L3_DATA_MODE );			/*Resetea el Audio Codec*/
    L3_putByte( (2 << 4), L3_DATA_MODE );						/*Quita el reset y fija frecuencia
																y formato de IIS*/
    L3_putByte( (ADDRESS << 2) | DATA0, L3_ADDR_MODE  ); 		/*Selecciona el canal 1*/
    L3_putByte( EA | (2), L3_DATA_MODE ); 						/*Selecciona el canal 1*/
    L3_putByte( ED | 1, L3_DATA_MODE );							/*Selecciona el canal 1*/
  
    uda1341ts_setvol( VOL_MAX );

    uda1341ts_on( UDA_DAC );
    uda1341ts_on( UDA_ADC );
}

/*
** Habilita/desabilita el silenciado del audio codec
** activar/desactivar mute  DATA0[7:6] = 2 y DATA0[2]=1/0 = MUTE
*/
void uda1341ts_mute( uint8 on )
{
	/* PLANTILLA DE REG SECUNDARIO
	 * Escribir en registro secundario:
	 * direccion del modo, 1er.Byte=(dir dispositivo + reg DATA0)*/
	//L3_putByte( (ADDRESS << 2) | DATA0, L3_ADDR_MODE  );
	/* identificador del reg secundario, 2do.Byte=(prefijo + reg secundario)*/
	//L3_putByte( EA | (4), L3_DATA_MODE );
	/* dato, 3er.Byte=(prefijo + dato)*/

	/* Escribir en registro PRIMARIO:
	 * modo direccion, 1er.Byte=(dir dispositivo + reg DATA0)*/
	L3_putByte( (ADDRESS << 2) | DATA0, L3_ADDR_MODE  );
	/* modo dato, 2do.Byte=(dato(8 bits))*/
	if(on==MUTE_OFF){
		L3_putByte( ((2<<6)| (0<<2)), L3_DATA_MODE );
	}else{
		L3_putByte( ((2<<6)| (1<<2)), L3_DATA_MODE );
	}
};
/*
** Enciende el conversor indicado
*/
void uda1341ts_on( uint8 converter )
{
	state |=converter;
	/* Escribir en registro PRIMARIO:
	 * modo direccion, 1er.Byte=(dir dispositivo + reg DATA0)*/
	L3_putByte( (ADDRESS << 2) | STATUS, L3_ADDR_MODE  );
	/* modo dato, 2do.Byte=(dato(8 bits))*/
	L3_putByte( ((1<<7) | (1<<5) | (state & 0x3)), L3_DATA_MODE );// 6dB ganancia
}

void uda1341ts_off( uint8 converter )
{
	state &= ~(converter);
    /* Escribir en registro PRIMARIO:
	 * modo direccion, 1er.Byte=(dir dispositivo + reg DATA0)*/
	L3_putByte( (ADDRESS << 2) | STATUS, L3_ADDR_MODE  );
	/* modo dato, 2do.Byte=(dato(8 bits))*/
	L3_putByte( ((1<<7) | (1<<5) | (state)), L3_DATA_MODE );// 6dB ganancia
}

uint8 uda1341ts_status( uint8 converter )
{
	if(converter==UDA_DAC){
		return state & 0x1;
	}else{
		return state & 0x2;
	}
}

void uda1341ts_setvol( uint8 vol )
{
	volume=vol;
	/* Escribir en registro PRIMARIO:
	 * modo direccion, 1er.Byte=(dir dispositivo + reg DATA0)*/
	L3_putByte((ADDRESS << 2) | DATA0, L3_ADDR_MODE);
	/* modo dato, 2do.Byte=(dato(8 bits))*/
	L3_putByte((00<<6) | ((vol- 0x3f) & 0x3f), L3_DATA_MODE);

};

uint8 uda1341ts_getvol( void )
{
    return volume;
};

