/*
* FILNAMN: styrmodul.c
* PROGRAMMERARE: Grupp 9 - WLRO
* DATUM: 2014-06-04
*
* .c-fil till styrmodulens ATmega1284P
*
*
*/

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include "StyrRobot.h"

#define F_CPU 1000000UL

/**************** INITIERING******************/
void initiate_variables()
{
	/************BUSS**********************/

	// Start för busskom
	start_request = 0;

	// Array för värden från buss
	storedValues[8] = 1;

	storedValues[11] = 15;
	storedValues[12] = 1;


	/******************REGLERING************************/

	distance = 0; // Avlagdsträcka

	speed = 60;

	turnisDone = 0;


	// Flaggor för regulateright
	firstRR=1;
	startregulate = 0;

	/**************POSITION******************/
	mydirection = 1; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
	myposX = 15; //Robotens position i X-led
	myposY = 1; //Robotens position i Y-led
	
	RFIDX = 42;
	RFIDY = 42;
	
	posdistance = 0;
	foundRFID = 0;
	
	
	/****************FJÄRRSTYRT**************************/
	button = 0x00;
	
	
	/***************FLAGGOR FÖR MAIN******************/
	start = 1; //vi står i startpositionen

	finished = 0; //1 då hela kartan utforskad
	onelap = 0; //1 då yttervarvet körts
	home = 0; //1 då roboten återvänt till startposition
	
	//secondlap
	searched = false;
	controlisland = false;
	
	// Position utanför kartan innan vi har varit vid en ö
	storeposX = 42;
	storeposY = 42;
	storedirection = 0;
}

void initiate_request_timer()
{
	TIMSK0 = 0b00000100; //Enable interupt vid matchning med OCR0B
	TCNT0 = 0x00;
	TCCR0B = 0b0000101; //Starta räknare, presscale 1024
	OCR0B = 0xFF; // 255
	// 261120 st klockcykler
}

void initiation()
{
	//Sätter utgångar/ingångar
	DDRA = 0b11111111;
	DDRC = 0b11000001;
	DDRD = 0b11100000;
	//TCCR1A=0b10000001; //setup, phase correct PWM
	//TCCR1B=0b00000010; //sätter hastigheten på klockan
	TCCR2A = 0b10100001;
	TCCR2B = 0b00000010;

	//Till displayen, vet inte om det behövs men den är efterbliven
	PORTA = 0b00110000;
	PORTC = 0b00000000;
	_delay_ms(200);
	PORTA = 0b00110000;
	PORTC = 0b10000000;
	PORTC = 0b00000000;
	_delay_ms(50);
	PORTA = 0b00110000;
	PORTC = 0b10000000;
	PORTC = 0b00000000;
	_delay_us(1100);
	//Startar initiering
	PORTA = 0b00110000; // 1-line mode ; 5x8 Dots
	PORTC = 0b10000000;
	PORTC = 0b00000000;
	_delay_us(4000);
	PORTA = 0b00001111; // Display on ; Cursor on ; Blink on
	PORTC = 0b10000000;
	PORTC = 0b00000000;
	_delay_us(4000);
	PORTA = 0b00000001; // Clear display
	PORTC = 0b10000000;
	PORTC = 0b00000000;
	_delay_ms(200);
	PORTA = 0b00000111; //Increment mode ; Entire shift on
	PORTC = 0b10000000;
	_delay_ms(200);
	//Initiering klar
	storedValues[6] = 0;
}

/*************************LCDSKÄRM******************/

void writechar(unsigned char data)
{
	PORTA = data;
	(PORTC |= 0b11000000);
	(PORTC &= 0b01000001);
	_delay_ms(30);
}

void shift(int steps) //
{
	int n = 0;
	while(n < steps)
	{
		PORTA = 0b00011100; //Shift display right
		(PORTC |= 0b11000000);
		(PORTC &= 0b01000001);
		_delay_ms(30);
		n += 1;
	}
	int m = 0;
	while(m < (2 * steps))
	{
		PORTA = 0b00010000; //Shift cursor left
		(PORTC |= 0b11000000);
		(PORTC &= 0b01000001);
		_delay_ms(30);
		m += 1;
	}
}

void shiftcursorleft()
{
	PORTA = 0b00010000; //Shift cursor left
	(PORTC |= 0b11000000);
	(PORTC &= 0b01000001);
	_delay_ms(30);
}

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
	writechar(what_lcd_number(lownumber));
	writechar(lcdspace);
}

void setcursor(char place) //16 platser på en rad. 0x00-0x0F
{
	PORTA = (0x80 + place - 0x01);
	(PORTC |= 0b10000000);
	(PORTC &= 0b00000001);
	for(long i = 0; i < 2000; i ++){}
}

/***************************************BUSSFUNKTIONER*************************/

void MasterInit(void)
{
	/* PORTB används som buss */
	/* Sätter MOSI, SSx och SCK utgångar, alla andra ingångar*/
	/* PORTB4 = SS1 och PORTB5 = SS2
	DDRB = (1<<DDB3)|(1<<DDB4)|(1<<DDB5)|(1<<DDB7);*/

	/* Enable SPI, Master, clock rate fosc/64, mode 3 SPI */
	SPCR = (1<<SPE)|(1<<MSTR)|(0<<SPI2X)|(1<<SPR1)|(0<<SPR0)|(1<<CPHA)|(1<<CPOL);

	// Ettställer SSx
	PORTB = (1<<PORTB3)|(1<<PORTB4);

	/* Enable External Interrupts */
	sei();
}

void MasterTransmit(char cData)
{
	/* Start överföring */
	SPDR = cData;
	/* Vänta på att överföringen ska bli klar */
	while(!(SPSR & (1<<SPIF)))
	;
	
	// Ge tid till bearbetning av överföring
	bussdelay();
}

void bussdelay()
{
	for(int i = 0; i < time; i++){}
}


void transmit()
{
	TransmitComm(0);
	TransmitSensor(0);
}

void TransmitSensor(char invalue)
{
	// Vänta på timer som ser till att sensor får tid till AD-omvandling
	while(start_request == 0)
	{
	}
	
	if(start_request == 1)
	{
		start_request = 0;
		
		PORTB &= 0b11101111; // ss2 low

		if(invalue == turn)
		{
			MasterTransmit(gyro);
			
			MasterTransmit(stop);
			storedValues[6] = SPDR; // Gyrovärde
		}
		else if(invalue == turnstop)
		{
			MasterTransmit(gyrostop);
			
			dummy = SPDR;
		}
		else if(invalue == RFID)
		{
			MasterTransmit(RFID);
			
			
			dummy = SPDR; // inget
		}
		else if(invalue == RFIDstop)
		{
			MasterTransmit(RFIDstop);
			
			MasterTransmit(stop);
			storedValues[7] = SPDR; //RFID hittat eller inte
		}
		else
		{
			MasterTransmit(traveldist); // Request front sensor
			dummy = SPDR; //Första värdet är vi inte intresserade av
			
			MasterTransmit(front); // Request front sensor
			
			storedValues[5] = SPDR; // Antal sektorer på svart/vit skiva
			MasterTransmit(rightfront);
			
			storedValues[0] = SPDR; // vänster lång sensor
			MasterTransmit(rightback);
			
			storedValues[1] = SPDR; // höger fram sensor
			MasterTransmit(leftfront);
			
			storedValues[2] = SPDR; // höger bak sensor
			MasterTransmit(leftback);
			
			storedValues[3] = SPDR; // Fram sensor
			MasterTransmit(stop);
			
			storedValues[4] = SPDR; // vänster kort sensor
		}

		PORTB ^= 0b00010000; // ss2 high

		if(invalue != trstraight) // Uppdatera ingen avlagd sträcka när vi kör straight
		{
			distance = distance + storedValues[5];
			posdistance = posdistance + storedValues[5];
			storedValues[5] = 0;
		}

		TCCR0B = 0b00000101; // Start timer
	}
}

void TransmitComm(char invalue)
{
	PORTB &= 0b11110111;

	if(invalue == update)
	{
		dummy = SPDR;
		MasterTransmit(arraytransmit);
		

		for(int i = 0; i < 11; i ++)
		{
			dummy = SPDR;
			MasterTransmit(storedValues[i]);
			
		}

		dummy = SPDR;
		MasterTransmit(myposX);
		

		dummy = SPDR;
		MasterTransmit(myposY);
		
		
		dummy = SPDR;
		MasterTransmit(RFIDX);
		
		
		dummy = SPDR;
		MasterTransmit(RFIDY);
		

		dummy = SPDR;
		MasterTransmit(updateroom);
		
	}
	else if(invalue == alrdyDone)
	{
		dummy = SPDR;
		MasterTransmit(alrdyDone);
		
		
		MasterTransmit(stop);
		
		searched = SPDR;
	}
	else if(invalue == firstdone)
	{
		dummy = SPDR;
		MasterTransmit(firstdone);
		
	}
	else
	{
		dummy = SPDR;
		MasterTransmit(arraytransmit);
		
		// Skicka hela arrayen med sensorvärden
		for(int i = 0; i < 11; i ++)
		{
			dummy = SPDR;
			MasterTransmit(storedValues[i]);
			
		}

		dummy = SPDR;
		MasterTransmit(myposX);
		

		dummy = SPDR;
		MasterTransmit(myposY);
		
		
		dummy = SPDR;
		MasterTransmit(RFIDX);
		

		dummy = SPDR;
		MasterTransmit(RFIDY);
		
		
	}

	PORTB ^= 0b00001000;
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
void traveledDist()
{
	char i = 0;
	if(n == 0)
	{
		i = 0;
	}
	else
	{
		i = 1;
	}

	if(posdistance > 31 + i)  //40/2.55125)*0.9
	{
		updatepos();
		if(n != 4)
		{
			n = n + 1;
		}
		else
		{
			n = 0;
		}
	}
}


void updatepos()
{
	if (foundRFID == 0)
	{
		stopp();
		start_request = 1;
		TransmitSensor(RFID);
	}
	start = 0;
	asm("");
	switch(mydirection)
	{
		case (1): // X+
		{
			myposX += 1;
			break;
		}
		case (2): // Y+
		{
			myposY += 1;
			break;
		}
		case (3): // X-
		{
			myposX -= 1;
			break;
		}
		case (4): // Y-
		{
			myposY -= 1;
			break;
		}
		default:
		{
		}
	}
	
	// Nollställ avlagdstäcka till nästa position i rutnätet
	posdistance = 0;
	
	
	if (foundRFID == 0)
	{
		for(long i = 0; i < 160000; i++){}
		for(long i = 0; i < 160000; i++){}
		start_request = 1;
		TransmitSensor(RFIDstop);
		if (storedValues[7] == 1)
		{
			//foundRFID = 1;
			RFIDX = myposX;
			RFIDY = myposY;
			
		}
		
	}
	else
	{
		storedValues[7] = 0;
	}
	
	// Tillåt att kolla vänster efter ö
	controlisland = true;
	//Tvinga en överföring på buss direkt
	start_request = 1;
	TransmitComm(update);

}


//-------------------------------------------------------------------------------------
/**************************************STYRNING*************************************/
void stopp()
{
	//Hastighet höger, vänster
	OCR2A = 0;
	OCR2B = 0;
}

void rotateleft()
{
	// Riktning höger, vänster
	PORTC = 0x00;
	PORTD = 0x20;
	//Hastighet höger, vänster
	OCR2B = 100;
	OCR2A = 100;
}

void rotateright()
{
	// Riktning höger, vänster
	PORTC = 0x01;
	PORTD = 0x00;
	// Hastighet höger, vänster
	OCR2B = 100;
	OCR2A = 100;
}

void rotate90left()
{
	start_request = 1;
	TransmitComm(update);
	
	//stå still och kontrollera väggar runt roboten
	for(long i = 0; i < 80000; i ++)
	{
		stopp();
	}
	
	
	storedValues[6] = 0;
	while(turnisDone == 0)
	{
		start_request = 1;
		TransmitSensor(turn); // Start gyro hos sensormodul
		if (storedValues[6] != 1)
		{
			rotateleft();
		}
		else
		{
			start_request = 1;
			TransmitSensor(turnstop); // kolla med sensormodul om vi roterat 90
			stopp();
			turnisDone = 1;
		}

	}
	
	// uppdatera riktning på roboten
	if(mydirection == 4)
	{
		mydirection = 1;
	}
	else
	{
		mydirection += 1;
	}
	turnisDone = 0;
	storedValues[8] = mydirection;
	
	//stå still och kontrollera väggar runt roboten
	for(long i = 0; i < 80000; i ++)
	{
		stopp();
	}

	start_request = 1;
	TransmitSensor(0);
	TransmitComm(update);
	posdistance = 0;
	n = 0;
	
	// Tållåt kontrol efter ö
	controlisland = true;
}

void rotate90right()
{
	start_request = 1;
	TransmitComm(update);
	for(long i = 0; i < 80000; i ++)
	{
		stopp();
	}

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
	if(mydirection == 1)
	{
		mydirection = 4;
	}
	else
	{
		mydirection -= 1;
	}
	turnisDone = 0;
	storedValues[8] = mydirection;

	for(long i = 0; i < 80000; i ++)
	{
		stopp();
	}

	start_request = 1;
	TransmitSensor(0);
	TransmitComm(update);
	posdistance = 0;
	n = 0;
}


//-----------------------------------KONVERTERING-------------------------
float shortsensor(unsigned char sensorvalue)
{
	float value = sensorvalue;
	value = 1 / value;
	value = value - 0.000741938763948;
	value = value / 0.001637008132828;

	return value;
}

float longsensor(unsigned char sensorvalue)
{
	float value = sensorvalue;
	value = 1 / value;
	value = value - 0.001086689563586;
	value = value / 0.000191822821525;

	return value;
}

/*****************************REGLERING & AVSÖKNING***************************/

void straight()
{
	start_request = 1;
	TransmitSensor(trstraight);
	sensor1r = shortsensor(storedValues[1]);
	sensor2r = shortsensor(storedValues[2]);


	while (fabs(sensor1r - sensor2r) > 0.5)
	{
		if((sensor1r - sensor2r) > 0.5)
		{
			PORTC = 0x01; //rotera höger
			PORTD = 0x00;
			OCR2A = 60;
			OCR2B = 60;
		}
		else if((sensor2r - sensor1r) > 0.2)
		{
			PORTC = 0x00; //rotera vänster
			PORTD = 0x20;
			OCR2A = 60;
			OCR2B = 60;
		}

		TransmitSensor(trstraight);
		sensor1r = shortsensor(storedValues[1]);
		sensor2r = shortsensor(storedValues[2]);
	}

}

void driveF()
{
	// Riktning
	PORTC = 0x01;
	PORTD = 0x20;
	// Hastighet
	OCR2B = speed-3;
	OCR2A = speed+1;
}

void drive(float dist) //kör dist cm
{
	distance=0;
	dist = dist / 1.275625;

	while (distance < dist * 0.8)
	{
		TransmitSensor(0);
		driveF();
	}
	stopp();

}

void drivefromstill(float dist) //kör dist cm
{
	distance=0;
	dist = dist / 1.275625;

	while (distance < dist * 1)
	{
		TransmitSensor(0);
		driveF();
	}
	stopp();
}


void regulateright()
{
	transmit();
	//REGLERING
	//Omvandling till centimeter

	sensor1r = shortsensor(storedValues[1]);
	sensor2r = shortsensor(storedValues[2]);
	sensorfront = shortsensor(storedValues[3]);
	sensormeanr = ((sensor1r + sensor2r) / 2) + 4 ;
	
	// Uppdatera kommunikationsmodulen med sensorvärden innan reglering börjar
	if(firstRR == 1)
	{
		firstRR = 0;
		TransmitComm(update);

		stopp();
	}
	else
	{
		//till PD-reglering
		Td = 5; //85000000; //128000000
		K = 5;
		if(sensorfront <= 15)
		{
			if(posdistance > 25)
			{
				updatepos();
			}
			straight();
			rotate90left();
			straight();
		}
		if(sensor1r < 25 && sensor2r < 25)
		{
			startregulate = 1;
			PORTC = 0x01;
			PORTD = 0x20;
			rightpwm = speed + K * ((15 - sensormeanr)) + Td * (sensor2r - sensor1r);
			leftpwm = speed - K * ((15 - sensormeanr)) - Td * (sensor2r - sensor1r);

			if (rightpwm > 150)
			{
				OCR2B = 150;
			}
			else if(rightpwm < 0)
			{
				OCR2B = 0;
			}
			else
			{
				OCR2B = rightpwm;
			}
			if (leftpwm > 150)
			{
				OCR2A = 150;
			}
			else if (leftpwm < 0)
			{
				OCR2A = 0;
			}
			else
			{
				OCR2A = leftpwm;
			}
			
		}// Tappat konakt med bakhöger men inte framhöger
		else if(sensor2r > 20 && sensor1r < 20 )
		{
			driveF();
		}
		else // Tappat kontakt med framhöger
		{
			//har vi reglerat mot vägg innan
			if(startregulate == 1)
			{
				
				startregulate = 0;
				drive(20); // Kör fram till mitten av ruta
				updatepos();
				rotate90right(); // sväng höger
				drivefromstill(35); // Kör så att du har vägg att reglera mot
				updatepos();

				straight(); // korrigera om roboten står snett efter sväng
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
	if(myposX == startX && myposY == startY && mydirection == 1 && !start)
	{
		stopp();
		onelap = 1;
		start = 1;

		straight();

		start_request = 1;
		TransmitComm(firstdone); // Berätta för komm att första varvet är klart
		start_request = 0;
	}
	else
	{
		regulateright();
	}
}

/***********************SECONDLAP*************************/

// Hjälpfunktioner
bool alreadyDone()
{
	searched = false;
	TransmitComm(update); // uppdatera kartan på kommunikationsmodulen
	for(int i = 0; i < 500; i++){}
	// Fråga kommunikationsmodulen om vi har varit vid väggen vi ser
	TransmitComm(alrdyDone);
	return searched;
}

void gotoIsland()
{
	start_request = 1;
	TransmitSensor(0);
	sensorfront = shortsensor(storedValues[3]);
	while(sensorfront > 15) // Kör tills roboten står vid ön
	{
		driveF();
		traveledDist();
		
		TransmitSensor(0);
		sensorfront = shortsensor(storedValues[3]);
	}

	if(posdistance > 25) // korrigering för om vi inte har hamnat mitt i ruta
	{
		updatepos();
	}
	rotate90left(); // ställ dig längsvägg
	
	start_request = 1;
	TransmitSensor(0);
	sensor1r = shortsensor(storedValues[1]);
	sensor2r = shortsensor(storedValues[2]);
	
	// räta upp roboten om du har kontakt med fram och bak sensor höger
	if(sensor2r < 20 && sensor1r < 20)
	{
		straight();
	}
}

void storepos()
{
	start = true;
	storeposX = myposX;
	storeposY = myposY;
	storedirection = mydirection;
}

void throwpos()
{
	storeposX = 42;
	storeposY = 42;
	storedirection = 0;
}

void secondlap()
{
	if(myposX == startX && myposY == startY && mydirection == 1 && !start)	//Det här kommer gälla de första sekunderna roboten börjar köra också..!
	{
		home = true; // Klar med hela körningen
	}
	else
	{
		TransmitSensor(0);
		sensorleft = longsensor(storedValues[0]);
		if(sensorleft < 190 && sensorleft > 45 && controlisland)
		{
			controlisland = false;
			if(!alreadyDone())
			{
				Island();
			}
			
		}
		
		regulateright();
		
	}
}

void Island()
{
	straight();
	rotate90left();
	
	gotoIsland(); // kör till ön
	
	storepos();
	
	// Reglera runt ön tills vi är tillbaka till storepos
	while(myposX != storeposX || myposY != storeposY || mydirection != storedirection || start)
	{
		traveledDist();
		regulateright();
	}
	
	straight();
	rotate90left();

	TransmitComm(firstdone); // Meddela komm att vi kört runt en hel ö
	
	throwpos();
	gotoIsland(); // kör från ön
	start = 1;
	controlisland = false;
	
	
}


int main(void)
{
	initiate_variables();
	initiation();
	int fjarrstyrt = (PIND & 0x01); //1 då roboten är i fjärrstyrt läge
	initiate_request_timer();
	

	if(fjarrstyrt == 1) // fjärrstyrt läge
	{
		for(long i = 0; i < 480000; i++){}
		remotecontrol();
	}
	else // Autonom
	{
		MasterInit();

		for(long i = 0; i < 480000; i++){}

		while(home == 0)
		{
			TransmitSensor(0);
			
			char i = 0;
			if(n == 0)
			{
				i = 0;
			}
			else
			{
				i = 1;
			}
			if(posdistance > 31 + i)  //40/2.55125)*0.9
			{
				updatepos();
				if(n != 4)
				{
					n = n + 1;
				}
				else
				{
					n = 0;
				}
			}
			

			if(!onelap)
			{
				firstlap();
			}
			else if(1) // else vill inte fungera på atmegan
			{
				secondlap();
			}
		}
		stopp();
		
		// Liten dans när den är på startpositionen
		while(1)
		{
			rotate90right();
			rotate90left();
		}
	}
	return 0;
}

ISR(TIMER0_COMPB_vect)
{
	TCCR0B = 0b0000000; //stop timer
	TCNT0 = 0x00;
	start_request = 1;
}
