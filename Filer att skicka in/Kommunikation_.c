/*
* FILNAMN: kommunikation_.c
* PROGRAMMERARE: Grupp 9 - WLRO
* DATUM: 2014-06-04
*
* .c-fil till kommunikationsmodulens ATmega1284P
*
*
*/

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


int senddataval = 0; //fördröjning för att skicka data på bluetooth

volatile bool arrayontheway = false; // indikation på att storedvalues kommer på bussen

volatile bool bussComplete = false;

bool remote = false;
bool iswall = false;
bool updateroombool = false;

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
const char alrdydone = 0b00001101;
const char findzeroY = 0b00001110;
const char updateroom = 0b00010000;
const char arraytransmit = 0b00001111;
const char stop = 0x00; //Stopbyte
volatile char selection; // Används i skicka avbrottet

char room[31][17];

volatile unsigned char storedValues[15];
int indexvalue = 0;

/****************KARTLÄGGNING*******************************/
bool drivetoY = true; // Y-led är prioriterad riktining om sant i driveto

int firstzeroX = 0; //Första nollan om man läser matrisen uppifrån och ned
int firstzeroY = 0;

volatile bool doextend = false;
volatile bool dofindfirst = false;

char room[31][17]; //=.... 0=outforskat, 1=vägg, 2=öppen yta

volatile char mydirection = 1;
volatile char myposX = 15;
volatile char myposY = 1;
float sensorfront;
float sensorleft;
float sensorright;


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
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);

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
	UCSR0B=(1<<RXEN0)|(1<<TXEN0);

}

unsigned char USART_Recive(void)
{
	//väta tills det finns data i bufferten
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
	}
	UDR0=data;

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

}

char SlaveRecieve(void) // Används inte just nu men....
{
	/*Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;
	/* Return Data Register */
	return SPDR;

}

//Skickar all data till PC-klienten

void SendStoredVal()
{
	for(int i = 0; i < 15; i++)
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
		}
		else if (i == 6)
		{
			USARTWriteChar(gyro);
		}
		else if (i == 7)
		{
			USARTWriteChar(RFID);
		}
		else if (i == 8)
		{
			USARTWriteChar(direction);
		}
		else if (i == 9)
		{
			USARTWriteChar(leftspeed);
		}
		else if (i == 10)
		{
			USARTWriteChar(rightspeed);
		}
		else if (i == 11)
		{
			USARTWriteChar(12);
		}
		else if (i == 12)
		{
			USARTWriteChar(13);
		}
		else if(i == 13)
		{
			USARTWriteChar(14);
		}
		else if(i == 14)
		{
			USARTWriteChar(15);
		}
		USARTWriteChar(storedValues[i]);
	}
}


/***********************************KARTHANTERING***********************************/
void setwall(int x,int y)
{
	if(!((room[x][y] == 2) || (room[x][y] == 4)))
	{
		
		room[x][y]=1;
	}
}
//uppdaterar kartan
void updatemap(char w) // w är hur långt det ska va till väggen på höger sida för att en vägg ska läggas till
{

	switch(mydirection)
	{
		case (1): // X+
		if(sensorright<=w)
		{
			setwall(myposX,myposY-1);
		}
		break;
		case (2): // Y+
		if(sensorright<=w)
		{
			setwall(myposX+1,myposY);
		}
		break;
		case (3): // X-
		if(sensorright<=w)
		{
			setwall(myposX,myposY+1);
		}
		break;
		case (4): // Y-
		if(sensorright<=w)
		{
			setwall(myposX-1,myposY);
		}
		break;
		default:
		{
		}
	}
	if(storedValues[7]==1)
	{
		room[myposX][myposY]=4;
	}
	if (!(room[myposX][myposY] == 4))
	{
		room[myposX][myposY]=2;
	}
}
/*********************************RITA UT FÖRLÄNGD VÄGG*************************************/
void extended_wall()
{
	for(int j = 0; j < 17; j++ )
	{
		for(int i = 0; i < 31; i++)
		{
			if((room[i][j] == 1) || (room[i][j] == 2))
			{
				break;
			}
			else
			{
				room[i][j] = 1;
			}
		}
	}

	for(int i = 0; i < 31; i++ )
	{
		for(int j = 0; j < 17; j++)
		{
			if((room[i][j] == 1) || (room[i][j] == 2))
			{
				break;
			}
			else
			{
				room[i][j] = 1;
			}
		}
	}

	for(int j = 0; j < 17; j++ )
	{
		for(int i = 30; i >= 0; i--)
		{
			if((room[i][j] == 1) || (room[i][j] == 2))
			{
				break;
			}
			else
			{
				room[i][j] = 1;
			}
		}
	}

	for(int i = 0; i < 29; i++ )
	{
		for(int j = 16; j >= 0; j--)
		{
			if((room[i][j] == 1) || (room[i][j] == 2))
			{
				break;
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
	int firstzero[2]={16,1};

	for(int j=0;j<17;j++)
	{
		for(int i=0;i<31;i++)
		{
			if(room[i][j]==0)
			{
				firstzero[0]=i;
				firstzero[1]=j;
			}
		}
	}

	firstzeroX = firstzero[0];
	firstzeroY = firstzero[1];

}

void sendmap()
{
	cli();
	USARTWriteChar(16);
	for(int i = 0; i < 31; i++)
	{
		for(int x = 0; x < 200; x++)
		{
		}
		for(int j = 0; j < 17; j++)
		{
			USARTWriteChar(room[i][j]);
		}
	}
	sei();
}

void clearroom()
{
	for(int i = 0; i < 31; i++)
	{
		for(int j = 0; j < 17; j++)
		{
			room[i][j] = 0;
		}
	}
}

void findclosestwall()
{
	if((sensorleft > 45) && (sensorleft < 75))
	{
		testforwall(2);
	}
	else if((sensorleft > 75) && (sensorleft < 115))
	{
		testforwall(3);
	}
	else if((sensorleft > 115) && (sensorleft < 145))
	{
		testforwall(4);
		
	}
	else if((sensorleft > 145) && (sensorleft < 190))
	{
		testforwall(5);
		
	}
	else if((sensorleft > 205) && (sensorleft < 215))
	{
		iswall = true;
		
	}
	else if((sensorleft > 245) && (sensorleft < 265))
	{
		iswall = true;
	}
	else
	{
		iswall = true;
	}
}

void testforwall(int checkval)
{
	switch(mydirection)
	{
		case(1):
		if(room[myposX][myposY + checkval] == 0 && room[myposX][myposY + 1] != 1)
		{
			iswall = false;
		}
		else
		{
			iswall = true;
		}
		break;
		case(2):
		if(room[myposX - checkval][myposY] == 0 && room[myposX - 1][myposY] != 1)
		{
			iswall = false;
		}
		else
		{
			iswall = true;
		}
		break;
		case(3):
		if(room[myposX][myposY - checkval] == 0 && room[myposX][myposY - 1] != 1)
		{
			iswall = false;
		}
		else
		{
			iswall = true;
		}
		break;
		case(4):
		if((room[myposX + checkval][myposY] == 0) && (room[myposX + 1][myposY] != 1))
		{
			iswall = false;
		}
		else
		{
			iswall = true;
		}
		break;
		default:
		{
		}
	}
	
}

float sidesensor(unsigned char sensorvalue)
{
	float value = sensorvalue;
	value = 1 / value;
	value = value - 0.000741938763948;
	value = value / 0.001637008132828;

	return value;
}

float frontsensor(unsigned char sensorvalue)
{
	float value = sensorvalue;
	value = 1 / value;
	value = value - 0.001086689563586;
	value = value / 0.000191822821525;

	return value;
}

int main(void)
{
	clearroom();
	bool secondlap = false;
	sensorright = 10;
	storedValues[11] = 16;
	storedValues[12] = 1;
	unsigned char data;
	char select = PIND & 0b01000000;
	// select avgör om roboten ska styras autonomt eller fjärrstyrt, olika initieringar för båda
	if(select == 0b01000000)
	{
		//fjärrstyrt
		cli();
		DDRB = 0b00000111;
		USARTInit2(3);
		remote = true;
	}
	else
	{
		//autonomt
		USARTInit(8);
		SlaveInit();
		sei();
		remote = false;
	}
	while(1)
	{

		while(remote) //fjärrstyrt
		{

			data = USART_Recive();

			PORTB = data;
		}

		while(!remote) // autonomt
		{
			if(doextend)
			{
				secondlap = true;
				
				cli();
				sendmap();
				sei();
				doextend = false;
			}
			

			sensorleft = frontsensor(storedValues[0]);
			sensorright = sidesensor(storedValues[1]);
			sensorfront = sidesensor(storedValues[3]);
			mydirection = storedValues[8];

			myposX = storedValues[11];
			myposY = storedValues[12];

			if(secondlap)
			{
				findclosestwall(); //hittar närmsta väggen till vänster och avgör om den är avsökt eller ej
			}
			if(updateroombool)
			{
				updatemap(25);
				updateroombool = false;
			}
			if(bussComplete == true)
			{
				cli();
				SendStoredVal();
				sei();
				bussComplete = false;
			}
		}

	}


	return 0;
}


/*******************************INTERRUPTS*************************/

ISR(SPI_STC_vect) // Answer to call from Master
{

	if(arrayontheway) //fyll på storedValues
	{
		storedValues[indexvalue] = SPDR;
		indexvalue++;
		
		if(indexvalue > 14)
		{
			indexvalue = 0;
			senddataval++;
			arrayontheway = false;
			
		}
		if(senddataval > 4) // skickar data till PC-klienten
		{
			bussComplete = true;
			
			senddataval = 0;
		}
	}
	else
	{
		selection = SPDR;
		if(selection == arraytransmit)
		{
			arrayontheway = true;
		}
		else if(selection == firstdone) //Roboten har åkt ett varv , rita ut området utanför till väggar
		{
			doextend = true;
			SPDR = dummy;
		}
		else if(selection == updateroom)
		{
			updateroombool = true;
		}
		else if(selection == alrdydone) // alrdydone , returnera true or false om segmentet till vänster är avsökt
		{
			SPDR = iswall;
		}
		else if(selection == stop)
		{

		}

	}
}
