
#include <s3c44b0x.h>
#include <leds.h>


//static uint8 stateL;
//static uint8 stateR;
static uint8 state;

void leds_init( void )
{
    led_off(LEFT_LED);
    state &= ~((ON<<0));
    led_off(RIGHT_LED);
    state &= ~((ON<<1));
}

void led_on( uint8 led )
{
	if(led==LEFT_LED){
		PDATB &= ~((1<<9));
		state = ((ON<<0));
	}else{
		PDATB &= ~((1<<10));
		state = ((ON<<1));
	}

}

void led_off( uint8 led )
{
	if(led == LEFT_LED){
		PDATB |= ( (1<<9));
		state = ((OFF<<0));
	}else{
	   	PDATB |= ( (1<<10));
	   	state = ((OFF<<1));
	}
}

void led_toggle( uint8 led )
{
	if(led==LEFT_LED){
	   	PDATB ^= ( (1<<9));
	   	state ^= ( (ON<<0));
	}else{
	   	PDATB ^= ( (1<<10));
	   	state ^= ( (ON<<1));
	}
}

uint8 led_status( uint8 led )
{
	if(led == LEFT_LED){
		//return state &= (1<<0);
		return  state & (1<<0);
	}else /*if(led == RIGHT_LED)*/{
		//return state &= (1<<1);
		 return (state & (1<<1))>>1;
	}
}
