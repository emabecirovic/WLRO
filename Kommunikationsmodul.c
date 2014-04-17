/*
* Bluetooth_test.c
*
* Created: 3/27/2014 8:23:02 AM
*  Author: poner538
*/


#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

/***********************SLAVE komm*****************************/

typedef int bool;
enum {false, true};

//Lables for transmition
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveldist = 0b00000110;
char gyro = 0b00000111;
char RFID = 0b00001000;
char direction = 0b00001001;
char rightspeed = 0b00001010;
char leftspeed = 0b00001011;
char stop = 0x00; //Stopbyte

unsigned char storedValues[11];
int indexvalue = 0;

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


void SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDRB = (1<<DDB6);

	/* Enable SPI */
	SPCR = (1<<SPE)|(1<<SPIE);

	/* Enable external interrupts */
	sei();
}

int main(void)
{
// char data;
unsigned char test = '0';
//USARTInit(8);
SlaveInit();

while(1)
{
//Infinite Loop

//Read data
//USART_Flush();
//data=USART_Recive();
//skicka data
for(int i = 0; i < 11; i++)
{
	if(i == 0)
	{
		USARTWriteChar(front);
	}
	else if (i == 0)
	{
		USARTWriteChar(rightfront);
	}
	else if (i == 0)
	{
		USARTWriteChar(rightback);
	}
	else if (i == 0)
	{
		USARTWriteChar(leftfront);
	}
	else if (i == 0)
	{
		USARTWriteChar(leftback);
	}
	else if (i == 0)
	{
		USARTWriteChar(traveldist);
		//Distance = 0;
	}
	else if (i == 0)
	{
		USARTWriteChar(gyro);
		//sendGyro = 0;
	}
	else if (i == 0)
	{
		USARTWriteChar(RFID);
	}
	else if (i == 0)
	{
		// behöver förmodligen inte göra något här
	}
	USARTWriteChar(storedValues[i]);
}
//skicka ut bluetooth signalen till portB
//PORTB=test;
}

return 0;
}

/*******************************INTERRUPTS*************************/
ISR(SPI_STC_vect) // Answer to call from Master
{
	storedValues[indexvalue] = SPDR;
	SPDR = storedValues[indexvalue]; //Just for controll by oscilloscope

	if(indexvalue < 11)
	indexvalue++;
	else
	indexvalue = 0;
}
