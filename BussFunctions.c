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
char start_request = 0;

//Control signals
char right = 1;
char left = 2;
char turn = 3;
bool remoteControl = false;  // Change to Port connected to switch
bool regulateright = false;
bool regulateleft = false;
bool regulateturn = false;
int time = 50;

unsigned char storedValues[11] = {0b11111111, 0b11001100, 0b00110011, 0b00000000, 0b00011100, 0b11100011, 0b11000111
								, 0b00111000, 0b11111111, 0b11001100, 0b00110011};



void MasterInit(void)
{
	/* Set MOSI and SCK output, alla others input*/
	/* Ersätt DDR_SPI med den port "serie" som används ex DD_SPI -> DDRB
	samt DD_MOSI och DD_SCK med specifik pinne ex DD_MOSI -> DDB5 */
	DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB7);

	/* Enable SPI, Master, set clock rate fosc/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(0<<SPI2X)|(1<<SPR1)|(0<<SPR0)|(1<<CPHA)|(1<<CPOL);

	PORTB = (1<<PORTB3)|(1<<PORTB4);
}  


void MasterTransmit(char cData)
{
	/* Start transmission */
	SPDR = cData;

	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}



void TransmitSensor(char invalue)
{
	PORTB &= 0b11101111; // ss2 low
	
	if(invalue == turn)
	{
		MasterTransmit(gyro);
		for(int i = 0; i < time; i++){}
		MasterTransmit(stop);
		storedValues[6] = SPDR; // Gyro
	}
	else if(invalue == turnstop)
	{
		MasterTransmit(0b1000000);
	}
	else
	{
	MasterTransmit(RFID);
	//First communication will contain crap on shift register
	for(int i = 0; i < time; i++){}
	MasterTransmit(traveldist); // Request front sensor
	for(int i = 0; i < time; i++){}
	storedValues[7] = SPDR; // SensorRFID
	MasterTransmit(front); // Request front sensor
	for(int i = 0; i < time; i++){}
	storedValues[5] = SPDR; // Distance
	
	if(invalue == right)
	{
		MasterTransmit(rightfront);
		for(int i = 0; i < time; i++){}
		storedValues[0] = SPDR; // Front
		MasterTransmit(rightback);
		for(int i = 0; i < time; i++){}
		storedValues[1] = SPDR; // Right front
		MasterTransmit(stop);
		storedValues[2] = SPDR; // Right back
	}
	else if(invalue == left)
	{
		MasterTransmit(leftfront);
		for(int i = 0; i < time; i++){}
		storedValues[0] = SPDR; // Front
		MasterTransmit(leftback);
		for(int i = 0; i < time; i++){}
		storedValues[3] = SPDR; // Left front
		MasterTransmit(stop);
		storedValues[4] = SPDR; // Left back
	}
	else
	{
		for(int i = 0; i < time; i++){}
		MasterTransmit(stop);
		storedValues[0] = SPDR; // Front
	}
	}
	PORTB ^= 0b00010000; // ss2 high
}



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

void initiate_request_timer()
{
	TIMSK0 = 0b00000100; //Enable interupt vid matchning med OCR1B		TCCR1B =0x00;
	TCNT0 = 0x00;
	TCCR0B = 0x03; //Starta räknare, presscale 64.
	OCR0BH = 0x01;
	OCR0BL = 0x00; //RANDOM! När ska requesten triggas? 
}


int main(void)
{
	MasterInit();
	while(1)
	{
		if(start_request == 1)
		{
			start_request = 0;
			if(regulateright)
			TransmitSensor(right);
			else if(regulateleft)
			TransmitSensor(left);
			else if(regulateturn)
			TransmitSensor(turn);
			else
			TransmitSensor(0x00);
		}
	TransmitComm(remoteControl);
	
	}
	
	return 0;
}

/*
 * Recievecomm.c
 *
 * Created: 4/7/2014 5:08:19 PM
 *  Author: robsv107
 */ 
 
ISR(TIMER0_COMPB_vect)
{
	TCNT0 = 0x00;
	start_request = 1;
}


#include <avr/io.h>
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

char dummy;

bool remoteControl = false; // Change to Port connected to switch

unsigned char storedValues[11];
int index = 0; // index for recieved storedValue from buss

void SlaveInit(void)
{
	
	/* Set MISO output, all others input */
	DDRB = (1<<DDB6);
	
	/* Enable Global Interrupt */
	sei();
	
	/* Enable SPI and interrupts */
	SPCR = (1<<SPE)|(1<<SPIE)|(1<<CPHA)|(1<<CPOL);
}

char SlaveRecieve(void)
{
	/*Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;

	/* Return Data Register */
	return SPDR;
}

/*******************************INTERRUPTS*************************/
ISR(SPI_STC_vector) // Answer to call from Master
{
	storedValues[index] = SPDR;
	SPDR = storedValues[index]; //Just for controll by oscilloscope
	
	if(indexvalue < 11)
	indexvalue++;
	else
	indexvalue = 0;
}

/*******************************MAIN*******************************/
int main(void)
{
	SlaveInit();
	
	while(1)
	{
		if(remoteControl)
		{
			
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
#include <avr/interrupt.h>

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
	
	/* Enable Global Interrupt */
	sei();
	
	/* Enable SPI and interrupts */
	SPCR = (1<<SPE)|(1<<SPIE)|(1<<CPHA)|(1<<CPOL);
}

char SlaveRecieve(void)
{
	/*Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;

	/* Return Data Register */
	return SPDR;
}


/*************************INTERRUPTS********************/
ISR(SPI_STC_vector)
{
	char selection = SPDR;
	
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
		//sendGyro = 0;
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

/****************************MAIN**********************/
int main(void)
{
	SlaveInit();
	
	while(1)
	{
	
	}
}
