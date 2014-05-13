/*
* Search.c
*
* Created: 4/29/2014 9:26 AM
*  Author: robsv107
*          patsu326
*	   marek588
*/

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include "styrRobot.h"

#define F_CPU 1000000UL

/***********************************************LCDSKÄRM*********************************/

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

void Initiation()
{
	//Sätter utgångar/ingångar    (Kanske skriva en initieringsfunktion för allt detta? /Robert)
	DDRA=0b11111111;
	DDRC=0b11000001;
	DDRD=0b11100000;

	//TCCR1A=0b10010001; 
	//TCCR1B=0b00000010; 
	TCCR2A=0b10010001; //setup, phase correct PWM
	TCCR2B=0b00000010; //sätter hastigheten på klockan

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
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPI2X)|(1<<SPR1)|(1<<SPR0)|(1<<CPHA)|(1<<CPOL);

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


void transmit()
{
	TransmitSensor(0);
	TransmitComm();
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
			bussdelay();
			
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
			bussdelay();
			
			MasterTransmit(traveldist); // Request front sensor
			bussdelay();
			storedValues[7] = SPDR; // SensorRFID
			
			MasterTransmit(front); // Request front sensor
			bussdelay();
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

		distance += storedValues[5];
		posdistance += storedValues[5];

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

/******************************FJÄRRSTYRNING**********************/
void remotecontrol()
{
	while(1)
	{
		char button = PINB & 0b00000111; //Ta emot styrdata

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
			posdistance=0;
		}
		case (2): // Y+
		{
			myposY+=1;
			//mypos[1]=myposY;
			posdistance=0;
		}
		case (3): // X-
		{
			myposX-=1;
			//mypos[0]=myposX;
			posdistance=0;
		}
		case (4): // Y-
		{
			myposY-=1;
			//mypos[1]=myposY;
			posdistance=0;
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

	switch(mydirection)
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
		if (!room[myposX][myposY+1]==(1|4))
		{
			room[myposX][myposY+1]=2;
		}
		break;
	}
	if (storedValues[7]==1)
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

/********************************STYRNING*****************************************/


void stopp()
{
	OCR2A = 0;
	OCR2B = 0;
}

void driveF()
{
	PORTC = 0x01;
	PORTD = 0x20;
	OCR2A = speed;
	OCR2B = speed;
}

// Hjälpfunktion för att köra en viss distans
void driveDist(float dist)
{
	
	dist = dist / 2.55125;
	//dist = dist / 2.55;

	while (distance < dist * 1)
	{
		transmit(0);
		driveF();
	}
	stopp();
}


void drivefromstill(float dist) //kör dist cm
{
	distance=0;
	dist = dist / 2.55125;
	//dist = dist / 2.55;

	while (distance < dist * 1.6)
	{
		transmit(0);
		driveF();
	}
	stopp();
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
			TransmitSensor(turnstop);
			isDone = 0;
		}

	}
}

void rotate90right()
{
	volatile int isDone = 0;
	while(isDone == 0)
	{
		TransmitSensor(turn);
		if (storedValues[6] != 1)
		{
			rotateright();
		}
		else
		{
			TransmitSensor(turnstop);
			isDone = 0;
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
/*********************************FÖRSTA VARV*************************************/

void firstlap()
{
	if(myposX == startpos[0] && myposY == startpos[1] && !(start))
	{
		onelap=1;
	}
	else
	{
		regulateright();
	}
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
			driveDist(40);
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
				driveDist(20);
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
		regulateright();
		getinpos = true;

	}
	else if(getinpos)
	{
		driveDist(20);
	}
	else
	{
		temporary90left();

		driveDist(40); // Kör en sektion ut i öppen yta

		temporary90right();

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
		temporary90left();
	}
	else if(zzleftturn)
	{
		TransmitSensor(left);
		zzleftturn = false;
		if(sensorleft < 20)
		{
			zigzagdone = true;
			return;
		}
		temporary90left();
		driveDist(40);
		while(sensorleft < 20)
		{
			driveDist(40);
		}
		temporary90left();
	}
	else
	{
		TransmitSensor(right);
		zzleftturn = true;
		if(sensorright < 20)
		{
			zigzagdone = true;
			return;
		}
		temporary90right();
		driveDist(40);
		while(sensorright < 20)
		{
			driveDist(40);
		}
		temporary90right();
	}
}

/*********************************MISSADE RUTOR*******************************/

/************************************HITTA FÖRSTA NOLLAN I RUMMET**********************************/
int * findfirstzero()
{

	static int firstzero[2]={15,0};

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
	transmit();
	if(myposX == pos[0]  && myposY  == pos[1])
	room[myposX][myposY] = 2;
	else if(myposX >= pos[0] && myposY <= pos[1]) //Fjärde kvadranten
	{
		switch(mydirection)
		{
			case(1):
			temporary90left();
			case(2):
			if(sensorfront > 50)
			driveF();
			else if((pos[1]-myposY) == 1)
			{
				driveDist(40);
				temporary90left();
			}
			case(3):
			if(sensorfront > 50)
			driveF();
			else if((myposX-pos[0]) == 1)
			{
				driveDist(40);
				temporary90right();
			}
			case(4):
			temporary90right();
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
			temporary90right();
			case(4):
			temporary90left();
		}
	}
	else if(myposX < pos[0] && myposY > pos[1])
	{
		switch(mydirection)
		{
			case(1):
			if(sensorfront > 50)
			driveF();
			case(2):
			temporary90right();
			case(3):
			temporary90left();
			case(4):
			if(sensorfront > 50)
			driveF();
		}
	}
	else
	{
		switch(mydirection)
		{
			case(1):
			temporary90right();
			case(2):
			temporary90left();
			case(3):
			if(sensorfront < 50)
			driveF();
			case(4):
			if(sensorfront < 50)
			driveF();
		}
	}
}

void findempty()
{
	int *notsearched = findfirstzero();

	if(notsearched[0] == 15 && notsearched[1] == 0)
	findemptydone = true;
	else
	driveto(notsearched);
}







void returntostart()
{
	//int mydirection; //Robotens riktning
	//int myposX; //Rpbotens position i X-led
	//int starposX; //Starpositionens värde i X-led

	if(mydirection == 4) //4=negativ y-led. x+,y+,x-,y- = 1,2,3,4
	{
		while(sensorfront>10)
		{
			//Kör rakt fram
			PORTC = 0x01;
			PORTD = 0x40;
			OCR2A = 180;
			OCR1A = 180;
		}
		if(myposX<startpos[0]) //Om ingången är till höger om roboten
		{
			while(myposX<startpos[0])
			{
				regulateright();
			}
		}
		else
		{
			while(myposX>startpos[0]) //Om ingången är till vänster om roboten
			{
				regulateright();
			}
		}
	}
	else
	{
		temporary90left();  //Rotera 90 grader om vi står i fel riktning
	}
}


int main(void)
{
	MasterInit();
	Initiation();
	_delay_ms(40000);
	initiate_request_timer();

	transmit();

	int fjarrstyrt = (PIND & 0x01); //1 då roboten är i fjärrstyrt läge

	if(fjarrstyrt==1)
	{
		remotecontrol();
	}
	else
	{
		

		while(home==0)
		{

			if(distance >= 40/0.8125) //dividera med sektor 0.8125
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




ISR(TIMER0_COMPB_vect)
{
	TCCR0B = 0b0000000; //stop timer
	TCNT0 = 0x00;
	start_request = 1;
	//writechar(0b01010111); //W
}
