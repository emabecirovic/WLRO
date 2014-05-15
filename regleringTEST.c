#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <math.h>
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
char stop = 0x00; //Stop byte
char button = 0x00;
char direction = 0b00001001;
char rightspeed = 0b00001010;
char leftspeed = 0b00001011;
char start_request = 0;
//Control signals
char right = 1;
char left = 2;
char turn = 3;
char turnstop = 4;
bool remoteControl = false;  // Change to Port connected to switch
//bool regulateright = true;
bool regulateleft = false;
bool regulateturn = false;
int time = 200;

volatile unsigned char storedValues[11];
float sensor1r, sensor2r, sensordiff, sensorfront, sensorleft, sensorright;
volatile float sensormeanr;
volatile float sensormeanr_old;
int K;
long Td;
volatile float rightpwm;
volatile float leftpwm;
volatile float emadistance = 0;
float traveled=0;
long overflow = 0;
long dt = 0;
int timer = 0;
char speed = 50;

bool finished=0; //1 då hela kartan utforskad
bool onelap=0; //1 då yttervarvet körts
bool home=0; //1 då robten återvänt till startposition
bool awaydone, finsempptydone, zigzagdone;
unsigned int myposX=0; //Robotens position i X-led
unsigned int myposY=0; //Robotens position i Y-led
unsigned int startpos[2]={15,0}; //Startpositionen sätts till mitten på nedre långsidan

//LCD-saker :)
char lcd0 = 0b00110000;
char lcd1 = 0b00110001;
char lcd2 = 0b00110010;
char lcd3 = 0b00110011;
char lcd4 = 0b00110100;
char lcd5 = 0b00110101;
char lcd6 = 0b00110110;
char lcd7 = 0b00110111;
char lcd8 = 0b00111000;
char lcd9 = 0b00111001;
char lcda = 0b01000001;
char lcdb = 0b01000010;
char lcdc = 0b01000011;
char lcdd = 0b01000100;
char lcde = 0b01000101;
char lcdf = 0b01000110;
char lcdspace = 0b00100000;

char turnready = 0;
char mydirection = 2;

char dummy;
char gyrostop = 0b10000000;

void initiate_variables()
{
	front = 0b00000001;
	rightfront = 0b00000010;
	rightback = 0b00000011;
	leftfront = 0b00000100;
	leftback = 0b00000101;
	traveldist = 0b00000110;
	gyro = 0b00000111;
	RFID = 0b00001000;
	stop = 0x00; //Stop byte
	button = 0x00;
	direction = 0b00001001;
	rightspeed = 0b00001010;
	leftspeed = 0b00001011;
	start_request = 0;
	//Control signals
	right = 1;
	left = 2;
	turn = 3;
	turnstop = 4;
	remoteControl = false;  // Change to Port connected to switch
	//bool regulateright = true;
	regulateleft = false;
	regulateturn = false;
	time = 200;

	emadistance = 0;
	traveled=0;
	overflow = 0;
	dt = 0;
	timer = 0;
	speed = 50;

	finished=0; //1 då hela kartan utforskad
	onelap=0; //1 då yttervarvet körts
	home=0; //1 då robten återvänt till startposition
	myposX=0; //Robotens position i X-led
	myposY=0; //Robotens position i Y-led
	startpos[0] = 15; //Startpositionen sätts till mitten på nedre långsidan
	startpos[1] = 0;

	//LCD-saker :)
	lcd0 = 0b00110000;
	lcd1 = 0b00110001;
	lcd2 = 0b00110010;
	lcd3 = 0b00110011;
	lcd4 = 0b00110100;
	lcd5 = 0b00110101;
	lcd6 = 0b00110110;
	lcd7 = 0b00110111;
	lcd8 = 0b00111000;
	lcd9 = 0b00111001;
	lcda = 0b01000001;
	lcdb = 0b01000010;
	lcdc = 0b01000011;
	lcdd = 0b01000100;
	lcde = 0b01000101;
	lcdf = 0b01000110;
	lcdspace = 0b00100000;

	turnready = 0;
	mydirection = 2;
}

void MasterInit(void)
{
	/* Set MOSI and SCK output, alla others input*/
	/* Ersätt DDR_SPI med den port "serie" som används ex DD_SPI -> DDRB
	samt DD_MOSI och DD_SCK med specifik pinne ex DD_MOSI -> DDB5 */
	DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB7);

	/* Enable SPI, Master, set clock rate fosc/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(0<<SPI2X)|(1<<SPR1)|(0<<SPR0)|(1<<CPHA)|(1<<CPOL);

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
			for(int i = 0; i < time; i++){}
			dummy=SPDR;
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

void TransmitComm()
{
	PORTB &= 0b11110111;

	for(int i = 0; i < time; i++){}
	for(int i = 0; i < 11; i ++)
	{
		dummy = SPDR;
		MasterTransmit(storedValues[i]);
		for(int i = 0; i < time; i++){}
	}

	PORTB ^= 0b00001000;
}

void initiate_request_timer()
{
	TIMSK0 = 0b00000100; //Enable interupt vid matchning med OCR0B
	TCNT0 = 0x00;
	TCCR0B = 0b0000101; //Starta räknare, presscale 1024
	OCR0B = 0xFF; // 255
	// 261120 st klockcykler
}

void initiate_timer()
{
	TIMSK1 = 0b00000001; //Enable interupt vid overflow

	TCCR1B = 0x00; //stop
	TCNT1 = 0x00; //set count

	TCCR1B = 0x03; //start timer prescale 64
}

void initiation()
{
	//Sätter utgångar/ingångar
	DDRA=0b11111111;
	DDRC=0b11000001;
	DDRD=0b11100000;
	//TCCR1A=0b10000001; //setup, phase correct PWM
	//TCCR1B=0b00000010; //sätter hastigheten på klockan
	TCCR2A=0b10100001;
	TCCR2B=0b00000010;

	//Till displayen, vet inte om det behövs men den är efterbliven
	PORTA=0b00110000;
	PORTC=0b00000000;
	_delay_ms(200);
	PORTA=0b00110000;
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_ms(50);
	PORTA=0b00110000;
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_us(1100);
	//Startar initiering
	PORTA=0b00110000; // 1-line mode ; 5x8 Dots
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_us(4000);
	PORTA=0b00001111; // Display on ; Cursor on ; Blink on
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_us(4000);
	PORTA=0b00000001; // Clear display
	PORTC=0b10000000;
	PORTC=0b00000000;
	_delay_ms(200);
	PORTA=0b00000111; //Increment mode ; Entire shift on
	PORTC=0b10000000;
	_delay_ms(200);
	//Initiering klar



	storedValues[6] = 0;

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

void shiftcursorleft()
{
	PORTA=0b00010000; //Shift cursor left
	(PORTC |= 0b11000000);
	(PORTC &= 0b01000001);
	_delay_ms(30);
}

//------------------------------------------------------------------------------

char what_lcd_number(char number)
{
	switch (number)
	{
		case 0:
		return lcd0;
		break;
		case 1:
		return lcd1;
		break;
		case 2:
		return lcd2;
		break;
		case 3:
		return lcd3;
		break;
		case 4:
		return lcd4;
		break;
		case 5:
		return lcd5;
		break;
		case 6:
		return lcd6;
		break;
		case 7:
		return lcd7;
		break;
		case 8:
		return lcd8;
		break;
		case 9:
		return lcd9;
		break;
		case 10:
		return lcda;
		break;
		case 11:
		return lcdb;
		break;
		case 12:
		return lcdc;
		break;
		case 13:
		return lcdd;
		break;
		case 14:
		return lcde;
		break;
		case 15:
		return lcdf;
		break;
		default:
		return lcd0;
		break;
	}
}

void print_on_lcd(char number)
{
	char highnumber = number >> 4;
	highnumber = highnumber & 0b00001111;
	char lownumber = number & 0b00001111;

	writechar(what_lcd_number(highnumber));
	//shift(1);
	writechar(what_lcd_number(lownumber));
	//shift(1);
	writechar(lcdspace);
	//shift(1);
}

void setcursor(char place) //16 platser på en rad. 0x00-0x0F
{
	
	PORTA=(0x80 + place - 0x01);
	(PORTC |= 0b10000000);
	(PORTC &= 0b00000001);
	_delay_ms(200);
}

//-------------------------------------------------------------------------------------
void stopp()
{
	OCR2A = 0;
	OCR2B = 0;
}

void rotateleft()
{
	PORTC = 0x00;
	PORTD = 0x20;
	OCR2B = 170;
	OCR2A = 170;
}

void rotateright()
{
	PORTC = 0x01;
	PORTD = 0x00;
	OCR2B = 170;
	OCR2A = 170;
}

void rotate90left()
{
	volatile  int isDone = 0;
	while(isDone == 0)
	{
		TransmitSensor(turn);
		if (storedValues[6] != 1)
		{
			rotateleft();
		}
		else
		{
			start_request = 1;
			TransmitSensor(turnstop);
			stopp();
			isDone = 1;
		}

	}
	if(mydirection==4)
	{
		mydirection=1;
	}
	else
	{
		mydirection+=1;
	}
}

void rotate90right()
{
	volatile int isDone = 0;
	while(isDone == 0)
	{
		TransmitSensor(turn);
		if (storedValues[6] != 2)
		{
			rotateright();
		}
		else
		{
			start_request = 1;
			TransmitSensor(turnstop);
			stopp();
			isDone = 1;
		}
	}
	if(mydirection==1)
	{
		mydirection=4;
	}
	else
	{
		mydirection-=1;
	}
}

void temporary90right()
{
	cli();
	PORTC = 0x01;
	PORTD = 0x00;
	OCR2B = 110;
	OCR2A = 110;
	_delay_ms(7000);
	sei();
}

void temporary90left()
{
	cli();
	PORTC = 0x00;
	PORTD = 0x20;
	OCR2B = 110;
	OCR2A = 110;
	_delay_ms(7000);
	sei();
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
	
	while (emadistance < dist * 0.9)
	{
		transmit();
		driveF();
	}
	stopp();
}

void drivefromstill(float dist) //kör dist cm
{
	emadistance=0;
	dist = dist / 2.55125;
	//dist = dist / 2.55;
	
	while (emadistance < dist * 1.05)
	{
		transmit();
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
	stopp();
	transmit();
	
	sensorleft = sidesensor(storedValues[3]);
	
	if(sensorleft<20)
	{
		rotate90left();
		rotate90left();
	}
	else
	{
		rotate90left();
	}
}

void remotecontrol()
{
	cli();
	DDRB = 0x00;
	PINB = 0x00;
	stopp();
	print_on_lcd(0xff);
	
	while(1)
	{
		button = PINB & 0b00000111;
		//print_on_lcd(button);
		setcursortostart();
		
		/*
		1 = W
		2 = D
		3 = A
		4 = S
		5 = E
		6 = Q
		*/
		
		/*switch(button)
		{
			case (0x01)://Kör framåt, W
			PORTC = 0x01; //Sätter båda DIR till 1
			PORTD = 0x20;
			OCR2A = 255; //PWM vänster
			OCR2B = 244; //PWM höger
			break;
			case (0x04): //Backa, S
			PORTC = 0x0; //Sätter båda DIR till 0
			PORTD = 0x0;
			OCR2A = 255;
			OCR2B = 244;
			break;
			case (0x06): //Rotera vänster, Q
			PORTC = 0x00; //DIR vänster till 0
			PORTD = 0x20; //DIR höger till 1
			OCR2A = 170;
			OCR2B = 170;
			break;
			case (0x05): //Rotera höger, E
			PORTC = 0x01;
			PORTD = 0x00;
			OCR2A = 170;
			OCR2B = 170;
			break;
			case (0x03): //Sväng vänster, A
			PORTC = 0x01;
			PORTD = 0x20;
			OCR2A = 120;
			OCR2B = 255;
			break;
			case (0x02): //Sväng höger, D
			PORTC = 0x01;
			PORTD = 0x20;
			OCR2A = 255;
			OCR2B = 125;
			break;
			default:
			stopp();
		}*/
	}
}

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

void firstlap()
{
	if(myposX == startpos[0] && myposY == startpos[1])	//Det här kommer gälla de första sekunderna roboten börjar köra också..!
	{
		onelap=1;
	}
	else
	{
		//regulateright();
	}
}

void rfid()
{ 
	storedValues[7] = 0;
	setcursortostart();
	volatile int bajs = 1;
	while(bajs==1)
	{
		
		transmit();
		print_on_lcd(storedValues[7]);
		if(storedValues[7] != 1)
		{
			PORTC = 0x01;
			PORTD = 0x20;
			OCR2B = 50;
			OCR2A = 50;
			setcursortostart();
		}
		else
		{
			stopp();
			bajs=0;
			setcursortostart();
			
		}
		
		
	}
}

int main()
{
	initiate_variables();
	initiation();
	int fjarrstyrt = (PIND & 0x01); //1 då roboten är i fjärrstyrt läge
	initiate_timer();
	MasterInit();
	_delay_ms(40000);
	initiate_request_timer();

	if(fjarrstyrt==1)
	{
		//drivefromstill(40);
		//rotate90left();
		//rfid();
		remotecontrol();
	}
	
	else
	{
		
		/*while(1) //Ta bort för att kunna köra autonomt
		{
			stopp();
		}*/
	
		int first=1;
		char start=0;

		
		while(1)
		{
			if(first==0)
			{
				sensormeanr_old=sensormeanr;
			}
			transmit();
			
			setcursortostart();
			print_on_lcd(mydirection);
			setcursortostart();

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
				Td = 128000000; //128000000
				K = 3;

				if(sensorfront<50)
				{
					drive(40);
					transmit();
					sensorright = sidesensor(storedValues[1]);

					if(sensorright>20)
					{
						rotate90right();
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
						dt = (timer + overflow * 65536) * 64;


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
						rotate90right();
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
}
