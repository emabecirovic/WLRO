/*
* Search.c
*
* Created: 4/29/2014 9:26 AM
* Author: robsv107
* patsu326
* marek588
*/

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include "styrRobot.h"

#define F_CPU 1000000UL

/**************** INITIERING******************/
void initiate_variables()
{
	/************BUSS**********************/
	
	// Start för busskom
	start_request = 0;

	// Array för värden från buss
	storedValues[11];


	/******************REGLERING************************/

	distance = 0; // Avlagdsträcka
	overflow = 0; // Räknat för långt
	dt = 0;
	timer = 0;

	speed = 50;

	turnisDone = 0;


	// Flaggor för regulateright
	firstRR=1;
	startregulate = 0;

	/**************POSITION******************/
	mydirection = 1; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
	myposX=15; //Robotens position i X-led
	myposY=0; //Robotens position i Y-led
	startpos[0] = 15; //Startpositionen sätts till mitten på nedre långsidan
	startpos[1] = 0;
	posdistance = 0;


	/***************FLAGGOR FÖR MAIN******************/
	start = 1; //vi står i startpositionen

	finished=0; //1 då hela kartan utforskad
	onelap=0; //1 då yttervarvet körts
	home=0; //1 då robten återvänt till startposition


	zzleftturn = true; // Till första toppsvängen i sicksacksak
	zzfirst = true; // Till första bottensväng i sicksacksak

	drivetoY = true; // Y-led är prioriterad riktining om sant i driveto
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

/*************************LCDSKÄRM******************/

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
	for(long i = 0; i < 2000; i ++){}
}

/***************************************BUSSFUNKTIONER*************************/

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

void transmit()
{
	TransmitComm();
	TransmitSensor(0);
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

		if(invalue != trstraight)
		{
			distance = distance + storedValues[5];
			posdistance = posdistance + storedValues[5];
			storedValues[5] = 0;
		}

		TCCR0B = 0b00000101; // Start timer
	}
}



void TransmitComm()
{
	if(start_request == 0)
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
		
		TCCR0B = 0b00000101; // Start timer
	}
}


/***********************************FJÄRRSTYRNING*****************************/

void remotecontrol()
{
	cli();
	DDRB = 0x00;
	PINB = 0x00;
	stopp();

	while(1)
	{
		button = PINB & 0b00000111;
		//print_on_lcd(button);

		/*
		1 = W
		2 = D
		3 = A
		4 = S
		5 = E
		6 = Q
		*/

		setcursor(1);
		print_on_lcd(button);
		switch(button)
		{
			case (1)://Kör framåt, W
			PORTC = 0x01; //Sätter båda DIR till 1
			PORTD = 0x20;
			OCR2A = 255; //PWM vänster
			OCR2B = 244; //PWM höger
			break;
			case (4): //Backa, S
			PORTC = 0x0; //Sätter båda DIR till 0
			PORTD = 0x0;
			OCR2A = 255;
			OCR2B = 244;
			break;
			case (6): //Rotera vänster, Q
			PORTC = 0x00; //DIR vänster till 0
			PORTD = 0x20; //DIR höger till 1
			OCR2A = 100;
			OCR2B = 100;
			break;
			case (5): //Rotera höger, E
			PORTC = 0x01;
			PORTD = 0x00;
			OCR2A = 100;
			OCR2B = 100;
			break;
			case (3): //Sväng vänster, A
			PORTC = 0x01;
			PORTD = 0x20;
			OCR2A = 120;
			OCR2B = 255;
			break;
			case (2): //Sväng höger, D
			PORTC = 0x01;
			PORTD = 0x20;
			OCR2A = 255;
			OCR2B = 125;
			break;
			default:
			stopp();
		}
	}
}

/************************POSITIONSHANTERING**********************/

void updatepos()
{

	start = 0;
	asm("");
	switch(mydirection)
	{
		case (1): // X+
		{
			myposX = myposX + 1;
			break;
		}
		case (2): // Y+
		{
			myposY+=1;
			break;
		}
		case (3): // X-
		{
			myposX-=1;
			break;
		}
		case (4): // Y-
		{
			myposY-=1;
			break;
		}
		default:
		{
		}
	}
	storedValues[8] = myposX;
	storedValues[9] = myposY;
	storedValues[10] = 2; //Besökt ruta, gör för RFID! :D
	posdistance = 0;
}


//-------------------------------------------------------------------------------------
/**************************************STYRNING*************************************/
void stopp()
{
	OCR2A = 0;
	OCR2B = 0;
}

void rotateleft()
{
	PORTC = 0x00;
	PORTD = 0x20;
	OCR2B = 100;
	OCR2A = 100;
}

void rotateright()
{
	PORTC = 0x01;
	PORTD = 0x00;
	OCR2B = 100;
	OCR2A = 100;
}

void rotate90left()
{
	storedValues[6] = 0;
	while(turnisDone == 0)
	{
		start = 1;
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
			turnisDone = 1;
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
	turnisDone = 0;
	for(long i = 0; i < 80000; i ++){stopp();}
}

void rotate90right()
{
	storedValues[6] = 0;
	while(turnisDone == 0)
	{
		start_request = 1;
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
			turnisDone = 1;
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
	turnisDone = 0;
	for(long i = 0; i < 80000; i ++){stopp();}
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


//-----------------------------------KONVERTERING-------------------------
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

void straight()
{
	TransmitSensor(trstraight);
	sensor1r = sidesensor(storedValues[1]);
	sensor2r = sidesensor(storedValues[2]);


	while (fabs(sensor1r - sensor2r)>2)
	{
		if((sensor1r-sensor2r) > 0.8)
		{
			PORTC = 0x01; //rotera höger
			PORTD = 0x00;
			OCR2A = 60;
			OCR2B = 60;
		}
		else if((sensor2r-sensor1r) > 0.8)
		{
			PORTC = 0x00; //rotera vänster
			PORTD = 0x20;
			OCR2A = 60;
			OCR2B = 60;
		}

		TransmitSensor(trstraight);
		sensor1r = sidesensor(storedValues[1]);
		sensor2r = sidesensor(storedValues[2]);
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
	distance=0;
	dist = dist / 2.55125;
	//dist = dist / 2.55;
	
	while (distance < dist * 0.9)
	{
		transmit();
		driveF();
	}
	stopp();
	
}

void drivefromstill(float dist) //kör dist cm
{
	distance=0;
	dist = dist / 2.55125;
	//dist = dist / 2.55;

	while (distance < dist * 1.05)
	{
		transmit();
		driveF();
	}
	stopp();
}



void leftturn() //Används när man vet att det är vägg framför och vägg till höger för att kolla om man ska svänga vänster eller vända om helt
{
	stopp();
	transmit();

	sensorleft = sidesensor(storedValues[3]);

	if(sensorleft<25)
	{
		rotate90left();
		rotate90left();
	}
	else
	{
		rotate90left();
	}
	straight();
}





void regulateright()
{
	if(firstRR==0)
	{
		sensormeanr_old=sensormeanr;
	}
	transmit();
	//REGLERING
	//Omvandling till centimeter

	sensor1r = sidesensor(storedValues[1]);
	sensor2r = sidesensor(storedValues[2]);
	sensorfront = frontsensor(storedValues[0]);
	sensormeanr = ((sensor1r + sensor2r) / 2) + 4;

	if(firstRR==1)
	{
		firstRR=0;
		stopp();
		sensormeanr_old=sensormeanr;
	}
	else
	{
		//till PD-reglering
		Td = 90000000; //128000000
		K = 2;
		if(sensorfront < 50)
		{
			straight();
			drive(40);
			updatepos();
			transmit();
			sensorright = sidesensor(storedValues[1]);
			if(sensorright>25)
			{
				
				rotate90right();
				
				transmit();
				start_request = 1;
				sensorfront = frontsensor(storedValues[0]);
				if(sensorfront < 70)
				{
					drivefromstill(40);
					updatepos();
					leftturn();
				}
				else
				{
					drivefromstill(40);
					updatepos();
				}
			}
			else
			{
				leftturn();
			}
		}
		else if(((sensor1r-sensor2r) < 15) && ((sensor2r-sensor1r) < 15))
		{
			if (fabs(sensor1r-sensor2r) > 4)
			{
				straight();
			}
			else
			{
				startregulate=1;
				timer = TCNT1;
				dt = (timer + overflow * 65536) * 64;
				TCNT1 = 0;
				overflow = 0;
				PORTC = 0x01;
				PORTD = 0x20;
				rightpwm = speed + K * (15-sensormeanr + Td * (sensormeanr_old-sensormeanr)/dt);
				leftpwm = speed - K * (15-sensormeanr + Td * (sensormeanr_old-sensormeanr)/dt);

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
		}//Om inte på båda
		else
		{
			if(startregulate==1)
			{
				startregulate=0;
				drive(20);
				updatepos();
				rotate90right();
				
				
				start_request = 1;
				transmit();
				sensorfront = frontsensor(storedValues[0]);
				if(sensorfront>65)
				{
					drivefromstill(40);
					updatepos();

				}
				else
				{
					drivefromstill(40);
					updatepos();

					leftturn();
				}
				straight();
			}
			else
			{
				stopp();
			}
		}
	}

}


void firstlap()
{
	if(myposX == startpos[0] && myposY == startpos[1] && !start)	//Det här kommer gälla de första sekunderna roboten börjar köra också..!
	{
		onelap=1;
		setcursor(1);
		print_on_lcd(0xCC);
		straight();
		for(long i = 0; i < 160000; i ++){stopp();}
	}
	else
	{
		regulateright();
	}
}



void away() // Få roboten från väggen
{
	transmit();
	
	// Vänd roboten posetiv x-led
	while(mydirection != 1)
	{
		if(mydirection < 3)
		rotate90right();
		else
		rotate90left();
	}

	transmit();
	sensorleft=sidesensor(storedValues[3]);
	setcursor(1);
	print_on_lcd(sensorleft);

	if(sensorleft < 20) // Vägg till vänster
	{
		regulateright();
		sensorleft=sidesensor(storedValues[3]);
		if(sensorleft > 20)
		{
			getinpos = true;
		}

	}
	else if(getinpos)
	{
		drive(20);
		updatepos();
		getinpos = false;
	}
	else
	{
		asm("");
		straight();
		stopp();
		rotate90left();
		print_on_lcd(sensorleft);
		drivefromstill(40); // Kör en sektion ut i öppen yta
		updatepos();

		rotate90right();

		awaydone = true;
	}
}

void zigzag() //sicksacksak
{
	transmit();
	sensorfront = frontsensor(storedValues[0]);
	sensorright = sidesensor(storedValues[1]);
	sensorleft = sidesensor(storedValues[3]);

	if(sensorfront>50) // Kör tills roboten står en ruta från väggen
	{
		driveF();
	}
	else if(zzfirst)	// Första gången vi når vägg
	{
		zzfirst = false;
		rotate90left();
		
		for(long i = 0; i < 160000; i ++){	stopp();}
	}
	else if(zzleftturn)
	{
		
		zzleftturn = false;
		if(sensorleft < 20) // Har vi vägg till vänster när vi vill svänga vänster
		{
			zigzagdone = true;
		}
		else
		{
			rotate90left();
			drivefromstill(40);
			updatepos();
			
			transmit();
			sensorfront = frontsensor(storedValues[0]);
			sensorright = sidesensor(storedValues[1]);
			sensorleft = sidesensor(storedValues[3]);
			
			while(sensorleft < 20) // Har vi vägg vänster efter första sväng
			{
				transmit();
				sensorfront = frontsensor(storedValues[0]);
				sensorright = sidesensor(storedValues[1]);
				sensorleft = sidesensor(storedValues[3]);
				drive(40);
				updatepos();
			}
			rotate90left();
		}
	}
	else
	{
		transmit();
		sensorfront = frontsensor(storedValues[0]);
		sensorright = sidesensor(storedValues[1]);
		sensorleft = sidesensor(storedValues[3]);
		zzleftturn = true;
		if(sensorright < 20) // Har vi vägg till höger när vi vill svänga höger
		{
			zigzagdone = true;
		}
		else
		{
			rotate90right();
			drivefromstill(40);
			updatepos();
			while(sensorright < 20) // Har vi vägg höger efter första sväng
			{
				transmit();
				sensorfront = frontsensor(storedValues[0]);
				sensorright = sidesensor(storedValues[1]);
				sensorleft = sidesensor(storedValues[3]);
				drive(40);
				updatepos();
			}
			rotate90right();
		}
	}
}



/*************************RFID****************************/
void rfid()
{
	storedValues[7] = 0;
	volatile int bajs = 1;
	while(bajs==1)
	{

		transmit();
		if(storedValues[7] != 1)
		{
			PORTC = 0x01;
			PORTD = 0x20;
			OCR2B = 50;
			OCR2A = 50;
		}
		else
		{
			stopp();
			bajs=0;

		}


	}
}



int main(void)
{
	initiate_variables();
	initiation();
	int fjarrstyrt = (PIND & 0x01); //1 då roboten är i fjärrstyrt läge
	initiate_timer();
	initiate_request_timer();

	if(fjarrstyrt==1)
	{
		
		for(long i = 0; i < 480000; i++){}
		/*while(1)
		{
			transmit();
			rotate90right();
			for(long i = 0; i < 80000; i++){}
			rotate90left();
			for(long i = 0; i < 80000; i++){}
			print_on_lcd(storedValues[0]);
		}*/
		remotecontrol();
	}
	else
	{
		MasterInit();
		for(long i = 0; i < 480000; i++){}
		
		while(home==0)
		{
			
			if(posdistance > 13)  //40/2.55125)*0.9
			{
				updatepos();
			}
			
			setcursor(1);
			print_on_lcd(myposX);
			print_on_lcd(myposY);
			
			if(!onelap)
			{
				firstlap();
			}
			else if(!awaydone)
			{
				away();
				print_on_lcd(0xAA);
			}
			else if(!zigzagdone)
			{
				zigzag();				
				print_on_lcd(0xBB);
			}
			/*else if(!findemptydone)
			{
			findempty();
			}
			else
			{
			returntostart();
			}*/
			else
			{
				stopp();
				home=1;
				print_on_lcd(0xAB);
			}
		}
		/*
		while(1)
		{
		rotate90left();
		for(long i = 0; i < 160000; i ++){stopp();}
		rotate90right();
		for(long i = 0; i < 160000; i ++){stopp();}
		}
		*/
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
