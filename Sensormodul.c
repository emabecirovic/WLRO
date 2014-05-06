#include <avr/io.h>
#include <avr/interrupt.h>

// Bussbeteckningar
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveldist = 0b00000110;
char gyro = 0b00000111;
char RFID = 0b00001000;
char stop = 0x00; //Stopbyte
volatile char selection; // Används i skicka avbrottet

//Konstanter som uppräkning i AD-omvandling
char i = 0; //Vilken sensor jag använder
char m = 0; //Hur många gånger jag har gått igenom sensorn.
const char samplings = 8;

unsigned char dFront[8];
unsigned char dRight_Front[8];
unsigned char dRight_Back[8];
unsigned char dLeft_Front[8];
unsigned char dLeft_Back[8];
unsigned char sortedValues[5];
unsigned char dDist;
unsigned char tempDistance;
char Distance = 0;

unsigned char rfid_data;
char isRFID = 0; //ETTA ELLER NOLLA!

//Gyrovariabler
char calibrated = 0;
char dGyro;
char gyroref;
char sendGyro = 0x40;
long double digital_angle = 0;
long double angle = 0;
long double ef = 5; //Felmarginal
long timer = 0;
long overflow = 0;

float dummy;

//Flaggor
char start_sample = 0;
volatile char ad_complete = 0;

//Bubble sort för att kunna ta ut medianen av avståndssensorer
void bubble_sort(unsigned char a[], int size)
{
	int k, l, temp;
	for (k = 0; k < (size - 1); ++k)
	{
		for (l = 0; l < size - 1 - k; ++l )
		{
			if (a[l] > a[l+1])
			{
				temp = a[l+1];
				a[l+1] = a[l];
				a[l] = temp;
			}
		}
	}
}

//Initiering av slave
void SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDRB = (1<<DDB6);
	/* Enable SPI */
	SPCR = (1<<SPE)|(1<<SPIE)|(1<<CPHA)|(1<<CPOL);
	/* Enable External Interrupt */
	// sei();
}

char SlaveRecieve(void) // Används inte just nu men....
{
	/*Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;
	/* Return Data Register */
	return SPDR;

}

//Initiering av USART för RFID-läsning
void USART_Init( unsigned int baud )
{
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
}

//Sensormodulens initiering
void initiate_sensormodul(void)
{
	MCUCR = 0b00000000;
	EIMSK = 0b00000001; //INT0-avbrott
	EICRA = 0b00000011; //Avbrott på uppflank INT0
	ADMUX = 6; //Gyro ligger på plats 6
	DDRA = 0x00;
	DDRD = 0x00;
	ADCSRA = 0b10001011;
	sei(); //Globala interrupts
	USART_Init(25);
	UCSR0B = (0<<RXEN0)|(0<<TXEN0); //Stäng av USART
	//Kalibrera gyro, sätt sedan ADMUX till 0 så att vi får Frontsensor
	ADCSRA = 0b11001011; //Starta AD-omvandling
	ADMUX = 0;
}

//Vet inte riktigt hur vi ska leta RFID, men det är ett mycket senare problem.
void find_RFID(void) 
{
	UCSR0B = (1<<RXEN0)|(1<<TXEN0); //Starta USART
	while(1)
	{
		rfid_data = USART_Receive();
		if (rfid_data == 0x0A)
		{
			isRFID = 1;
		}
	}
}

//Gyro-timer för att beräkna tid som har gått sen senaste gyromätning
void initiate_gyro_timer()
{
	TIMSK0 = 0b00000001; //Enable interupt vid overflow
	TCCR0B = 0x00; //stop
	TCNT0 = 0x00; //set count
	TCCR0B = 0x03; //start timer prescale 64
}

//Timer för samplehastighet
void initiate_sample_timer()
{
	TIMSK1 = 0b00000100; //Enable interupt vid matchning med OCR1B TCCR1B =0x0060;
	TCNT1 = 0x00;
	//TCCR1B = 0x03; //Starta samplingsräknare, presscale 64.
	OCR1BH = 0x00;
	OCR1BL = 0x60; //RANDOM! När ska comparen triggas? SAMPLING Borde vara oftare ATM
}

//Beräkna vinkel, KALIBRERA OM
void calculate_angle()
{
	if(digital_angle >= 0)
	{
		digital_angle = digital_angle * 0.02530364372469;
	}
	else
	{
		digital_angle = digital_angle * 0.02197748528729;
	}

	angle = angle + digital_angle;
}

//Beräknar vilket värde som ska skickas till styrmodulen
void calculate_sendGyro()
{
	sendGyro = 0;

	if (angle < -180 - ef)
	{
		angle = angle + 360;
	}
	else if (angle > 180 + ef)
	{
		angle = angle - 360;
	}

	//Olika koder beroende på vinkel. Felmarginal ef. Koderna har Ema.
	if(angle >= -180 - ef && angle < -180 + ef)
	{
		sendGyro = 0x00;
	}
	else if (angle >= -180 + ef && angle < -90 - ef)
	{
		sendGyro = 0x10;
		for(char i = 0; i < 16; i++)
		{
			if(angle + 180 >= i * 5.625 && angle + 180 < (i + 1) * 5.625)
			{
				sendGyro = sendGyro + i;
				break;
			}
		}
	}
	else if(angle >= -90 - ef && angle < -90 + ef)
	{
		sendGyro = 0x20;
	}
	else if (angle >= -90 + ef && angle < 0 - ef)
	{
		sendGyro = 0x30;
		for(char i = 0; i < 16; i++)
		{
			if(angle + 90 >= i * 5.625 && angle + 90 < (i + 1) * 5.625)
			{
				sendGyro = sendGyro + i;
				break;
			}
		}
	}
	else if(angle >= 0 - ef && angle < 0 + ef)
	{
		sendGyro = 0x40;
	}
	else if (angle >= 0 + ef && angle < 90 - ef)
	{
		sendGyro = 0x50;
		for(char i = 0; i < 16; i++)
		{
			if(angle >= i * 5.625 && angle < (i + 1) * 5.625)
			{
				sendGyro = sendGyro + i;
				break;
			}
		}
	}
	else if(angle >= 90 - ef && angle < 90 + ef)
	{
		sendGyro = 0x60;
	}
	else if (angle >= 90 + ef && angle < 180 - ef)
	{
		sendGyro = 0x70;
		for(char i = 0; i < 16; i++)
		{
			if(angle - 90 >= i * 5.625 && angle - 90 < (i + 1) * 5.625)
			{
				sendGyro = sendGyro + i;
				break;
			}
		}
	}
	else if(angle >= 180 - ef && angle < 180 + ef)
	{
		sendGyro = 0x80;
	}
}

//Mainfunktion
int main(void)
{
	SlaveInit();
	initiate_sensormodul();
	initiate_gyro_timer();
	initiate_sample_timer();
	while(1)
	{
		if(start_sample == 1)
		{
			start_sample = 0;
			//find_RFID();
		}
		else
		{
			dummy = 1;
		}

		asm("");

		//När AD-omvandling är klar så sätts ad_complete och då görs dessa beräkningar
		if(ad_complete == 1)
		{
			ad_complete = 0;
			
			//Kalibrera gyrot första gången
			if(calibrated == 0)
			{
				gyroref = ADC >> 2;
				ADMUX = 0;
				calibrated = 1;
				TCCR1B = 0x03;
			}
			else
			{
				//Front
				if(i == 0)
				{
					dFront[m] = ADC >> 2;
					if (m == samplings - 1)
					{
						bubble_sort(dFront, samplings);
						sortedValues[0] = dFront[1];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				//Right Front
				else if(i == 1)
				{
					dRight_Front[m] = ADC >> 2;
					if (m == samplings - 1)
					{
						bubble_sort(dRight_Front, samplings);
						sortedValues[1] = dRight_Front[1];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				//Right Back
				else if(i == 2)
				{
					dRight_Back[m] = ADC >> 2;
					if (m == samplings - 1)
					{
						bubble_sort(dRight_Back, samplings);
						sortedValues[2] = dRight_Back[1];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				//Left Front
				else if(i == 3)
				{
					dLeft_Front[m] = ADC >> 2;
					if (m == samplings - 1)
					{
						bubble_sort(dLeft_Front, samplings);
						sortedValues[3] = dLeft_Front[1];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				//Left Back
				else if(i == 4)
				{
					dLeft_Back[m] = ADC >> 2;
					if (m == samplings - 1)
					{
						bubble_sort(dLeft_Back, samplings);
						sortedValues[4] = dLeft_Back[1];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				//Fototransistor
				else if(i == 5)
				{
					tempDistance = dDist;
					dDist = ADC >> 2;
					//Om den nya och den gamla ligger på olika sidor på 150 så ska Distance räknas upp. Det betyder att vi har gått förbi ett segment på skivan.
					if (((tempDistance <= 150) && (dDist > 150)) | ((tempDistance >= 150) && (dDist < 150)))
					{
						Distance = Distance + 1;
					}
					i = i + 1;
					ADMUX = i;
				}
				//Gyro
				else if(i == 6)
				{
					dGyro = ADC >> 2;
					//Har en felmarginal på 1
					if ((dGyro < gyroref + 2) && (dGyro > gyroref - 2))
					{
						digital_angle = 0;
						timer = TCNT0;
						TCNT0 = 0x00;
						overflow = 0;
					}
					else
					{
						digital_angle = dGyro - gyroref;
						timer = TCNT0;
						digital_angle = digital_angle * (timer + overflow * 256); //64 är prescalen på timern
						overflow = 0;
						TCNT0 = 0x00; //set count
					}
					calculate_angle();
					calculate_sendGyro();
					i = 0;
					ADMUX = i;
				}
			}
			//Starta samplingsräknare
			TCCR1B = 0x03; 
		}
	}
}

//Avbrott för sampletid
ISR(TIMER1_COMPB_vect)
{
	TCCR1B = 0x00;
	TCNT1 = 0x00;
	start_sample = 1;
	ADCSRA = 0b11001011;
}

//Avbrott när vi går över bitmängden för gyro-timer
ISR(TIMER0_OVF_vect)
{
	TCNT0 = 0;
	overflow++;
}

//Avbrott för knapp
ISR(INT0_vect) //knapp ska vi inte ha irl, men ja.
{
	dummy = 0;
	//:)
}

//Avbortt för AD-omvandlingen är klar
ISR(ADC_vect)
{
	ADCSRA = 0b10001011;
	ad_complete = 1;
}

//Avbrott för buss klar
ISR(SPI_STC_vect) // Skicka på buss!! // Robert
{
	SPDR = 0; //Dummyskrivning
	selection = SPDR;
	if(selection == front)
	{
		SPDR = sortedValues[0];
	}
	else if (selection == rightfront)
	{
		SPDR = sortedValues[1];
		//SPDR = 125;
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
		asm("");
		//Distance = 0;

	}
	else if (selection == gyro)
	{
		SPDR = sendGyro;
		asm("");
		//sendGyro = 0;
	}
	else if (selection == RFID)
	{
		SPDR = isRFID;
	}
	else if (selection == stop)
	{
		dummy = 1;
		// behöver förmodligen inte göra något här
	}
}

/* GAMMALT SOM HAR ANVÄNTS I TESTSYFTE */
/*ISR(ADC_vect)
{
if (n == 1)
{
tempDistance = dDist;
dDist = ADC >> 2;
if (((tempDistance <= 150) && (dDist > 150)) | ((tempDistance >= 150) && (dDist < 150)))
{
Distance = Distance + 1;
}
ADCSRA = 0b11001011;
}
if (n == 0)
{
ADCSRA = 0b10001011;
}
}*/
/*ISR(ADC_vect)
{
if(n == 0)
{
gyroref = ADC >> 2;
}
if(n < 20000)
{
n = n + 1;
dGyro = ADC >> 2;
if ((dGyro < gyroref + 2) && (dGyro > gyroref - 2))
{
testgyro = testgyro;
}
else
{
dGyro= dGyro - gyroref;
timer = TCNT0;
dGyro = dGyro * timer;
TCNT0 = 0x00; //set count
//dGyro = dGyro / 244000;
if(dGyro >= 0)
{
dGyro = dGyro * 9.33;
}
else
{
dGyro = dGyro * 5,04;
}
testgyro = testgyro + dGyro;
}
ADCSRA = 0b11001011;
}
else
{
testgyro /= 244000;
dummy = testgyro;
gyroref = 0;
ADCSRA = 0b10001011;
n = 0;
testgyro = 0;
}
}*/
//I PATRIKS KOD BORDE GYROT BETE SIG SÅHÄR TYP
/*
if(sendGyro >= 0)
{
sendGyro = sendGyro * 9.33;
}
else
{
sendGyro = sendGyro * 5,04;
}
senastgrader = sendGyro*64/244000;
grader = grader + senastgrader;
*/
//I PATRIKS KOD BORDE FRAMSENSORN BETE SIG SÅHÄR TYP
/*
ICENTIMETER = ((1/DIGITALT) - 0,001086689563586) / 0,000191822821525;
*/
//I PATRIKS KOD BORDE SIDOSENSORN BETE SIG SÅHÄR TYP
/*
ICENTIMETER = ((1/DIGITALT) - 0,000741938763948) / 0,001637008132828;
*/
