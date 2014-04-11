#include <avr/io.h>
#include <avr/delay.h>

int speed=150; //Standardhastiget i autonomt läge
int finished=0; //1 då hela kartan utforskad
int onelap=0; //1 då yttervarvet körts
int home=0; //1 då robten återvänt till startposition
int mydirection = 2; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
int traveled=0; //Hur långt roboten har färdats (sedan senast vi tog emot värden från sensor?)
unsigned myposX=0; //Robotens position i X-led
unsigned myposY=0; //Robotens position i Y-led
unsigned mypos[2]={0,0}; //Vektor med robotens koordinater; (x,y)
unsigned startpos[2]={0,0}; //Startpositionen sätts till origo

unsigned char gyro;
unsigned char sensorfront;
unsigned char sensor1right;
unsigned char sensor2right;
unsigned char sensor1left;
unsigned char sensor2left;

int map[17][31]; //=....


int main(void)
{
	//Sätter utgångar/ingångar
	DDRA=0b11111111;
	DDRB=0b00000000;
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
	
	int fjarrstyrt = (PIND & 0x01); //1 då roboten är i fjärrstyrt läge
	
	if(fjarrstyrt==1)
	{
		int button=PINB;
		fjarrstyr(button);
	}
	else
	{	
		while(home==0)
		{
			updatepos(traveled);
			if(onelap==0)
			{
				firstlap();
			}
			else if(finished==0)
			{
				//gör massa skit
			}
			else
			{
				returntostart();
			}
		}		
	}
	return 0;
}



void fjarrstyr(int button)
{
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


void rotate90left()
{
	int degrees=0;
	int ready=0;
	
	while(ready==0)
	{
		//ta emot data
		gyro=0;
		int sendGyro=0;
		if(degrees<90)
		{
			PORTC = 0x00;
			PORTD = 0x40;
			OCR2A = 150;
			OCR1A = 150;
			if(sendGyro >= 0)
			{
				sendGyro = sendGyro * 9.33;
			}
			else
			{
				sendGyro = sendGyro * 5,04;
			}
			gyro = sendGyro*64/244000;
			degrees+=gyro;
		}
		else if(mydirection==1)
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
	OCR2A = 0;
	OCR1A = 0;
}


void rotate90right()
{
	int degrees=0;
	int ready=0;
	int gyro;
	while(ready==0)
	{
		if(degrees<90)
		{
			PORTC = 0x01;
			PORTD = 0x00;
			OCR2A = 150;
			OCR1A = 150;
			gyro=getgyro();
			degrees+=gyro;
		}
		else if(mydirection==1)
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
	OCR2A = 0;
	OCR1A = 0;
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

	while(regleramotvagg==1)
	{
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

void updatepos(int traveled)
{
	switch(mydirection)
	{
		case (1): // X+
		if (traveled>=40)
		{
			myposX+=1;
			mypos[0]=myposX;
			traveled=0;
		}
		case (2): // Y+
		if (traveled>=40)
		{
			myposY+=1;
			mypos[1]=myposY;
			traveled=0;
		}
		case (3): // X-
		if (traveled>=40)
		{
			myposX-=1;
			mypos[0]=myposX;
			traveled=0;
		}
		case (4): // Y-
		if (traveled>=40)
		{
			myposY-=1;
			mypos[1]=myposY;
			traveled=0;
		}
	}
}
