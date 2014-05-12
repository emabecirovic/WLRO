/*
* Search.c
*
* Created: 4/29/2014 9:26 AM
*  Author: robsv107
*          patsu326
*		   marek588
*/

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL

typedef int bool;
enum{false, true};


//Start bytes for transition (simplification when coding)
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveldist = 0b00000110;
char gyro = 0b00000111;
char gyrostop = 0b10000000;
char RFID = 0b00001000;
char direction = 0b00001001;
char rightspeed = 0b00001010;
char leftspeed = 0b00001011;
char stop = 0x00; //Stop byte

unsigned char storedValues[5];
char traveled = 0;
bool RFID;
char gyro;



char speed=150; //Standardhastiget i autonomt läge
bool finished=0; //1 då hela kartan utforskad
bool onelap=0; //1 då yttervarvet körts
bool home=0; //1 då robten återvänt till startposition
bool fisklar, pruttklar, shartklar;
char mydirection = 2; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
char distance=0; //Hur långt roboten har färdats (sedan senast vi tog emot värden från sensor?)
unsigned int myposX=0; //Robotens position i X-led
unsigned int myposY=0; //Robotens position i Y-led
unsigned int startpos[2]={15,0}; //Startpositionen sätts till mitten på nedre långsidan
int firstzero; //Första nollan om man läser matrisen uppifrån och ned

unsigned char gyro;
unsigned char sensorfront;
unsigned char sensor1right;
unsigned char sensor2right;
unsigned char sensor1left;
unsigned char sensor2left;

char room[29][15]; //=.... 0=outforskat, 1=vägg, 2=öppen yta


/***********************************************LCDSKÄRM*********************************/
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

void writechar(unsigned char data)
{
	PORTA=data;
	(PORTC |= 0b11000000);
	(PORTC &= 0b01000001);
	_delay_ms(30);
}


/*********************************BUSSFUNKTIONER******************************/
void MasterInit(void)
{
	/* Set MOSI and SCK output, alla others input*/
	/* Ersätt DDR_SPI med den port "serie" som används ex DD_SPI -> DDRB
	samt DD_MOSI och DD_SCK med specifik pinne ex DD_MOSI -> DDB5 */
	DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB7);
	
	/* Enable SPI, Master, set clock rate fosc/64 and mode 3 */
	SPCR = (1<<SPE)|(1<<MSTR)(1<<SPI2X)|(1<<SPR1)|(1<<SPR0)|(1<<CPHA)|(1<<CPOL);
	
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
	
	//SPSR = (1<<SPIF);
}

void bussdelay()
{
	for(int i = 0; i < time; i++){}
}


void TransmitSensor(char invalue)
{
	if(start_request == 1)
	{
		start_request = 0;

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
			MasterTransmit(gyrostop);
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
			MasterTransmit(rightfront);
			for(int i = 0; i < time; i++){}
			storedValues[0] = SPDR; // Front
			MasterTransmit(rightback);
			for(int i = 0; i < time; i++){}
			storedValues[1] = SPDR; // Right front
			MasterTransmit(leftfront);
			for(int i = 0; i < time; i++){}
			storedValues[2] = SPDR; // Right back
			MasterTransmit(leftback);
			for(int i = 0; i < time; i++){}
			storedValues[3] = SPDR; // Left front
			MasterTransmit(stop);
			for(int i = 0; i < time; i++){}
			storedValues[4] = SPDR; // Left back
		}

		PORTB ^= 0b00010000; // ss2 high

		emadistance += storedValues[5];

		TCCR0B = 0b00000101; // Start timer
	}
}

void getAllSensor()
{
	PORTB &= 0b11101111; // ss2 low
	
	for(int i = 0; i < time; i++){}
	MasterTransmit(RFID);
	//First communication will contain crap on shift register
	for(int i = 0; i < time; i++){}
	MasterTransmit(traveldist); // Request front sensor
	for(int i = 0; i < time; i++){}
	RFID = SPDR; // SensorRFID
	MasterTransmit(front); // Request front sensor
	for(int i = 0; i < time; i++){}
	traveled = SPDR; // Distance
	MasterTransmit(rightfront);
	for(int i = 0; i < time; i++){}
	storedValues[0] = SPDR; // Front
	MasterTransmit(rightback);
	for(int i = 0; i < time; i++){}
	storedValues[1] = SPDR; // Right front
	MasterTransmit(leftfront);
	for(int i = 0; i < time; i++){}
	storedValues[2] = SPDR; // Right back
	MasterTransmit(leftback);
	for(int i = 0; i < time; i++){}
	storedValues[3] = SPDR; // Left front
	MasterTransmit(gyro);
	for(int i = 0; i < time; i++){}
	storedValues[4] = SPDR; // Left back
	MasterTransmit(stop);
	for(int i = 0; i < time; i++){}
	gyro = SPDR; // Gyro
	
	PORTB ^= 0b00010000; // ss2 high
	
	
	
}

/******************************FJÄRRSTYRNING**********************/
void remotecontrol()
{
	while(1)
	{
		char button; //Ta emot styrdata
		
		switch(button)
		{
			case (0x01)://Kör framåt, W
			PORTC = 0x01; //Sätter båda DIR till 1
			PORTD = 0x40;
			OCR2A = 250; //PWM vänster
			OCR1A = 250; //PWM höger
			writechar(0b01010111); //W
			break;
			case (0x04): //Backa, S
			PORTC = 0x0; //Sätter båda DIR till 0
			PORTD = 0x0;
			OCR2A = 250;
			OCR1A = 250;
			writechar(0b01010011); //S
			break;
			case (0x06): //Rotera vänster, Q
			PORTC = 0x00; //DIR vänster till 0
			PORTD = 0x40; //DIR höger till 1
			OCR2A = 250;
			OCR1A = 250;
			writechar(0b01010001); //Q
			break;
			case (0x05): //Rotera höger, E
			PORTC = 0x01;
			PORTD = 0x00;
			OCR2A = 250;
			OCR1A = 250;
			writechar(0b01000101); //E
			break;
			case (0x03): //Sväng vänster, A
			PORTC = 0x01;
			PORTD = 0x40;
			OCR2A = 100;
			OCR1A = 250;
			writechar(0b01000001); //A
			break;
			case (0x02): //Sväng höger, D
			PORTC = 0x01;
			PORTD = 0x40;
			OCR2A = 250;
			OCR1A = 100;
			writechar(0b01000100); //D
			break;
			default:
			OCR2A = 0;
			OCR1A = 0;
		}
	}
}

/****************************POSITIONSHANTERING*********************/
void updatepos()
{
	switch(mydirection)
	{
		case (1): // X+
		{
			myposX+=1;
			//mypos[0]=myposX;
			traveled=0;
		}
		case (2): // Y+
		{
			myposY+=1;
			//mypos[1]=myposY;
			traveled=0;
		}
		case (3): // X-
		{
			myposX-=1;
			//mypos[0]=myposX;
			traveled=0;
		}
		case (4): // Y-
		{
			myposY-=1;
			//mypos[1]=myposY;
			traveled=0;
		}
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

	int sensorfront;
	int sensormeanright;
	int sensormeanleft;

	switch(mydirection)
	{
		case (1): // X+
		if(sensormeanright<=w) //Vet inte vad som är en lämplig siffra här
		{
			setwall(myposX,myposY-1);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX,myposY+1);
		}
		if (!room[myposX-1][myposY]==(1|4))
		{
			room[myposX-1][myposY]=2;
		}
		break;
		
		case (2): // Y+
		if(sensormeanright<=w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX-1,myposY);
		}
		
		if (!room[myposX][myposY-1]==(1|4))
		{
			room[myposX][myposY-1]=2;
		}
		break;
		
		case (3): // X-
		if(sensormeanright<=w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX,myposY-1);
		}

		if (!room[myposX+1][myposY]==(1|4))
		{
			room[myposX+1][myposY]=2;
		}
		break;
		
		case (4): // Y-
		if(sensormeanright<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY-1);
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX+1,myposY);
		}
		if (!room[myposX][myposY+1]==(1|4))
		{
			room[myposX][myposY+1]=2;
		}
		break;
	}
	if (isRFID==1)
	{
		room[myposX][myposY]=4
	}
}

/*********************************RITA UT FÖRLÄNGD VÄGG*************************************/
void extended_wall()
{
	for(int j = 0; j < 15; j++ )
	{
		for(int i = 0; i < 29; i++)
		{
			if(room[i][j] == 1 | 2)
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
			if(room[i][j] == 1 | 2)
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
			if(room[i][j] == 1 | 2)
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
			if(room[i][j] == 1 | 2)
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

/********************************REGLERING*****************************************/

void driveF()
{
	PORTC = 0x01;
	PORTD = 0x20;
	OCR2A = speed;
	OCR2B = speed;
}

// Hjälpfunktion för att köra en viss distans
void driveDist(char length)
{
	distance = 0;
	while(distance < length)
	{
		TransmitSensor(0x00);
		driveF();
	}
}
/*********************************FÖRSTA VARV*************************************/
void firstlap()
{
	if(mypos==startpos)	//Det här kommer gälla de första sekunderna roboten börjar köra också..!
	{
		onelap=1;
	}
	else
	{
		regleringright();
	}
}

void initiate_timer()
{
	TIMSK1 = 0b00000001; //Enable interupt vid overflow

	TCCR1B = 0x00; //stop
	TCNT1 = 0x00; //set count

	TCCR1B = 0x03; //start timer prescale 64
}


void stopp()
{
	OCR2A = 0;
	OCR2B = 0;
}

void straight()
{
	if((sensor1r-sensor2r) > 0.5)
	{
		PORTC = 0x01; //rotera höger
		PORTD = 0x00;
		OCR2A = 70;
		OCR2B = 70;
	}
	else if((sensor2r-sensor1r) > 0.5)
	{
		PORTC = 0x00; //rotera vänster
		PORTD = 0x20;
		OCR2A = 70;
		OCR2B = 70;
	}
}

void transmit()
{
	TransmitSensor(0);
	TransmitComm();
}

void temporary90right()
{
	cli();
	rotateright();
	_delay_ms(7000);
	sei();
	if(mydirection==1)
	{
		ready=1;
		mydirection=4;
	}
	else
	{
		ready=1;
		mydirection-=1;
	}
}

void temporary90left()
{
	cli();
	rotateleft();
	_delay_ms(7000);
	sei();
	if(mydirection==4)
	{
		ready=1;
		mydirection=1;
	}
	else
	{
		ready=1;
		mydirection+=1;
	}
}

void driveF()
{
	PORTC = 0x01;
	PORTD = 0x20;
	OCR2B = speed;
	OCR2A = speed;
}

void drive(float dist) //kör dist cm
{
	emadistance=0;
	dist = dist / 2.55125;
	//dist = dist / 2.55;
	
	while (emadistance < dist * 1)
	{
		transmit(0);
		driveF();
	}
	stopp();
}

void drivefromstill(float dist) //kör dist cm
{
	emadistance=0;
	dist = dist / 2.55125;
	//dist = dist / 2.55;
	
	while (emadistance < dist * 1.6)
	{
		transmit(0);
		driveF();
	}
	stopp();
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

void leftturn()  //Används när man vet att det är vägg framför och vägg till höger för att kolla om man ska svänga vänster eller vända om helt
{
	transmit(0);
	
	sensorright = sidesensor(storedValues[1]);
	sensorleft = sidesensor(storedValues[3]);
	
	if(sensorleft<20)
	{
		temporary90left();
		temporary90left();
	}
	else
	{
		temporary90left();
	}
}

void rotateleft()
{
	PORTC = 0x00;
	PORTD = 0x20;
	OCR2B = 145;
	OCR2A = 145;
}

void rotateright()
{
	PORTC = 0x01;
	PORTD = 0x00;
	OCR2B = 110;
	OCR2A = 110;
}

void regulateright()
{
	

	int first=1;
	char start=0;

	if(first==0)
	{
		sensormeanr_old=sensormeanr;
	}
	transmit();
	
	//REGLERING
	//Omvandling till centimeter
	
	sensor1r = sidesensor(storedValues[1]);
	sensor2r = sidesensor(storedValues[2]);
	sensorfront = frontsensor(storedValues[0]);
	sensormeanr = ((sensor1r + sensor2r) / 2) + 9;

	if(first==1)
	{
		first=0;
		stopp();
		sensormeanr_old=sensormeanr;
	}
	else
	{
		//till PD-reglering
		Td = 400000;
		K = 4;

		if(sensorfront<50)
		{
			drive(40);
			transmit();
			sensorright = sidesensor(storedValues[1]);
			
			if(sensorright>20)
			{
				temporary90right();
				transmit();
				sensorfront = frontsensor(storedValues[0]);
				
				if(sensorfront<60)
				{
					drivefromstill(40);
					leftturn();
				}
				else
				{
					drivefromstill(40);
				}
			}
			else
			{
				leftturn();
			}
		}
		else if(((sensor1r-sensor2r) < 20) && ((sensor2r-sensor1r) < 20))
		{
			if (fabs(sensor1r-sensor2r) > 3)
			{
				straight();
			}
			else
			{
				start=1;
				timer = TCNT1;
				dt = (time + overflow * 65536) * 64;
				

				TCNT1 = 0;
				overflow = 0;
				
				PORTC = 0x01;
				PORTD = 0x20;
				rightpwm = speed + K * (18-sensormeanr + Td * (sensormeanr_old-sensormeanr)/dt);
				leftpwm = speed - K * (18-sensormeanr + Td * (sensormeanr_old-sensormeanr)/dt);


				if (rightpwm > 255)
				{
					OCR2B = 255;
				}
				else if(rightpwm < 0)
				{
					OCR2B = 0;
				}
				else
				{
					OCR2B = rightpwm;
				}
				if (leftpwm > 255)
				{
					OCR2A = 255;
				}
				else if (leftpwm < 0)
				{
					OCR2A = 0;
				}
				else
				{
					OCR2A = leftpwm;
				}

			}
		}
		else
		{
			if(start==1)
			{
				start=0;
				drive(20);
				temporary90right();
				transmit();
				sensorfront = frontsensor(storedValues[0]);
				
				if(sensorfront>80)
				{
					drivefromstill(40);
				}
				else
				{
					drivefromstill(40);
					leftturn();
				}
			}
			else
			{
				stopp();
			}
		}


	}
	
}


void regleringleft()
{
	PORTC = 0x01; //sätter båda DIR till 1
	PORTD = 0x40;
	int K = 5; //Bestämmer hur snabbt roboten ska reagera på skillnader mellan önskat avstånd till väggen(10cm) och uppmätt avstånd
	
	
	while(regleramotvagg==1)
	{
		int sensordiffr = sensor1l-sensor2l;
		int sensormeanr = (sensor1l+sensor2l)/2;
		
		if (sensorfront>50)
		{
			if(sensordiff<20)
			{
				OCR1A = 180-K*(9-sensormeanr); //PWM höger
				OCR2A = 180+K*(9-sensormeanr); //PWM vänster
			}
			else //sensordiff>20 innebär att ett hörn kommer
			{
				while(traveled<20)
				{
					OCR2A = 180;
					OCR1A = 180;
				}
				rotate90left();
				while(traveled<20)
				{
					OCR2A = 180;
					OCR1A = 180;
				}
			}
		}
		else
		driveDist(20);
		rotate90right();
	}
}


/*************************************Öppna Ytor********************************/

void away() // Få roboten från väggen
{
	TransmitSensor(left);

	/* Vänd roboten posetiv x-led
	while(mydirection != 3)
	{
	if(mydirection < 3)
	rotate90right();
	else
	rotate90left();
	}*/

	if(leftfront < 20)
	{
		regleringright();
		bool getinpos = true;

	}
	else if(getinpos)
	{
		driveDist(20);
	}
	else
	{
		rotate90left();

		driveDist(40); // Kör en sektion ut i öppen yta

		rotate90right();

		awaydone = true;
	}
}

void zigzag() //sicksacksak
{
	if(sensorfront>60)
	{
		PORTC = 0x01;
		PORTD = 0x40;
		OCR2A = speed;
		OCR1A = speed;
	}
	else if(first)
	{
		first = false;
		rotate90left2();
	}
	else if(leftturn)
	{
		TransmitSensor(left);
		leftturn = false;
		if(sensorvanster < 20)
		{
			zigzagdone = true;
			return;
		}
		rotate90left2();
		driveDist(40);
		while(sensorvanster < 20)
		{
			driveDist(40);
		}
		rotate90left2();
	}
	else
	{
		TransmitSensor(right);
		leftturn = true;
		if(sensorhoger < 20)
		{
			zigzagdone = true;
			return;
		}
		rotate90right2();
		driveDist(40);
		while(sensorhoger < 20)
		{
			driveDist(40);
		}
		rotate90right2();
	}
}

/*********************************MISSADE RUTOR*******************************/
void findempty()
{
	int notsearched[2] = findfirstzero();

	if(notsearched == [15, 0]) //KOLLA UPP COMPARE ARRAY!!!
	findemptydone = true;
	else
	driveto(notsearched);
}


/************************************HITTA FÖRSTA NOLLAN I RUMMET**********************************/
int findfirstzero()
{
	int i; //X
	int j; //Y
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
	return firstzero;
}

void driveto(int pos[2])
{
	getAllSensor();
	if(myposX == pos[0]  && myposY  == pos[1])
	room[myposX, myposY] = 2;
	if else(myposX >= pos[0] && myposY <= pos[1]) //Fjärde kvadranten
	{
		switch(mydirection)
		{
			case(1):
			rotate90left();
			case(2):
			if(sensorfront > 50)
			driveF();
			else if((pos[1]-myposY) == 1)
			{
				driveDist(40);
				rotate90left();
			}
			case(3):
			if(sensorfront > 50)
			driveF();
			else if(myposX-pos[0]) == 1)
			{
				driveDist(40);
				rotate90right();
			}
			case(4):
			rotate90right();
		}
	}
	else if(myposX <= pos[0] && myposY <= pos[1]) // Tredje kvadranten KOLLA HÄR OM DET FUNKAR
	{
		switch(mydirection)
		{
			case(1):
			if(sensorfront > 50)
			driveF();
			else if((pos[0]-myposX) == 1)
			driveDist(40);
			case(2):
			if(sensorfront > 50)
			driveF();
			else if((pos[1]-myposY) == 1)
			driveDist(40);
			case(3):
			rotate90right();
			case(4):
			rotate90left();
		}
	}
	else if(myposX < pos[0] && myposY > pos[1])
	{
		switch
		{
			case(1):
			if(sensorfront > 50)
			driveF();
			case(2):
			rotate90right();
			case(3):
			rotate90left();
			case(4):
			if(sensorfront > 50)
			driveF();
		}
	}
	else
	{
		switch
		{
			case(1):
			rotate90right();
			case(2):
			rotate90left();
			case(3):
			if(sensorfront < 50)
			driveF();
			case(4):
			if(sensorfront < 50)
			driveF();
		}
	}
}


void returntostart()
{
	int mydirection; //Robotens riktning
	int myposX; //Rpbotens position i X-led
	int starposX; //Starpositionens värde i X-led

	if(mydirection=4) //4=negativ y-led. x+,y+,x-,y- = 1,2,3,4
	{
		while(sensorfram>10)
		{
			//Kör rakt fram
			PORTC = 0x01;
			PORTD = 0x40;
			OCR2A = 180;
			OCR1A = 180;
		}
		if(myposX<startposX) //Om ingången är till höger om roboten
		{
			while(myposX<startposX)
			{
				regleringright();
			}
		}
		else
		{
			while(myposX>startposX) //Om ingången är till vänster om roboten
			{
				regleringleft();
			}
		}
	}
	else
	{
		while(mydirection!=4)
		{
			rotate90left();  //Rotera 90 grader om vi står i fel riktning
		}
	}
}


int main(void)
{
	MasterInit();
	Initiation();
	_delay_ms(40000);
	initiate_request_timer();

	getAllSensor();

	int fjarrstyrt = (PIND & 0x01); //1 då roboten är i fjärrstyrt läge

	if(fjarrstyrt==1)
	{
		int button=PINB;
		remotecontrol();
	}
	else
	{
		bool leftturn = true; // Till första toppsvängen i sicksacksak
		bool first = true; // Till första bottensväng i sicksacksak

		bool drivetoY = true; // Y-led är prioriterad riktining om sant i driveto

		while(home==0)
		{

			if(traveled >= 40/0.8125) //dividera med sektor 0.8125
			updatepos();


			if(onelap==0)
			{
				firstlap();
			}
			else if(!awaydone)
			{
				away();
			}
			else if(!zigzagdone)
			{
				zigzag();
			}
			else if(!findemptydone)
			{
				findempty();
			}
			else
			{
				returntostart();
			}
		}
	}
	return 0;
}


ISR(TIMER1_OVF_vect)
{
	TCNT1 = 0;
	overflow++;
}


ISR(TIMER0_COMPB_vect)
{
	TCCR0B = 0b0000000; //stop timer
	TCNT0 = 0x00;
	start_request = 1;
	//writechar(0b01010111); //W
}
