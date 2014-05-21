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

volatile char room[31][17];

volatile unsigned char storedValues[13];

int indexvalue = 0;

/****************KARTLÄGGNING*******************************/
bool drivetoY = true; // Y-led är prioriterad riktining om sant i driveto

int firstzeroX = 0; //Första nollan om man läser matrisen uppifrån och ned
int firstzeroY = 0;

volatile bool doextend = false;
volatile bool dofindfirst = false;

//char room[31][17]; //=.... 0=outforskat, 1=vägg, 2=öppen yta

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
void roominit()
{
	for(int i = 0; i < 31; i++)
	{
		for(int j = 0; j < 17 ; j++)
		{
			room[i][j] = 0;
		}
	}
}


void SendStoredVal()
{
	storedValues[5] = firstzeroX;
	storedValues[6] = firstzeroY;

	for(int i = 0; i < 13; i++)
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
		else if (i == 11)
		{
			USARTWriteChar(12);// behöver förmodligen inte göra något här
		}
		else if (i == 12)
		{
			USARTWriteChar(13);// behöver förmodligen inte göra något här
		}
		USARTWriteChar(storedValues[i]);
	}
}


/***********************************KARTHANTERING***********************************/
void setwall(char x,char y)
{
	if((room[x][y] == 2) || (room[x][y] == 4))
	{
		
	}
	else
	{
		room[x][y] = 1;
	}
}

void updatemap(char w) // Kan väl bara gälla för yttervarvet?
{
	//char w=30; //Hur långt ifrån vi ska vara för att säga att det är en vägg.
	
	switch(mydirection)
	{
		case (1): // X+
		if(sensorright<=25) //Vet inte vad som är en lämplig siffra här
		{
			setwall(myposX,myposY-1);
		}/*
		else if(sensorfront<=25)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensorleft<w)
		{
			setwall(myposX,myposY+1);
		}
		else if((sensorfront>=45) && (sensorfront<=55))
		{
			setwall(myposX+2, myposY);
		}
		else if((sensorfront>=85) && (sensorfront<=95))
		{
			setwall(myposX+3, myposY);
		}
		else if((sensorfront>=125) && (sensorfront<=135))
		{
			setwall(myposX+4, myposY);
		}
		else if((sensorfront>=165) && (sensorfront<=175))
		{
			setwall(myposX+5, myposY);
		}

		/*if (!((room[myposX-1][myposY] == 1) || (room[myposX-1][myposY]== 4)))
		{
			room[myposX-1][myposY]=2;
		}*/
		break;

		case (2): // Y+
		if(sensorright<=w)
		{
			setwall(myposX+1,myposY);
		}/*
		else if(sensorleft<w)
		{
			setwall(myposX-1,myposY);
		}
		else if((sensorfront>=45) && (sensorfront<=55))
		{
			setwall(myposX, myposY+2);
		}
		else if((sensorfront>=85) && (sensorfront<=95))
		{
			setwall(myposX, myposY+3);
		}
		else if((sensorfront>=125) && (sensorfront<=135))
		{
			setwall(myposX, myposY+4);
		}
		else if((sensorfront>=165) && (sensorfront<=175))
		{
			setwall(myposX, myposY+5);
		}

		/*if (!((room[myposX][myposY-1] == 1) || (room[myposX][myposY-1]== 4)))
		{
			room[myposX][myposY-1]=2;
		}*/
		break;

		case (3): // X-
		if(sensorright<=w)
		{
			setwall(myposX,myposY+1);
		}/*
		else if(sensorfront<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensorleft<w)
		{
			setwall(myposX,myposY-1);
		}
		else if((sensorfront>=45) && (sensorfront<=55))
		{
			setwall(myposX-2, myposY);
		}
		else if((sensorfront>=85) && (sensorfront<=95))
		{
			setwall(myposX-3, myposY);
		}
		else if((sensorfront>=125) && (sensorfront<=135))
		{
			setwall(myposX-4, myposY);
		}
		else if((sensorfront>=165) && (sensorfront<=175))
		{
			setwall(myposX-5, myposY);
		}

		/*if (!((room[myposX+1][myposY]) || (room[myposX + 1][myposY] == 4)))
		{
			room[myposX+1][myposY]=2;
		}*/
		break;

		case (4): // Y-
		if(sensorright<=w)
		{
			setwall(myposX-1,myposY);
		}/*
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY-1);
		}
		else if(sensorleft<w)
		{
			setwall(myposX+1,myposY);
		}
		else if((sensorfront>=45) && (sensorfront<=55))
		{
			setwall(myposX, myposY-2);
		}
		else if((sensorfront>=85) && (sensorfront<=95))
		{
			setwall(myposX, myposY-3);
		}
		else if((sensorfront>=125) && (sensorfront<=135))
		{
			setwall(myposX, myposY-4);
		}
		else if((sensorfront>=165) && (sensorfront<=175))
		{
			setwall(myposX, myposY-5);
		}*/

		break;
		default:
		{
			
		}
		}//swich
	if(storedValues[7]==1)
	{
		room[myposX][myposY]=4;
	}
	else
	{
		room[myposX][myposY] = 2;
	}
}
	
			/*********************************RITA UT FÖRLÄNGD VÄGG*************************************/
void extended_wall()
{
	/*for(int i = 0; i < 31;i++)
	{
		room[i][0] = 1;
	}*/
				
	volatile char flag = 0;
		
	for(int y = 0; y < 17; y++)
	{
		flag = 0;
		for(int x = 0; x < 31; x++)
		{
			if(flag == 0)
			{
				if(room[x][y] == 0)
				{
					room[x][y] = 1;
				}
						
				if(room[x][y] == 2)
				{
					flag = 1;
				}
			}
			else if(flag == 1)
			{
				if(room[x][y] == 0)
				{
				}
				if(room[x][y] == 1)
				{
					flag = 0;
				}
			}
		}
	}
}
				
	

/******************GE POSITIONEN FÖR ICKE SÖKT RUTA*********************/
void findfirstzero()
{
	int firstzero[2]={15,1};
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
	char data;
	USARTWriteChar(14);
	for(int i = 0; i < 31; i++)
	{
		for(int x = 0; x < 300; x++)
		{
			
		}
		for(int j = 0; j < 17; j++)
		{
			data = room[i][j];
			USARTWriteChar(data);
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
	// char data;
	int f = 0;
	unsigned char data;
	char select = PIND & 0b01000000;
	roominit();
	storedValues[8] = 1;
	storedValues[11] = 15;
	storedValues[12] = 1;
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
				cli();
				doextend = false;
				asm("");
						
				extended_wall();
				sendmap();
				findfirstzero();
						
						
				sei();
			}
					
			sensorfront = frontsensor(storedValues[0]);
			sensorright = sidesensor(storedValues[1]);
			sensorleft = sidesensor(storedValues[3]);
			mydirection = storedValues[8];
			
			myposX = storedValues[11];
			myposY = storedValues[12];
						
						
						
			updatemap(25);
			if(f > 2000)
			{
				sendmap();
				f = 0;
			}
			f++;
			findfirstzero();
						
			if(bussComplete == true)
			{
				//cli();
						
				//SendStoredVal();
				USARTWriteChar(12);
				USARTWriteChar(myposX);
				USARTWriteChar(13);
				USARTWriteChar(myposY);
						
				bussComplete = false;
				sei();
			}
		}//while!remote

	}//while 1


	return 0;
}//main


/*******************************INTERRUPTS*************************/

ISR(SPI_STC_vect) // Answer to call from Master
{
			
	if(arrayontheway)
	{
		storedValues[indexvalue] = SPDR;
		indexvalue++;
			
		if(indexvalue > 12)
		{
			indexvalue = 0;
			senddataval++;
			arrayontheway = false;
		}
		if(senddataval > 0)
		{
			bussComplete = true;
			senddataval = 0;
			cli();
		}
	}
	else
	{
		selection = SPDR;
		if(selection == arraytransmit)
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

