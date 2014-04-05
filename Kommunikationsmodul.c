*
 * Bluetooth_test.c
 *
 * Created: 3/27/2014 8:23:02 AM
 *  Author: poner538
 */ 


#include <avr/io.h>
#include <inttypes.h>


void USARTInit(unsigned int ubrr_value)
{
	//sätter en baud rate
	UBRR0H = (unsigned char)(ubrr_value>>8);
	UBRR0L = (unsigned char)ubrr_value;
	
	/*sätt frame format till 8 databitar och 1 stoppbit*/
	UCSR0C=(0<<USBS0)|(3<<UCSZ00);
	
	UCSR0A =(1<<U2X0);
	
	
	/* Tillåt reciever och transmitter kl*/
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);

}

unsigned char USART_Recive(void)
{
	//väta tills det finns data i bufferten
	//while( !(UCSR0A & (1<<UDRE0)));
	//PORTB=UDR0;
	while(!(UCSR0A & (1<<RXC0)))
	{
		
	}
	
	//returnera datan från bufferten
	return UDR0;
}

void USARTWriteChar(unsigned char data)
{
	
	//Wait until the transmitter is ready
	while(!(UCSR0A & (1<<UDRE0)))
	{
		//Do nothing
	}
	UDR0=data;

	//Now write the data to USART buffer
	
}

void USART_Flush( void )
{
	unsigned char dummy;
	while (UCSR0A & (1<<RXC0) )
	{ 
		dummy = UDR0;
	}
}

int main(void)
{
	DDRB=0xFF;
	unsigned char data;
	unsigned char test = {'B'};
	USARTInit(8);
	while(1)
	{	 
 
	//Infinite Loop  
 
    //Read data
	USART_Flush();
	data=USART_Recive();
	//skicka data
	//USARTWriteChar(test);
    //skicka ut bluetooth signalen till portB
   // PORTB=test;
  PORTB=data;
	}
	
return 0;
}
