#include <s3c44b0x.h>
#include <uart.h>

void uart0_init(void) {
	UFCON0 = 0x1;
	UMCON0 = 0x0;
	ULCON0 = (0 << 6) | (0 << 3) | (0 << 2) | (3);
	UBRDIV0 = 0x22;
	UCON0 = 0x5;
}

//envia un caracter
void uart0_putchar(char ch) {
	//Esperar mientras UFSTAT0[9] == 1 si se usa FIFO
	while (((UFSTAT0>>9) & 1) == 1)//espera mientras Rx FIFO no este llena
		;
	UTXH0 = ch;
}

//Espera la recepción de un carácter y lo devuelve
char uart0_getchar(void) {

	while ((UFSTAT0 & 0xF) == 0)//se espera mientras Rx FIFO este vacía
		;
	char s = URXH0;
	return s;
}

//envia una cadena de caracteres
void uart0_puts(char *s) {
	while ((*s) != '\0') {
		uart0_putchar(*s);
		s++;
	}
}

/*Envía un entero con signo como una cadena de dígitos decimales*/
void uart0_putint(int32 i) {
	char buf[9 + 1 + 1];
	char *p = buf + 9 + 1;
	char sig = '0';
	int8 digito;

	*p = '\0';

	do {
		if (i < 0) {
			sig = '1';
			i *= -1;
		}
		digito = i % 10;
		*--p = '0' + digito; //almacena digito
		i = i / 10;
	} while (i);
	if (sig == '1') {
		*--p = '-'; // pone signo
	}
	uart0_puts(p); // envia cadena
}

/*Envía un entero sin signo como una cadena de dígitos hexadecimales*/
void uart0_puthex(uint32 i) {
	char buf[8 + 1];
	char *p = buf + 8; //Los caracteres se	generan	comenzando	por	el	menos	significativo
	uint8 c;

	*p = '\0';  // fin de cadena

	do {
		c = i & 0xf;	// resto de division /16
		if (c < 10)
			*--p = '0' + c;	//almacena caracter
		else
			*--p = 'a' + c - 10;// alamacena caracter
		i = i >> 4; // division por 16
	} while (i);

	uart0_puts(p); // envia cadena
}

/*Espera la recepción de una cadena y la almacena.*/
void uart0_gets(char *s) {/*Los caracteres se reciben y almacenan de uno en uno hasta detectar el fin de línea LF ('\n') que no se almacena.
 En su lugar se almacena el centinela fin de cadena ('\0')*/

	do{
		*s = uart0_getchar();
		if((*s) != '\n')
				s++;
	}
	while ((*s) != '\n');
	*s = '\0';
}

/*Espera la recepción de una cadena de dígitos decimales que interpreta como un entero con signo que devuelve*/
int32 uart0_getint(void) {
//	int32 numero=0;
//	int8 signo=1;
//	char c;
//	c = uart0_getchar();
//	if(c == '-'){ //si es negativo
//		signo=-1;
//	}else{
//		numero = numero*10 + ( c - '0');
//	}
//
//	c = uart0_getchar(); //cogemos digito
//	if (c != '\n') {
//		do {
//			numero = numero * 10 + (c - '0'); //acumulamos digitos
//			c = uart0_getchar();
//		} while (c != '\n');
//	}
//
//	return numero*signo;

	char buf[10+1+1];
	char *p=buf;
	int32 numero=0;
	int8 signo=1;
	uart0_gets(p);
	if((*p)=='-'){
		signo=-1;
		++p;
	}
	while((*p)!='\0'){
		numero = numero * 10 + ((*p) - '0'); //acumulamos digitos
		++p;
	}

	return signo*numero;
}

/*Espera la recepción de una cadena de dígitos hexadecimales que interpreta como un entero sin signo que devuelve*/
uint32 uart0_gethex(void) {
	uint32 numero = 0;
	char c ;
	c = uart0_getchar(); //cogemos digito

	if (c != '\n') {
		do {
			if (c-'0' < 10) {
				numero = numero * 16 + (c - '0'); // acumulamos digitos entre [0:9]
			}else if(c >= 'a' && c<= 'f'){
				numero = numero * 16 + (c - 'a' + 10);
			}
			else {
				numero = numero * 16 + (c - 'A' + 10); //// acumulamos digitos entre [A:F]
			}
			c = uart0_getchar();
		} while (c != '\n');
	}

	return numero;
}
