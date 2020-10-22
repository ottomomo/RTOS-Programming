/*-------------------------------------------------------------------
**
**  Fichero:
**    lab7.c  5/3/2015
**
**    (c) J.M. Mendias
**    Programación de Sistemas y Dispositivos
**    Facultad de Informática. Universidad Complutense de Madrid
**
**  Propósito:
**    Test del laboratorio 7
**
**  Notas de diseño:
**
**-----------------------------------------------------------------*/

#include <s3c44b0x.h>
#include <common_types.h>
#include <system.h>
#include <keypad.h>
#include <lcd.h>

/*
** Direcciones en donde se encuentran cargados los BMP tras la ejecucion en
** la consola del depurador los comandos:
**   cd <ruta>
**   source load_bmp.txt
*/

#define ARBOL      ((uint8 *)0x0c210000)
#define PADRINO    ((uint8 *)0x0c220000)
#define PICACHU    ((uint8 *)0x0c230000)
#define HARRY      ((uint8 *)0x0c240000)
#define CHAPLIN    ((uint8 *)0x0c250000)
#define PULP       ((uint8 *)0x0c260000)
#define MAPA       ((uint8 *)0x0c270000)

void main( void )
{
    sys_init();
    keypad_init();
    lcd_init();
    
    lcd_clear();
    lcd_on();
    
    while( 1 )
    {
        /************************************/

        lcd_draw_box( 10, 10, 310, 230, BLACK, 5 );

        /************************************/

        lcd_puts( 20, 16, BLACK, "ABCDEFGHIJKLMNÑOPQRSTUVWXYZ" );
        lcd_puts( 20, 32, BLACK, "abcdefghijklmnñopqrstuvwxyz" );
        lcd_puts( 20, 48, BLACK, "0123456789!$%&/()=^*+{}-.,;: " );
        lcd_putint( 20, 64, BLACK, 1234567890 );
        lcd_puthex( 108, 64, BLACK, 0xabcdef );

        /************************************/

        lcd_puts_x2( 20, 80, BLACK, "ABCDEFGHIJKLMNÑOP" );
        lcd_puts_x2( 20, 112, BLACK, "abcdefghijklmnñop" );
        lcd_puts_x2( 20, 144, BLACK, "0123456789!$%&/()" );
        lcd_putint_x2( 20, 176, BLACK, 1234567890 );
        lcd_puthex_x2( 196, 176, BLACK, 0xabcdef );

        /************************************/

        keypad_wait_any_keyup();
        lcd_putWallpaper( ARBOL );
        keypad_wait_any_keyup();
        lcd_putWallpaper( PADRINO );
        keypad_wait_any_keyup();
        lcd_putWallpaper( PICACHU );
        keypad_wait_any_keyup();
        lcd_putWallpaper( HARRY );
        keypad_wait_any_keyup();
        lcd_putWallpaper( CHAPLIN );
        keypad_wait_any_keyup();
        lcd_putWallpaper( PULP );
        keypad_wait_any_keyup();
        lcd_putWallpaper( MAPA );

        /************************************/

        keypad_wait_any_keyup();
        lcd_clear();
    }
}
