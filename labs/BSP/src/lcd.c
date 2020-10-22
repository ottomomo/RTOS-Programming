#include <s3c44b0x.h>
#include <lcd.h>

extern uint8 font[];
static uint8 lcd_buffer[LCD_BUFFER_SIZE];

static uint8 state;

void lcd_init(void) {
	DITHMODE = 0x12210;
	DP1_2 = 0xa5a5;
	DP4_7 = 0xba5da65;
	DP3_5 = 0xa5a5f;
	DP2_3 = 0xd6b;
	DP5_7 = 0xeb7b5ed;
	DP3_4 = 0x7dbe;
	DP4_5 = 0x7ebdf;
	DP6_7 = 0x7fdfbfe;

	REDLUT = 0x0;
	GREENLUT = 0x0;
	BLUELUT = 0x0;

	LCDCON1 = 0x1c020;
	LCDCON2 = 0x13cef;
	LCDCON3 = 0x0;

	LCDSADDR1 = (2 << 27) | ((uint32) lcd_buffer >> 1);
	LCDSADDR2 = (1 << 29)
			| (((uint32) lcd_buffer + LCD_BUFFER_SIZE)& 0x3FFFFF)>>1;
	LCDSADDR3 = 0x50;

	lcd_off();
}

void lcd_on(void) {
	LCDCON1 |= 0x1;
	state = ON;
}

void lcd_off(void) {
//	LCDCON1 &= 0x0;
//	state = OFF;
}

uint8 lcd_status(void) {
return state;
}

void lcd_clear(void) {
uint32 i, j;
for (i = 0; i < LCD_WIDTH; i++) {
	for (j = 0; j < LCD_HEIGHT; j++) {
		lcd_putpixel(i, j, WHITE);
	}
}
}

void lcd_putpixel(uint16 x, uint16 y, uint8 c) {
uint8 byte, bit;
uint16 i;

i = x / 2 + y * (LCD_WIDTH / 2);//calcula la posicion que ocupa el pixel en el array
bit = (1 - x % 2) * 4;//calcula la pos que ocupa el pixel en el Byte

byte = lcd_buffer[i];// lee el Byte que contiene el pixel
byte &= ~(0xF << bit);//borra el nibble correspondiente al pixel
byte |= c << bit;//escribe el nibble correspondiente al pixel
lcd_buffer[i] = byte;//escribe el Byte modificado que contiene el pixel
}

uint8 lcd_getpixel(uint16 x, uint16 y) {
	uint8 byte;
	byte = lcd_buffer[x / 2 + y * (LCD_WIDTH / 2)];
return byte;
}
/*
 ** Dibuja una línea horizontal desde el pixel (xleft,y) hasta el pixel (xright,y) del color y grosor indicados
 */
void lcd_draw_hline(uint16 xleft, uint16 xright, uint16 y, uint8 color,uint16 width) {
	uint16 row, line;

	for (line = 0; line < width; line++) {
		for (row = xleft; row <= xright; row++) {
			lcd_putpixel(row, y+line, color);
		}
	}
}

//Dibuja una línea vertical desde el pixel (x,yup) hasta el pixel (x,ydown) del color y grosor indicados
void lcd_draw_vline(uint16 yup, uint16 ydown, uint16 x, uint8 color, uint16 width) {

	uint16 row, line;

	for (line = 0; line < width; line++) {
		for (row = yup; row <= ydown; row++) {
			lcd_putpixel(x+line, row,  color);
		}
	}
}

void lcd_draw_box(uint16 xleft, uint16 yup, uint16 xright, uint16 ydown,
	uint8 color, uint16 width) {
lcd_draw_hline(xleft, xright, yup, color, width);
lcd_draw_vline(yup, ydown, xleft, color, width);
lcd_draw_hline(xleft, xright, ydown, color, width);
lcd_draw_vline(yup, ydown, xright, color, width);
}

void lcd_putchar(uint16 x, uint16 y, uint8 color, char ch) {
uint8 line, row;
uint8 *bitmap;

bitmap = font + ch * 16;
for (line = 0; line < 16; line++)
	for (row = 0; row < 8; row++)
		if (bitmap[line] & (0x80 >> row))
			lcd_putpixel(x + row, y + line, color);
		else
			lcd_putpixel(x + row, y + line, WHITE);
}

void lcd_puts(uint16 x, uint16 y, uint8 color, char *s) {
uint8 i = 0;
while (s[i] != '\0') {
	lcd_putchar(x, y, color, s[i]);
	i++;
	x += 8;
}
}

void lcd_putint(uint16 x, uint16 y, uint8 color, int32 i) {
	uint16 aux = x;
	char buf[8 + 1];
	char *p = buf + 8;
	uint8 c;
	*p = '\0';
	if (i < 0) { //SI EL NUMERO ES NEGATIVO HAY QUE PINTAR EL GUION Y AVANZAR X PARA QUE NO SE PINTE EL NUMERO ENCIMA DEL GUION
		lcd_putchar(x, y, color, '-');
		i = i * -1;
		aux = aux + 8;
	}

	//SE ALMACENA EL NUMERO EN UNA CADENA Y SE MUESTRA CON PUTS
	do {
		c = i % 10;
		*--p = '0' + c;
		i = i / 10;
	} while (i);

	lcd_puts(aux, y, color, p);
}

void lcd_puthex(uint16 x, uint16 y, uint8 color, uint32 i) {
	char buf[8 + 1];
	char *p = buf + 8;
	uint8 c;
	*p = '\0'; //Fin de cadena
	do {
		c = i & 0xf; // Resto de la division por 16
		if (c < 10)
			*--p = '0' + c; // Almacenaje del caracter
		else
			*--p = 'a' + c - 10;
		i = i >> 4; //Division por 16
	} while (i);
	lcd_puts(x, y, color, p); //Enviar cadena
}

void lcd_putchar_x2(uint16 x, uint16 y, uint8 color, char ch) {
	uint8 line, row;
		uint8 *bitmap;
		bitmap = font + ch * 16;
		for (line = 0; line < 16; line++){

			for (row = 0; row < 8; row++){
				if (bitmap[line] & (0x80 >> row)){
					lcd_putpixel(x + 2*row,  y + 2*line, color);
					lcd_putpixel(x + 2*row,y + 2*line+1, color);
					lcd_putpixel(x + 2*row+1,  y + 2*line, color);
					lcd_putpixel(x + 2*row+1,y + 2*line+1, color);
				}
				else{
					lcd_putpixel(x + 2*row,   y + 2*line, WHITE);
					lcd_putpixel(x + 2*row, y + 2*line+1, WHITE);
					lcd_putpixel(x + 2*row+1,   y + 2*line, WHITE);
					lcd_putpixel(x + 2*row+1, y + 2*line+1, WHITE);
				}
			}
		}
}

void lcd_puts_x2(uint16 x, uint16 y, uint8 color, char *s) {
	uint16 aux=x;
		uint8 i = 0;
		while (s[i] != '\0') {
			lcd_putchar_x2(aux, y, color, s[i]);
			i++;
			aux = aux + 16;
		}
}

void lcd_putint_x2(uint16 x, uint16 y, uint8 color, int32 i) {
	uint16 aux=x;
		char buf[8 + 1];
		char *p = buf + 8;
		uint8 c;
		*p = '\0';
		if (i < 0) {
			lcd_putchar_x2(x, y, color, '-');
			i = i * -1;
			aux =aux+16; // para que no se pise el guión
		}

		do {
			c = i % 10;
			*--p = '0' + c; // Almacenaje del caracter
			i = i / 10;
		} while (i);

		lcd_puts_x2(aux, y, color, p);
}

void lcd_puthex_x2(uint16 x, uint16 y, uint8 color, uint32 i) {
	char buf[8 + 1];
		char *p = buf + 8;
		uint8 c;
		*p = '\0'; //Fin de cadena
		do {
			c = i & 0xf; // Resto de la division por 16
			if (c < 10)
				*--p = '0' + c; // Almacenaje del caracter
			else
				*--p = 'a' + c - 10;
			i = i >> 4; //Division por 16
		} while (i);
		lcd_puts_x2(x, y, color, p); //Enviar cadena
}

void lcd_putWallpaper(uint8 *bmp) {
	uint32 headerSize;

	uint16 x, ySrc, yDst;
	uint16 offsetSrc, offsetDst;

	headerSize = bmp[10] + (bmp[11] << 8) + (bmp[12] << 16) + (bmp[13] << 24);

	bmp = bmp + headerSize;

	for (ySrc = 0, yDst = LCD_HEIGHT - 1; ySrc < LCD_HEIGHT; ySrc++, yDst--) {
		offsetDst = yDst * LCD_WIDTH / 2;
		offsetSrc = ySrc * LCD_WIDTH / 2;
		for (x = 0; x < LCD_WIDTH / 2; x++)
			lcd_buffer[offsetDst + x] = ~bmp[offsetSrc + x];
	}
}
