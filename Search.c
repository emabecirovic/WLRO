/*
* Search.c
*
* Created: 4/29/2014 9:26 AM
*  Author: robsv107
*          	patsu326
*				marek588
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

		if(invalue == turn) // Starta flöde av gyrovärden
		{
			MasterTransmit(gyro);
			bussdelay();
			
			MasterTransmit(stop);
			storedValues[6] = SPDR; // Gyro
			
		}
		else if(invalue == turnstop) // Stoppa flöde av gyrovärden
		{
			MasterTransmit(gyrostop);
		}
		else  // Annars ta emot ett paket med övriga sensorvärden
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
			bussdelay();
			storedValues[0] = SPDR; // Front
			
			MasterTransmit(rightback);
			bussdelay();
			storedValues[1] = SPDR; // Right front
			
			MasterTransmit(leftfront);
			bussdelay();
			storedValues[2] = SPDR; // Right back
			
			MasterTransmit(leftback);
			bussdelay();
			storedValues[3] = SPDR; // Left front
			
			MasterTransmit(stop);
			bussdelay();
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
		if(start_request == 1)
	{
		start_request = 0;
	PORTB &= 0b11110111; // ss1 low

	bussdelay();
	for(int i = 0; i < 11; i ++)
	{
		dummy = SPDR; // Dummy läsning för att cleara SPIF
		MasterTransmit(storedValues[i]);
		bussdelay();
	}

	PORTB ^= 0b00001000; // ss1 low
	
		TCCR0B = 0b00000101; // Start timer
	}
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
			posdistance=0;
		}
		case (2): // Y+
		{
			myposY+=1;
			posdistance=0;
		}
		case (3): // X-
		{
			myposX-=1;
			posdistance=0;
		}
		case (4): // Y-
		{
			myposY-=1;
			posdistance=0;
		}
	}
}


/********************************STYRNING*****************************************/


void stopp() // Stanna roboten
{
	OCR2A = 0;
	OCR2B = 0;
}

void driveF() // Kör framåt
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


void drivefromstill(float dist) //kör dist cm från stillastående
{
	distance=0;
	dist = dist / 2.55125;
	
	while (distance < dist * 1.6)
	{
		transmit(0);
		driveF();
	}
	stopp();
}

void straight()
{
	if((sensor1r-sensor2r) > 0.5) // diff mindre än 0.5
	{
		PORTC = 0x01; //rotera höger
		PORTD = 0x00;
		OCR2A = 70;
		OCR2B = 70;
	}
	else if((sensor2r-sensor1r) > 0.5) // diff mindre än 0.5 andra hållet
	{
		PORTC = 0x00; //rotera vänster
		PORTD = 0x20;
		OCR2A = 70;
		OCR2B = 70;
	}
}

void temporary90right() // Hårdkodad 90 graders högersväng
{
	cli();
	PORTC = 0x01;
	PORTD = 0x00;
	OCR2B = 110;
	OCR2A = 110;
	_delay_ms(7000);
	sei();
}

void temporary90left() // Hårdkodad 90 graders vänstersväng
{
	cli();
	PORTC = 0x00;
	PORTD = 0x20;
	OCR2B = 110;
	OCR2A = 110;
	_delay_ms(7000);
	sei();
}


void rotate90left() // Rotera 90 grader med gyro
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

void rotate90right() // Rotera 90 grader med gyro
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

/**********************************KONVERTERING**************************/
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

	// sensorright = sidesensor(storedValues[1]);
	sensorleft = sidesensor(storedValues[3]);

	if(sensorleft<20) // Vägg närmare än 20 cm till vänster
	{
		temporary90left();
		temporary90left();
	}
	else
	{
		temporary90left();
	}
}

void rotateleft() // Rotera vänster
{
	PORTC = 0x00;
	PORTD = 0x20;
	OCR2B = 170;
	OCR2A = 170;
}

void rotateright() // Rotera höger
{
	PORTC = 0x01;
	PORTD = 0x00;
	OCR2B = 170;
	OCR2A = 170;
}
/*********************************FÖRSTA VARV*************************************/

void firstlap()
{
	if(myposX == startpos[0] && myposY == startpos[1] && !(start)) // pos[0] = X-koordinat & pos[1] = Y-koordinat
	{
		onelap=1;
	}
	else
	{
		regulateright();
	}
}



void regulateright() // Reglering mot högervägg
{

	if(firstRR==0) // Gör endast första gången funktionen körs
	{
		sensormeanr_old=sensormeanr;
	}
	
	transmit(); // Hämta och skicka värden

	//REGLERING
	sensor1r = sidesensor(storedValues[1]);
	sensor2r = sidesensor(storedValues[2]);
	sensorfront = frontsensor(storedValues[0]);
	
	sensormeanr = ((sensor1r + sensor2r) / 2) + 9; // Tar ut ett medelvärde

	if(firstRR==1) // Alla gånger förutom första gången
	{
		firstRR=0;
		stopp();
		sensormeanr_old=sensormeanr;
	}
	else
	{
		//till PD-reglering
		Td = 400000;
		K = 4;

		if(sensorfront<50) // Vägg närmare än 50 cm
		{
			driveDist(40); // Kör 40 cm
			transmit();
			sensorright = sidesensor(storedValues[1]);

			if(sensorright>20) // Vägg längre bort än 20 cm
			{
				temporary90right();
				transmit();
				sensorfront = frontsensor(storedValues[0]);

				if(sensorfront<60) // Vägg en ruta bort?
				{
					drivefromstill(40);
					leftturn(); // Återvändsgrändsrutin
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
		else if(((sensor1r-sensor2r) < 20) && ((sensor2r-sensor1r) < 20))  // Har vägg att reglera mot
		{
			if (fabs(sensor1r-sensor2r) > 3) // För sne för reglering
			{
				straight(); // Rakställ roboten
			}
			else
			{
				
				driven=1;
				
				// Tidsdifferans för D-reglering
				timer = TCNT1;
				dt = (time + overflow * 65536) * 64;


				TCNT1 = 0;
				overflow = 0;

				PORTC = 0x01;
				PORTD = 0x20;
				rightpwm = speed + K * (18-sensormeanr + Td * (sensormeanr_old-sensormeanr)/dt);
				leftpwm = speed - K * (18-sensormeanr + Td * (sensormeanr_old-sensormeanr)/dt);


				if (rightpwm > 255) // Se till att PWM inte blir för stor
				{
					OCR2B = 255;
				}
				else if(rightpwm < 0) // Se till att PWM inte blir för liten
				{
					OCR2B = 0;
				}
				else
				{
					OCR2B = rightpwm;
				}
				if (leftpwm > 255) // Dito
				{
					OCR2A = 255;
				}
				else if (leftpwm < 0) // Dito
				{
					OCR2A = 0;
				}
				else
				{
					OCR2A = leftpwm;
				}

			}
		}
		else // Tappat vägg på högersida
		{
			if(driven==1)
			{
				driven=0;
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

	if(leftfront < 20) // Vägg till vänster
	{
		regulateright();
		getinpos = true;

	}
	else if(getinpos)
	{
		driveDist(20);
		getinpos = false;
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
	if(sensorfront>50) // Kör tills roboten står en ruta från väggen
	{
		driveF();
	}
	else if(first)	// Första gången vi når vägg
	{
		first = false;
		temporary90left();
	}
	else if(zzleftturn) 
	{
		TransmitSensor(0);
		zzleftturn = false;
		if(sensorleft < 20) // Har vi vägg till vänster när vi vill svänga vänster
		{
			zigzagdone = true;
			return; // Klar med zigzag
		}
		temporary90left();
		driveDist(40);
		while(sensorleft < 20) // Har vi vägg höger efter första sväng
		{
			TransmitSensor(0);
			driveDist(40);
		}
		temporary90left();
	}
	else
	{
		TransmitSensor(0);
		zzleftturn = true;
		if(sensorright < 20)   // Har vi vägg till höger när vi vill svänga höger
		{
			zigzagdone = true;
			return; // Klar med zigzag
		}
		temporary90right();
		driveDist(40);
		while(sensorright < 20)  // Har vi vägg höger efter första sväng
		{
			TransmitSensor(0);
			driveDist(40);
		}
		temporary90right();
	}
}

/*********************************MISSADE RUTOR*******************************/

/************************************HITTA FÖRSTA NOLLAN I RUMMET**********************************/
void findempty()
{
	int *notsearched = findfirstzero();

	if(notsearched[0] == 15 && notsearched[1] == 0)
	findemptydone = true;
	else
	driveto(notsearched);
}


void driveto(int pos[2])
{
	transmit();
	if(myposX == pos[0]  && myposY  == pos[1]) // Jämför koordinaterna roboten står på med positionen vi vill åka till
	{
		
	}	
	else if(myposX >= pos[0] && myposY <= pos[1]) //Fjärde kvadranten, tänk önskad position som origo
	{
		switch(mydirection)
		{
			case(1): // X+
			temporary90left();
			case(2): // Y+
			if(sensorfront > 50) // Ingen vägg framför
				driveF(); 
			else if((pos[1]-myposY) == 1) // Om vi står endast en ruta ifrån önskad position
			{
				driveDist(40);
				temporary90left();
			}
			case(3): // X-
			if(sensorfront > 50) // Ingen vägg framför
				driveF();
			else if((myposX-pos[0]) == 1) // Om vi står endast en ruta ifrån önskad position
			{
				driveDist(40);
				temporary90right();
			}
			case(4): // Y-
			temporary90right();
		}
	}
	else if(myposX <= pos[0] && myposY <= pos[1]) // Tredje kvadranten KOLLA HÄR OM DET FUNKAR
	{																					// Sammma princip som ovan se kommentarer där
		switch(mydirection)
		{
			case(1): // X+
			if(sensorfront > 50)
			driveF();
			else if((pos[0]-myposX) == 1)
			{
				driveDist(40);
				temporary90left();
			}
			case(2): //Y+
			if(sensorfront > 50)
			driveF();
			else if((pos[1]-myposY) == 1)
			{
				driveDist(40);
				temporary90right();
			}
			case(3): // X-
			temporary90right();
			case(4): // Y-
			temporary90left();
		}
	}
	else if(myposX < pos[0] && myposY > pos[1]) // Andra kvadranten, dito
	{
		switch(mydirection)
		{
			case(1): // X+
			if(sensorfront > 50)
			driveF();
			else if((pos[0] - myposX == 1))
			{
				driveDist(40);
				temporary90right();
			}
			else
			{
				driveDist(40);
				rotate
			}
			case(2): // Y+
			temporary90right();
			case(3): // X-
			temporary90left();
			case(4): // Y-
			if(sensorfront > 50)
			driveF();
			else if((myposY-pos[1]) == 1)
			{
				driveDist(40);
				temporary90left();
			}
		}
	}
	else // Första kvadranten, dito
	{
		switch(mydirection)
		{
			case(1): // X+
			temporary90right();
			case(2): // Y+
			temporary90left();
			case(3): // X-
			if(sensorfront > 50) 
				driveF();
			else if((myposX-pos[0]) == 1) 
			{
				driveDist(40);
				temporary90right();
			}
			case(4): // Y-
			if(sensorfront > 50)
			driveF();
			else if((myposY-pos[1]) == 1)
			driveDist(40);
		}
	}
}


void returntostart()
{
	if(myposX == startpos[0] && myposY == startpos[1] && !(start)) // pos[0] = X-koordinat & pos[1] = Y-koordinat
	{
		home=1;
	}
	else
	{
		driveto(startpos);
	}
	
	/*
	//int mydirection; //Robotens riktning
	//int myposX; //Rpbotens position i X-led
	//int starposX; //Starpositionens värde i X-led

	if(mydirection == 4) //4=negativ y-led. x+,y+,x-,y- = 1,2,3,4
	{
		while(sensorfront>50)
		{
			driveF();
			//Kör rakt fram
			/*PORTC = 0x01;
			PORTD = 0x40;
			OCR2A = 180;
			OCR1A = 180;*
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
				regulateright(); // REGULATE LEFT ?????
			}
		}
	}
	else
	{
		temporary90left();  //Rotera 90 grader om vi står i fel riktning
	}*/
}


int main(void)
{
	
	Initiation();
	_delay_ms(40000);
	initiate_request_timer();

	int fjarrstyrt = (PIND & 0x01); //1 då roboten är i fjärrstyrt läge

	if(fjarrstyrt==1)
	{
		remotecontrol();
	}
	else
	{
		
		MasterInit();
		_delay_ms(40000);
		transmit();
		
		while(home==0)
		{

			if(distance >= 40/0.8125) // FEL VÄRDE KOLLA MED EMA
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
