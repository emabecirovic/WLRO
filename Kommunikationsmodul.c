/*
* Kommunikationsmodul.c
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


bool remote = false;
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
char room[29][15];

volatile unsigned char storedValues[11] = {11,12,13,14};
int indexvalue = 0;

char dummy;

void USARTInit(unsigned int ubrr_value)
{
	//sätter en baud rate
	UBRR0H = (unsigned char)(ubrr_value>>8);
	UBRR0L = (unsigned char)ubrr_value;

	/*sätt frame format till 8 databitar och 1 stoppbit*/
	UCSR0C=(0<<USBS0)|(3<<UCSZ00);

	UCSR0A =(1<<U2X0);


	/* Tillåt reciever och transmitter kl*/
	UCSR0B=(1<<RXEN0)|(1<<TXEN0); //|(1<<RXCIE0);

	/* */
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
	SPCR = (1<<SPE)|(1<<SPIE)|(1<<CPHA)|(1<<CPOL);

	/* Enable external interrupts */
	//sei();
}

char SlaveRecieve(void) // Används inte just nu men....
{
	/*Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;
	/* Return Data Register */
	return SPDR;

}

void SendStoredVal()
{

		
		for(int i = 0; i < 11; i++)
		{
			if(i == 0)
			{
				USARTWriteChar(front);
			}
			else if (i == 1)
			{
				USARTWriteChar(rightfront);
			}
			else if (i == 2)
			{
				USARTWriteChar(rightback);
			}
			else if (i == 3)
			{
				USARTWriteChar(leftfront);
			}
			else if (i == 4)
			{
				USARTWriteChar(leftback);
			}
			else if (i == 5)
			{
				USARTWriteChar(traveldist);
				//Distance = 0;
			}
			else if (i == 6)
			{
				USARTWriteChar(gyro);
				//sendGyro = 0;
			}
			else if (i == 7)
			{
				USARTWriteChar(RFID);
			}
			else if (i == 8)
			{
				USARTWriteChar(direction);// behöver förmodligen inte göra något här
			}
			else if (i == 9)
			{
				USARTWriteChar(leftspeed);// behöver förmodligen inte göra något här
			}
			else if (i == 10)
			{
				USARTWriteChar(rightspeed);// behöver förmodligen inte göra något här
			}
			USARTWriteChar(storedValues[i]);
		}
	
}




int main(void)
{
	// char data;
	char data;
	DDRB = (1<<DDB0)|(1<<DDB1)|(1<<DDB2);
	USARTInit(8);
	SlaveInit();
	//sei();
	remote = false;
	while(1)
	{
		
		while(remote)
		{

			data = USART_Recive();
			if(data == 'K')
			{
				remote = false;
			}
			else
			{
				
			PORTB &= 0b01000000;
			PORTB |= data;
			}
			//USART_Flush();
			//data=USART_Recive();
			//skicka data
			
		}
		while(!remote)
		{
			char ss1 = PORTB & 0b00010000;
			while (ss1 == 0)
			{
				if(indexvalue<11)
				{
					storedValues[indexvalue]=SlaveRecieve();
					indexvalue++;
				}
				else
				{
					indexvalue = 0;
				}
				ss1 = PORTB & 0b00010000;
			}	
			//for(int i; i)
			
			SendStoredVal();
			//sei();
			
		
		}
		//USARTWriteChar(data);
	}
	//kicka ut bluetooth signalen till portB
	//PORTB=tes

return 0;  
}

/*******************************INTERRUPTS*************************/
/*
ISR(SPI_STC_vect) // Answer to call from Master
{
	SPDR = 0;
	cli();
	//SPDR = storedValues[indexvalue]; //Just for controll by oscilloscope

		storedValues[indexvalue] = SPDR;

	sei();
}
/*
ISR(USART0_RX_vect)
{

}*/
