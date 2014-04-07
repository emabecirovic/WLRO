/*
 * Slave.c
 *
 * Created: 3/29/2014 10:03:33 AM
 *  Author: robsv107
 */ 


#include <avr/io.h>

void SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDRB = (1<<DDB6);
	
	/* Enable SPI */
	SPCR = (1<<SPE);
}

char SlaveRecieve(void)
{
	/*Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;
	
	/* Return Data Register */
	return SPDR;
}


int main(void)
{
	DDRA = 0xFF;
	SlaveInit();
	
    do
    {
		SPDR = 0b01101101;
		if(1<<PORTB4)
		{		
		PORTA = SlaveRecieve();
		}
    }while(1);
}
