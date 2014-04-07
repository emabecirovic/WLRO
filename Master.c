/*
 * Master.c
 *
 * Created: 3/29/2014 9:43:31 AM
 *  Author: robsv107
 */ 


#include <avr/io.h>
#include <inttypes.h>

void MasterInit(void)
{
	/* Set MOSI and SCK output, alla others input*/
	/* Ersätt DDR_SPI med den port "serie" som används ex DD_SPI -> DDRB
	samt DD_MOSI och DD_SCK med specifik pinne ex DD_MOSI -> DDB5 */
	DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB7);
	
	/* Enable SPI, Master, set clock rate fosc/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
	
	PORTB = (1<<PORTB3)|(1<<PORTB4);
} 


void MasterTransmit(char cData)
{
	/* Start transmission */
	SPDR = cData;
	
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
	
	//SPSR = (1<<SPIF);
}


int main(void)
{

	MasterInit();
	DDRA = 0xFF;
	
    while(1)
    {
        PORTB &= 0b11101111;
		MasterTransmit(0b01111001);
		PORTA = SPDR;
		for (int i=1; i<=100; i++);
		MasterTransmit(0b10101010);
		PORTA = SPDR;
		PORTB ^= 0b00010000;
		for (int i=1; i<=1000; i++);
    }
}
