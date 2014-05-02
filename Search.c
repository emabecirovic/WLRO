/*
 * Search.c
 *
 * Created: 4/29/2014 9:26 AM
 *  Author: robsv107
 */

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h> // Robert
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
char finished=0; //1 då hela kartan utforskad
char onelap=0; //1 då yttervarvet körts
char home=0; //1 då robten återvänt till startposition
char mydirection = 2; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
char traveled=0; //Hur långt roboten har färdats (sedan senast vi tog emot värden från sensor?)
unsigned int myposX=0; //Robotens position i X-led
unsigned int myposY=0; //Robotens position i Y-led
unsigned int startpos[2]={15,0}; //Startpositionen sätts till mitten på nedre lånsidan
int firstzero; //Första nollan om man läser matrisen uppifrån och ned

unsigned char gyro;
unsigned char sensorfront;
unsigned char sensor1right;
unsigned char sensor2right;
unsigned char sensor1left;
unsigned char sensor2left;
 
char room[15][29]; //=.... 0=outforskat, 1=vägg, 2=öppen yta


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

	if(invalue == right)
	{
		MasterTransmit(rightfront);
		for(int i = 0; i < time; i++){}
		storedValues[0] = SPDR; // Front
		MasterTransmit(rightback);
		for(int i = 0; i < time; i++){}
		storedValues[1] = SPDR; // Right front
		MasterTransmit(stop);
		for(int i = 0; i < time; i++){}
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
		for(int i = 0; i < time; i++){}
		storedValues[4] = SPDR; // Left back
	}
	else if(invalue == turn)
	{
		MasterTransmit(gyro);
		for(int i = 0; i < time; i++){}
		storedValues[0] = SPDR; // Front
		MasterTransmit(stop);
		for(int i = 0; i < time; i++){}
		gyro = SPDR; // Gyro
	}
	else
	{
		MasterTransmit(stop);
		for(int i = 0; i < time; i++){}
		storedValues[0] = SPDR; // Front
	}

	PORTB ^= 0b00010000; // ss2 high
	//for(int i = 0; i < 100; i++){}
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

/******************************FJÄRSTYRNING**********************/
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
	room[y][x]=1;
}

void updatemap()
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
			room[myposX-1][myposY]=2;
		}
		else if(sensorfront<=w)
		{
			setwall(myposX+1,myposY);
			room[myposX-1][myposY]=2;
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX,myposY+1);
			room[myposX-1][myposY]=2;
		}
		case (2): // Y+
		if(sensormeanright<=w)
		{
			setwall(myposX+1,myposY);
			room[myposX][myposY-1]=2;
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY+1);
			room[myposX][myposY-1]=2;
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX-1,myposY);
			room[myposX][myposY-1]=2;
		}
		case (3): // X-
		if(sensormeanright<=w) 
		{
			setwall(myposX,myposY+1);
			room[myposX+1][myposY]=2;
		}
		else if(sensorfront<=w)
		{
			setwall(myposX-1,myposY);
			room[myposX+1][myposY]=2;
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX,myposY-1);
			room[myposX+1][myposY]=2;
		}
		case (4): // Y-
		if(sensormeanright<=w) 
		{
			setwall(myposX-1,myposY);
			room[myposX][myposY+1]=2;
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY-1);
			room[myposX][myposY+1]=2;
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX+1,myposY);
			room[myposX][myposY+1]=2;
		}
	}
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


/*********************************UPPRITNING AV FÖRSTA VARV*************************************/
void firstlap()
{
	if(mypos==startpos)
	{
		onelap=1;
	}
	else
	{
		regleringright();
	}
}

void regleringright()
{
	DDRC=0b00000001;
	DDRD=0b11100000;
	TCCR1A=0b10010001; //setup, phase correct PWM
	TCCR1B=0b00000010; //sätter hastigheten på klockan
	TCCR2A=0b10010001;
	TCCR2B=0b00000010;
	PORTC = 0x01; //sätter båda DIR till 1
	PORTD = 0x40;
	int K = 5; //Bestämmer hur snabbt roboten ska reagera på skillnader mellan önskat avstånd till väggen(10cm) och uppmätt avstånd
	
	
	if(start_request == 1)
		{
			//writechar(0b01001100); //L
			start_request = 0;
			TransmitSensor(right);
			TCCR0B = 0b0000101; // Start timer
		}
	
		int sensordiffr = sensor1r-sensor2r;
		int sensormeanr = (sensor1r+sensor2r)/2;
		
		if (sensorfront>10)
		{
		if(sensordiff<20) //Byt plats på höger och vänster för att reglera mot vänster vägg
		{
			OCR1A = 180+K*(9-sensormeanr); //PWM höger
			OCR2A = 180-K*(9-sensormeanr); //PWM vänster
		}
		else //sensordiff>20 innebär att ett hörn kommer
		{
			while(traveled<20)
			{
				OCR2A = 180;
				OCR1A = 180;	
			}
			rotate90right();	
			while(traveled<20)
			{
				OCR2A = 180;
				OCR1A = 180;
			}		
		}
	}
	else
	rotate90left();
}


void regleringleft()
{
	DDRC=0b00000001;
	DDRD=0b11100000;
	TCCR1A=0b10010001; //setup, phase correct PWM
	TCCR1B=0b00000010; //sätter hastigheten på klockan
	TCCR2A=0b10010001;
	TCCR2B=0b00000010;
	PORTC = 0x01; //sätter båda DIR till 1
	PORTD = 0x40;
	int K = 5; //Bestämmer hur snabbt roboten ska reagera på skillnader mellan önskat avstånd till väggen(10cm) och uppmätt avstånd

	while(regleramotvagg==1)
	{
		int sensordiffr = sensor1l-sensor2l;
		int sensormeanr = (sensor1l+sensor2l)/2;
		
		if (sensorfront>10)
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
		rotate90right();
	}
}

void fis()
{
	// Vänd roboten posetiv y-led
	while(mydirection != 2)
	{
		if(mydirection < 2)
		rotate90right();
		else
		rotate90left();
	}
	
	// Kör en sektion ut i öppen yta
	while(distance<40)
	{
		PORTC = 0x01;
		PORTD = 0x40;
		OCR2A = speed;
		OCR1A = speed;
	}
	
	rotate90right();
	
}

void prutt() //sicksacksak
{
	bool leftturn = true;
	bool first = true;
	

		if(sensorfront>50)
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
			leftturn = false;
			rotate90left2();
			int distance=0;
			while(distance<40)
			{
				PORTC = 0x01;
				PORTD = 0x40;
				OCR2A = speed;
				OCR1A = speed;
			}
			rotate90left2();
		}
		else
		{
			leftturn = true;
			rotate90right2();
			int distance=0;
			while(distance<40)
			{
				PORTC = 0x01;
				PORTD = 0x40;
				OCR2A = speed;
				OCR1A = speed;
			}
			rotate90right2();
			
		}
			
}

int findfirstzero()
{
	int i; //X
	int j; //Y
	int firstzero[2]={0,0};
	
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
	if(myposX!=pos[0])
	{
		if(pos[0]>myposX)
		{
			switch(mydirection)
			{
				case(1):
				PORTC = 0x01;
				PORTD = 0x40;
				OCR2A = speed;
				OCR0A = speed;
				case(2):
				rotate90right2();
				case(3):
				rotate90right2();
				rotate90right2();
				case(4):
				rotate90left2();
			}
		}
		else
		{
			switch(mydirection)
			{
				case(3):
				PORTC = 0x01;
				PORTD = 0x40;
				OCR2A = speed;
				OCR0A = speed;
				case(4):
				rotate90left2();
				case(1):
				rotate90right2();
				rotate90right2();
				case(2):
				rotate90right2();
			}
		}
		
	}
	else if (myposY!=pos[1])
	{
		if(pos[1]>myposY)
		{
			switch(mydirection)
			{
				case(2):
				PORTC = 0x01;
				PORTD = 0x40;
				OCR2A = speed;
				OCR0A = speed;
				case(3):
				rotate90right2();
				case(4):
				rotate90right2();
				rotate90right2();
				case(1):
				rotate90left2();
			}
		}
		else
		{
			switch(mydirection)
			{
				case(4):
				PORTC = 0x01;
				PORTB = 0x04;
				OCR2A = speed;
				OCR0A = speed;
				case(1):
				rotate90left2();
				case(2):
				rotate90right2();
				rotate90right2();
				case(3):
				rotate90right2();
			}
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
		while(home==0)
		{
			if(traveled >= 40/0.8125) //dividera med sektor 0.8125
			updatepos();
			
			
			if(onelap==0)
			{
				firstlap();
			}
			else if(finished==0)
			{
				fis();
				prutt();
				//Printa till lcd att roboten har pruttat klart
				
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
