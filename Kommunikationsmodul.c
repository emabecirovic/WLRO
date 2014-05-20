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

int senddataval = 0;

volatile bool arrayontheway = false;

volatile bool bussComplete = false;

bool remote = false;

//Lables for transmition
const char front = 0b00000001;
const char rightfront = 0b00000010;
const char rightback = 0b00000011;
const char leftfront = 0b00000100;
const char leftback = 0b00000101;
const char traveldist = 0b00000110;
const char gyro = 0b00000111;
const char RFID = 0b00001000;
const char direction = 0b00001001;
const char rightspeed = 0b00001010;
const char leftspeed = 0b00001011;
const char firstdone = 0b00001100;
const char findzeroX = 0b00001101;
const char findzeroY = 0b00001110;
const char arraytransmit = 0b00001111;
const char stop = 0x00; //Stopbyte
volatile char selection; // Används i skicka avbrottet

char room[29][15];

volatile unsigned char storedValues[11];
int indexvalue = 0;

/****************KARTLÄGGNING*******************************/
bool drivetoY = true; // Y-led är prioriterad riktining om sant i driveto

int firstzeroX; //Första nollan om man läser matrisen uppifrån och ned
int firstzeroY;

volatile bool doextend = false;
volatile bool dofindfirst = false;

char room[29][15]; //=.... 0=outforskat, 1=vägg, 2=öppen yta


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
void USARTInit2(unsigned int ubrr_value)
{
	//sätter en baud rate
	UBRR0H = (unsigned char)(ubrr_value>>8);
	UBRR0L = (unsigned char)ubrr_value;

	/*sätt frame format till 8 databitar och 1 stoppbit*/
	UCSR0C=(0<<USBS0)|(3<<UCSZ00);

	//UCSR0A =(1<<U2X0);


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


/***********************************KARTHANTERING***********************************/
void setwall(int x,int y)
{
	room[x][y]=1;
}

void updatemap() // Kan väl bara gälla för yttervarvet?
{
	char w=30; //Hur långt ifrån vi ska vara för att säga att det är en vägg.

	switch(storedValues[8])
	{
		case (1): // X+
		if(sensormeanr<=w) //Vet inte vad som är en lämplig siffra här
		{
			setwall(myposX,myposY-1);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensorleft<w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensorfront>=45 & sensorfron<=55)
		{
			setwall(myposX+2, myposY)
		}
		else if(sensorfront>=85 & sensorfron<=95)
		{
			setwall(myposX+3, myposY)
		}	
		else if(sensorfront>=125 & sensorfron<=135)
		{
			setwall(myposX+4, myposY)
		}	
		else if(sensorfront>=165 & sensorfron<=175)
		{
			setwall(myposX+5, myposY)
		}
		
		if (!room[myposX-1][myposY]==(1|4))
		{
			room[myposX-1][myposY]=2;
		}
		break;

		case (2): // Y+
		if(sensormeanr<=w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensorleft<w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensorfront>=45 & sensorfron<=55)
		{
			setwall(myposX, myposY+2)
		}
		else if(sensorfront>=85 & sensorfron<=95)
		{
			setwall(myposX, myposY+3)
		}	
		else if(sensorfront>=125 & sensorfron<=135)
		{
			setwall(myposX, myposY+4)
		}	
		else if(sensorfront>=165 & sensorfron<=175)
		{
			setwall(myposX, myposY+5)
		}
		
		if (!room[myposX][myposY-1]==(1|4))
		{
			room[myposX][myposY-1]=2;
		}
		break;

		case (3): // X-
		if(sensormeanr<=w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensorleft<w)
		{
			setwall(myposX,myposY-1);
		}
		else if(sensorfront>=45 & sensorfron<=55)
		{
			setwall(myposX-2, myposY)
		}
		else if(sensorfront>=85 & sensorfron<=95)
		{
			setwall(myposX-3, myposY)
		}	
		else if(sensorfront>=125 & sensorfron<=135)
		{
			setwall(myposX-4, myposY)
		}	
		else if(sensorfront>=165 & sensorfron<=175)
		{
			setwall(myposX-5, myposY)
		}

		if (!room[myposX+1][myposY]==(1|4))
		{
			room[myposX+1][myposY]=2;
		}
		break;

		case (4): // Y-
		if(sensormeanr<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY-1);
		}
		else if(sensorleft<w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensorfront>=45 & sensorfron<=55)
		{
			setwall(myposX, myposY-2)
		}
		else if(sensorfront>=85 & sensorfron<=95)
		{
			setwall(myposX, myposY-3)
		}	
		else if(sensorfront>=125 & sensorfron<=135)
		{
			setwall(myposX, myposY-4)
		}	
		else if(sensorfront>=165 & sensorfron<=175)
		{
			setwall(myposX, myposY-5)
		}
		
		if (!room[myposX][myposY+1]==(1|4))
		{
			room[myposX][myposY+1]=2;
		}
		break;
	}
	if(storedValues[7]==1)
	{
		room[myposX][myposY]=4;
	}
}

/*********************************RITA UT FÖRLÄNGD VÄGG*************************************/
void extended_wall()
{
	for(int j = 0; j < 15; j++ )
	{
		for(int i = 0; i < 29; i++)
		{
			if(room[i][j] == (1 | 2))
			{
				i = 29;
			}
			else
			{
				room[i][j] = 1;
			}
		}
	}
	for(int i = 0; i < 29; i++ )
	{
		for(int j = 0; j < 15; j++)
		{
			if(room[i][j] == (1 | 2))
			{
				j = 15;
			}
			else
			{
				room[i][j] = 1;
			}
		}
	}
	for(int j = 0; j < 15; j++ )
	{
		for(int i = 28; i >= 0; i--)
		{
			if(room[i][j] == (1 | 2))
			{
				i = -1;
			}
			else
			{
				room[i][j] = 1;
			}
		}
	}
	for(int i = 0; i < 29; i++ )
	{
		for(int j = 14; j >= 0; i--)
		{
			if(room[i][j] == (1 | 2))
			{
				j = -1;
			}
			else
			{
				room[i][j] = 1;
			}
		}
	}
}

/******************GE POSITIONEN FÖR ICKE SÖKT RUTA*********************/
void findfirstzero()
{
	int firstzero[2]={15,0};

	for(int j=0;j<=17;j++)
	{
		for(int i=0;i<=31;i++)
		{
			if(room[j][i]==0)
			{
				firstzero[0]=i;
				firstzero[1]=j;
			}
		}
	}
	
	firstzeroX = firstzero[0];
	firstzeroY = firstzero[1];
	
}




int main(void)
{
	// char data;
	unsigned char data;
	char select = PIND & 0b01000000;
	if(select == 0b01000000)
	{
		cli();
		DDRB = 0b00000111;
		USARTInit2(3);
		remote = true;
	}
	else
	{
		USARTInit(8);
		SlaveInit();
		sei();
		remote = false;
	}
	
	while(1)
	{
		
		while(remote)
		{

			data = USART_Recive();
								
			PORTB = data;
			/*
			PORTB &= 0b01000000;
			PORTB |= data;

			//USART_Flush();
			//data=USART_Recive();
			//skicka data*/

		}
			
		while(!remote)
		{
			if(doextend)
			{
					doextend = false;
					extended_wall();
			}
			
			sensorfront = storedValues[0];
			sensorright = storedValues[1];
			sensorleft = storedValues[3];
			mydirection = storedValues[8];
			
			myposX = storedValues[11];
			myposY = storedValues[12];
			
			
			updatemap();
			firstzero = findfirstzero();
			
			if(bussComplete == true)
			{
				SendStoredVal();
				
				bussComplete = false;
				sei();
			}
		}

	}


	return 0;
}


			/*******************************INTERRUPTS*************************/

			ISR(SPI_STC_vect) // Answer to call from Master
			{
				
				if(arrayontheway)
				{
					storedValues[indexvalue] = SPDR;
					indexvalue++;
					senddataval++;
					
					if(indexvalue > 12)
					{
						indexvalue = 0;
					}
					if(senddataval > 150)
					{
						bussComplete = true;
						senddataval = 0;
						cli();
					}
				}
				else
				{
					selection = SPDR;
					if(arraytransmit)
					{
						arrayontheway = true;
					}
					else if(selection == firstdone)
					{
						doextend = true;
					}
					else if(selection == findzeroX)
					{
						SPDR = firstzeroX;
					}
					else if(selection == findzeroY)
					{
						SPDR = firstzeroY;
					}
					else if(selection == stop)
					{
						
					}
					
				}
			}
			/*
			ISR(USART0_RX_vect)
			{

			}*/

