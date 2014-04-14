#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
 #define F_CPU 1000000UL

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
bool regulateright = true;
bool regulateleft = false;
bool regulateturn = false;
int time = 50;

unsigned char storedValues[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

float sensor1r, sensor2r, sensorfront;
float sensordiff;
float sensormeanr;

int K = 3;

void MasterInit(void)
{
	/* Set MOSI and SCK output, alla others input*/
	/* Ersätt DDR_SPI med den port "serie" som används ex DD_SPI -> DDRB
	samt DD_MOSI och DD_SCK med specifik pinne ex DD_MOSI -> DDB5 */
	DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB7);

	/* Enable SPI, Master, set clock rate fosc/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);

	/* Set Slave select high */
	PORTB = (1<<PORTB3)|(1<<PORTB4);
	
	/* Enable External Interrupts */
	sei();
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

	MasterTransmit(RFID);
	//First communication will contain crap on shift register
	for(int i = 0; i < time; i++){}
	MasterTransmit(traveldist); // Request front sensor
	for(int i = 0; i < time; i++){}
	storedValues[7] = SPDR; // SensorRFID
	MasterTransmit(front); // Request front sensor
	storedValues[5] = SPDR; // Distance

	if(invalue == right)
	{
		MasterTransmit(rightfront);
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
		storedValues[0] = SPDR; // Front
		MasterTransmit(leftback);
		for(int i = 0; i < time; i++){}
		storedValues[3] = SPDR; // Left front
		MasterTransmit(stop);
		storedValues[4] = SPDR; // Left back
	}
	else if(invalue == turn)
	{
		MasterTransmit(gyro);
		for(int i = 0; i < time; i++){}
		storedValues[0] = SPDR; // Front
		MasterTransmit(stop);
		storedValues[6] = SPDR; // Gyro
	}
	else
	{
		for(int i = 0; i < time; i++){}
		MasterTransmit(stop);
		storedValues[0] = SPDR; // Front
	}

	PORTB ^= 0b00010000; // ss2 high
	//for(int i = 0; i < 100; i++){}
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
	TIMSK0 = 0b00000100; //Enable interupt vid matchning med OCR0B		TCCR1B =0x00;
	TCNT0 = 0x00;
	TCCR0B = 0b0000101; //Starta räknare, presscale 1024
	OCR0B = 0xFF; // 255
	
	// 261120 st klockcykler
}


void initiation()
{
	//Sätter utgångar/ingångar    (Kanske skriva en initieringsfunktion för allt detta? /Robert)
	DDRA=0b11111111;
	DDRC=0b11000001;
	DDRD=0b11100000;

	TCCR1A=0b10010001; //setup, phase correct PWM
	TCCR1B=0b00000010; //sätter hastigheten på klockan
	TCCR2A=0b10010001;
	TCCR2B=0b00000010;

	//Till displayen, vet inte om det behövs men den är efterbliven
	PORTA=0b00110000;
	PORTC=0b00000000;
	_delay_ms(20);
	PORTA=0b00110000;
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_ms(5);
	PORTA=0b00110000;
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_us(110);

	//Startar initiering
	PORTA=0b00111100; // 2-line mode ; 5x8 Dots
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_us(400);
	PORTA=0b00001111; // Display on ; Cursor on ; Blink on
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_us(400);
	PORTA=0b00000001; // Clear display
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_ms(20);
	PORTA=0b00000111; //Increment mode ; Entire shift on
	PORTC=0b10000000;
	_delay_ms(20);
	//Initiering klar
}


int getgyro()
{
	unsigned char sendGyro;
	//Robert, ge mig sendGyro
	if(sendGyro >= 0)
	{
		sendGyro = sendGyro * 9.33;
	}
	else
	{
		sendGyro = sendGyro * 5.04;
	}
	gyro = sendGyro*64/244000;
	return gyro;
}

void writechar(unsigned char data)
{
	PORTA=data;
	(PORTC |= 0b11000000);
	(PORTC &= 0b01000001);
	_delay_ms(30);
}


void shift(int steps) //
{
	int n=0;
	while(n<steps)
	{
		PORTA=0b00011100; //Shift display right
		(PORTC |= 0b11000000);
		(PORTC &= 0b01000001);
		_delay_ms(30);
		n+=1;
	}
	
	int m=0;
	while(m<(2*steps))
	{
		PORTA=0b00010000; //Shift cursor left
		(PORTC |= 0b11000000);
		(PORTC &= 0b01000001);
		_delay_ms(30);
		m+=1;
	}
}

int main()
{
	initiation();
	MasterInit();
	_delay_ms(40000);
	initiate_request_timer();
	//writechar(0b01010111); //W
	
	OCR1A = 0;
	OCR2A = 0;
	
	while(1)
	{
		
		
		if(start_request == 1)
		{
			//writechar(0b01001100); //L
			start_request = 0;
			if(regulateright)
			TransmitSensor(right);
			else if(regulateleft)
			TransmitSensor(left);
			else if(regulateturn)
			TransmitSensor(turn);
			else
			TransmitSensor(0x00);
			
			start_request = 0;
			TCCR0B = 0b0000101; // Start timer
			
		}
		
		//writechar(0b01010010); //R
		sensor1r = ((1/storedValues[1]) - 0.000741938763948) / 0.001637008132828;
		sensor2r = ((1/storedValues[2]) - 0.000741938763948) / 0.001637008132828;
		if (storedValues[1] != 0)
		{
			writechar(0b00100001);//!
			_delay_ms(2000);
		}
		if(sensor1r < 40)
		{
			writechar(0b00111001);//9
			_delay_ms(1000);
		}
		if (sensor1r < 30)
		{
			writechar(0b00111000);//8
			_delay_ms(1000);
		}
		if (sensor1r < 20)
		{
			writechar(0b00110111);//7
			_delay_ms(1000);
		}
		if(sensor1r < 15)
		{
			writechar(0b00110110);//6
			_delay_ms(1000);
		}
		if(sensor1r < 10)//5
		{
			writechar(0b00110101);
			_delay_ms(1000);
		}
		if (sensor1r < 0)
		{
			writechar(0b00110000);//0
			_delay_ms(1000);
		}
		sensorfront = ((1/storedValues[0]) -  0.001086689563586) / 0.000191822821525;
		sensordiff = sensor1r-sensor2r;
		sensormeanr = (sensor1r+sensor2r)/2;

		//writechar(0b01001111); //O
		
			if(sensordiff<20) //Byt plats på höger och vänster för att reglera mot vänster vägg
			{
				PORTC = 0x01;
				PORTD = 0x40;
				OCR1A = 150+K*(9-sensormeanr); //PWM höger
				OCR2A = 150-K*(9-sensormeanr); //PWM vänster
			}
			else if(sensormeanr<5)
			{
				OCR1A = 0;
				OCR2A = 0;
			}
			else
			{
				OCR1A = 0;
				OCR2A = 0;
			}
					
	}
	return 0;
}

ISR(TIMER0_COMPB_vect)
{
	TCCR0B = 0b0000000; //stop timer
	TCNT0 = 0x00;
	start_request = 1;
	//writechar(0b01010111); //W
}
