/*
 * SEND.c
 *
 * Created: 4/7/2014 1:18:39 PM
 *  Author: robsv107
 */ 


/*
 * SendSensor.c
 *
 * Created: 4/7/2014 8:44:12 AM
 *  Author: robsv107
 */ 



#include <avr/io.h>
#include <inttypes.h>

/***********************MASTER Styr*****************************/

typedef int bool;
enum {false, true};

//Start bytes for transition
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
char stop = 0x00; //Stop byte

//Control signals
char right = 1;
char left = 2;
char turn = 3;
bool remoteControl = false;  // Change to Port connected to switch
bool regulateright = true;
bool regulateleft = false;
bool regulateturn = false;

unsigned char storedValues[11] = {0b11111111, 0b11001100, 0b00110011, 0b00000000, 0b00011100, 0b11100011, 0b11000111
								, 0b00111000, 0b11111111, 0b11001100, 0b00110011};



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
void delay()
{
	for(int i = 0; i < 100; i++){}
}

/*
void TransmitSensor(char invalue)
{
	PORTB &= 0b11101111; // ss2 low
	
	MasterTransmit(RFID);
	//First communication will contain crap on shift register
	for(int i = 0; i < 100; i++){}
	MasterTransmit(traveldist); // Request front sensor
	for(int i = 0; i < 100; i++){}
	storedValues[7] = SPDR; // SensorRFID
	MasterTransmit(front); // Request front sensor
	storedValues[5] = SPDR; // Distance
	
	if(invalue == right)
	{
		MasterTransmit(rightfront);
		storedValues[0] = SPDR; // Front
		MasterTransmit(rightback);
		for(int i = 0; i < 100; i++){}
		storedValues[1] = SPDR; // Right front
		MasterTransmit(stop);
		storedValues[2] = SPDR; // Right back
	}
	else if(invalue == left)
	{
		MasterTransmit(leftfront);
		storedValues[0] = SPDR; // Front
		MasterTransmit(leftback);
		storedValues[3] = SPDR; // Left front
		MasterTransmit(stop);
		storedValues[4] = SPDR; // Left back
	}
	else if(invalue == turn)
	{
		MasterTransmit(gyro);
		storedValues[0] = SPDR; // Front
		MasterTransmit(stop);
		storedValues[6] = SPDR; // Gyro
	}
	else
	{
		MasterTransmit(stop);
		storedValues[0] = SPDR; // Front
	}
	
	for(int i = 0; i < 100; i++){}
	PORTB ^= 0b00010000; // ss2 high
}

*/

void TransmitComm(bool invalue)
{
	PORTB &= 0b11110111;
	
	if(invalue)
	{
		
	}
	else
	{
		for(int i = 0; i < 11;i++)
		{
			MasterTransmit(storedValues[i]);
		}
	}
	
	PORTB ^= 0b00001000;
	
}


int main(void)
{
	MasterInit();
	while(1)
	{	
	/*if(regulateright)
	TransmitSensor(right);
	else if(regulateleft)
	TransmitSensor(left);
	else if(regulateturn)
	TransmitSensor(turn);
	else
	TransmitSensor(0x00);
	*/
	
	TransmitComm(remoteControl);
	for(int i = 0; i < 100; i++){}
	}
	return 0;
}

/*
 * Recievecomm.c
 *
 * Created: 4/7/2014 5:08:19 PM
 *  Author: robsv107
 */ 


#include <avr/io.h>

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

char dummy;

bool remoteControl = false; // Change to Port connected to switch

unsigned char storedValues[11];


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
	SlaveInit();
	while(1)
	{
		if(remoteControl)
		{
		
		}
		else
		{
			while(1<<PINB3)
			{
				for(int i = 0; i < 11; i++)
				{
					storedValues[i] = SlaveRecieve();
					dummy = 1;
				}
			
			
				dummy  = 0;
			}
		}
	}
}




/*
 * Recieve.c
 *
 * Created: 4/7/2014 1:49:30 PM
 *  Author: robsv107
 */ 

#include <avr/io.h>

/***********************SLAVE Sensor*****************************/
typedef int bool;
enum {false, true};

//Start bytes for transmition
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveldist = 0b00000110;
char gyro = 0b00000111;
char RFID = 0b00001000;
char stop = 0x00; //Stopbyte

char selection;

unsigned char sortedValues[5] = {0b11001100, 0b00110011, 0b11100011
									,0b00011100, 0b11000111};
unsigned char Distance = 0b00001100;
unsigned char sendGyro = 0b10101010;
bool isRFID = true;


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
	SlaveInit();
	
	while(1)
	{
		
		while(PINB4) // slave selected
		{
			selection = SlaveRecieve();
			SPDR = 0b11111111;
						
			if(selection == front)
			{
				SPDR = sortedValues[0];
			}
			else if (selection == rightfront)
			{
				SPDR = sortedValues[1];
			}
			else if (selection == rightback)
			{
				SPDR = sortedValues[2];
			}
			else if (selection == leftfront)
			{
				SPDR = sortedValues[3];
			}
			else if (selection == leftback)
			{
				SPDR = sortedValues[4];
			}
			else if (selection == traveldist)
			{
				SPDR = Distance;
				//Distance = 0;
			}
			else if (selection == gyro)
			{
				SPDR = sendGyro;
				sendGyro = 0;
			}
			else if (selection == RFID)
			{
				SPDR = isRFID;
			}
			else if (selection == stop)
			{
				// behöver förmodligen inte göra något här
			}
			
		}
	}
}

