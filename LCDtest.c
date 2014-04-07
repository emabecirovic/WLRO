#include <avr/io.h>
#include <avr/delay.h>



int main()
{
	DDRA=0b11111111; //Sätter utgångar
	DDRC=0b11000000;
	
	//Startar initiering
	PORTA=0b00111100; // 2-line mode ; 5x8 Dots
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_us(400);
	PORTA=0b00001111; // Display on ; Cursor on ; Blink on
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_us(400);
	PORTA=0b00000001; // Clear display
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_ms(20);
	PORTA=0b00000111; //Decrement mode ; Entire shift on
	PORTC=0b10000000;
	_delay_ms(20);
	//Initiering klar
	
	_delay_ms(4000);	

	 while(1)
		{
			writechar(0b01010111); //W
			writechar(0b01001100); //L
			writechar(0b01010010); //R
			writechar(0b01001111); //O
			writechar(0b00010000); //Space
			shift(5);
			_delay_ms(5000);
		 }
	return 0;
}

void writechar(unsigned char data)
{
	PORTA=data;
	PORTC=0b11000000;
	PORTC=0b01000000;
	_delay_ms(30);
}


void shift(int steps) //Gör saker
{
	int n=0;
	while(n<steps)
	{
	PORTA=0b00011100; //Shift display right
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_ms(30);
	n+=1;
	}
	
	int m=0;
	while(m<(2*steps))
	{
	PORTA=0b00010000; //Shift cursor left
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_ms(30);
	m+=1;
	}
}
