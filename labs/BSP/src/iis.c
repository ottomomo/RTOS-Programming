
#include <s3c44b0x.h>
#include <s3cev40.h>
#include <iis.h>
#include <dma.h>

static void isr_bdma0( void ) __attribute__ ((interrupt ("IRQ")));

static uint8 flag;
static uint8 iomode;
static int8 reload;


/*
** Configura el controlador de IIS según los siguientes parámetros
**   Master mode en reposo (no transfer y todo desabilitado)
**   fs: 16000 KHz
**   CODECLK: 256fs
**   SDCLK: 32fs
**   Bits por canal: 16
**   Protocolo de trasmisión de audio: iis
**   Señalizacion de canal izquierdo: a baja
** Si mode = IIS_DMA
**   No transfer mode
**   Deshabilita Tx/Rx FIFOs
**   Deshabilita prescaler e IIS
**   Inicializa el BDMA0
**   Abre las interrupciones por BDMA0
** Si mode = IIS_POLLING
**   Transmit and receibe mode
**   Tx/Rx por pooling
**   Habilita Tx/Rx FIFOs
**   Habilita prescaler e IIS
*/
void iis_init( uint8 mode )
{
    iomode = mode;

    if( mode == IIS_POLLING )
    {
        IISPSR  = ((7<<0) | (7<<4));	//fs: 16000 KHz
        IISMOD = 0xC9;	// [011001001] CODECLK: 256fs, SDCLK: 32fs, Bits por canal: 16, master mode, modo transmision y recepccion
        IISFCON = (0x3<<8);
        IISCON = ((3<<0) | (0x0<<2));
    }
    if( mode == IIS_DMA )
    {
        IISPSR  = ((7<<0) | (7<<4));	//fs: 16000 KHz
        IISMOD  = ((0x09) | (0<<8));// [000001001] CODECLK: 256fs, SDCLK: 32fs, Bits por canal: 16, master mode, modo NO TRANSFER
        IISFCON = 0;
        IISCON  = 0;
        bdma0_init();
        bdma0_open( isr_bdma0 );
        flag = OFF;
    }
}

/* Interrupcion de la DMA0, cunado termina de transmitir/recibir. Pone el FLAG=OFF indicando que ha terminado.*/
static void isr_bdma0( void )
{
	if(reload==0){
    	IISCON &= ~1;
    	flag = OFF;
	}
	I_ISPC = BIT_BDMA0;
}

/*
** Envía por el bus IIS una muestra por pooling
*/
inline void iis_putSample( int16 ch0, int16 ch1 )
{
    while( ((IISFCON>>4) & 0xf) > 6);/*Esperar a que haya al menos 2 huecos en la FIFO Tx (mientras IISFCON[7:4] > 6)*/
    IISFIF = ch0;
    IISFIF = ch1;
}

/*
** Almacena por pooling una muestra recibida por el bus IIS
*/
inline void iis_getSample( int16 *ch0, int16 *ch1 )
{
	while(((IISFCON) & 0xf) < 2);/*Esperar a que haya al menos 2 huecos en la FIFO Tx (mientras IISFCON[3:0] < 2)*/
	*ch0=IISFIF;
	*ch1=IISFIF;
}

void iis_play( int16 *buffer, uint32 length, uint8 loop )
{
    uint32 i;
    int16 ch1, ch2;

    if( iomode == IIS_POLLING )
        for( i=0; i<length/2; )
        {
            ch1 = buffer[i++];
            ch2 = buffer[i++];
            iis_putSample( ch1, ch2 );
        }
    if( iomode == IIS_DMA )
    {
    	while( flag != OFF );			//si esta recibiendo/transmitiendo, espera...
		BDISRC0 = (1 << 30) | (1 << 28) | ((uint32) buffer & 0xFFFFFFF);
		BDIDES0 = (1 << 30) | (3 << 28) | ((uint32) &IISFIF & 0xFFFFFFF);
		BDCON0 = 0;
		BDICNT0 = (1 << 30) | (1 << 26) | (3 << 22) | ((loop & 0x1) << 21) |(0xfffff & length);/* conexion DMA con IIS, generacion de int al final...*/
		BDICNT0 |= (1 << 20);

		reload=loop;
		IISMOD = 0x89; 				/* modo TRANSFER*/
		IISFCON = ((0xA00) & 0xfff); /*DMA access mode*/
		IISCON = (0x23 & 0x3f);
		flag = ON;						//Transmitiendo
    }
}

void iis_rec( int16 *buffer, uint32 length )
{
    uint32 i;
    int16 ch1, ch2;

    if( iomode == IIS_POLLING )
    	for( i=0; i<length/2; )
    	{
			iis_getSample(&ch1, &ch2);
			buffer[i++]=ch1;
			buffer[i++]=ch2;
		}
    if( iomode == IIS_DMA )
    {
        while( flag != OFF );			//si esta recibiendo/transmitiendo, espera...
        BDISRC0  = (1 << 30) | (3 << 28) | ((uint32) &IISFIF & 0xFFFFFFF);
        BDIDES0  = (2 << 30) | (1 << 28) | ((uint32) buffer & 0xFFFFFFF);
        BDCON0   = 0;
        BDICNT0  = (1 << 30) | (1 << 26) | (3 << 22) | (0<<21) | (0xfffff & length); /* conexion DMA con IIS, generacion de int al final...*/
        BDICNT0 |= (1 << 20);

        reload=0;
        IISMOD  = 0x49;				/* modo recepcion*/
        IISFCON = ((0x500)& 0xfff);	/*DMA access mode*/
        IISCON  = ((0x13) & 0x1f);
        flag = ON;
    }
}

void iis_pause( void )
{
	IISCON &= ~1;
    flag = OFF;
}

void iis_continue( void )
{
	 IISCON |=(1<<0);
	 flag = ON;
}

uint8 iis_status( void )
{

    if(flag==ON){
    	return ON;
    }else{
    	return OFF;
    }
}


void iis_playwawFile( uint8 *fileAddr )
{
    uint32 size;

    while ( !(fileAddr[0] == 'd' && fileAddr[1] == 'a' && fileAddr[2] == 't' && fileAddr[3] == 'a') )/*Busca el identificador del chunk*/
        fileAddr++;
    fileAddr += 4;	/*Salta el id del chunk*/

    size = (uint32) fileAddr[0];				/*Extrae el numero de Bytes*/
    size += (uint32) fileAddr[1] << 8;
    size += (uint32) fileAddr[2] << 16;
    size += (uint32) fileAddr[3] << 24;
    fileAddr += 4;

    iis_play( (int16 *)fileAddr, size, OFF );
}

