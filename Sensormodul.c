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
char gyrostop = 0b10000000;
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
volatile char sendGyro = 0;
volatile long double angle = 0;
volatile char gyroflag = 0;

char counter_distance = 0;

float dummy;

//Flaggor
char start_sample = 0;
volatile char ad_complete = 0;

//Bubble sort för att kunna ta ut medianen av avståndssensorer

void initiate_variables()
{
	// Bussbeteckningar
	front = 0b00000001;
	rightfront = 0b00000010;
	rightback = 0b00000011;
	leftfront = 0b00000100;
	leftback = 0b00000101;
	traveldist = 0b00000110;
	gyro = 0b00000111;
	gyrostop = 0b10000000;
	RFID = 0b00001000;
	stop = 0x00; //Stopbyte

	//Konstanter som uppräkning i AD-omvandling
	i = 0; //Vilken sensor jag använder
	m = 0; //Hur många gånger jag har gått igenom sensorn.

	Distance = 0;

	isRFID = 0; //ETTA ELLER NOLLA!

	//Gyrovariabler
	calibrated = 0;
	dGyro = 0;
	gyroref = 0;
	sendGyro = 0;
	angle = 0;
	gyroflag = 0;

	counter_distance = 0;

	//Flaggor
	start_sample = 0;
	ad_complete = 0;
}

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
	/* Enable receiver and transmitter, enable receive interrupt */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
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
	//Kalibrera gyro, sätt sedan ADMUX till 0 så att vi får Frontsensor
	ADCSRA = 0b11001011; //Starta AD-omvandling
	ADMUX = 0;
}

//Timer för samplehastighet
void initiate_sample_timer()
{
	TIMSK1 = 0b00000100; //Enable interupt vid matchning med OCR1B TCCR1B =0x0060;
	TCNT1 = 0x00;
	TCCR1B = 0x03; //Starta samplingsräknare, presscale 64.
	OCR1BH = 0x00;
	OCR1BL = 0x30; //RANDOM! När ska comparen triggas? SAMPLING Borde vara oftare ATM
}

//Mainfunktion
int main(void)
{
	initiate_variables();
	SlaveInit();
	initiate_sensormodul();
	initiate_sample_timer();
	while(1)
	{
		/*
		if (dGyro == 255)
		{
			dummy = 0;
		}
		else if(gyroref == 255)
		{
			dummy = 1;
		}
		else if(gyroflag == 255)
		{
			dummy = 0;
		}
		*/
		
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
				ADMUX = 6;
				calibrated = 1;
				TCCR1B = 0x03;
			}
			else if(gyroflag == 0)
			{
				if (counter_distance < 2)
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
							i = 0;
							m = 0;
							ADMUX = i;
						}
						else
						{
							m = m + 1;
						}
					}

					if (counter_distance == 1)
					{
						//Nästa gång så ska vi ad-omvandla fototransistor
						ADMUX = 5;
					}

					counter_distance++;

				}//coutner_distance < 2
				else
				{
					//Fototransistor
					tempDistance = dDist;
					dDist = ADC >> 2;
					//Om den nya och den gamla ligger på olika sidor på 150 så ska Distance räknas upp. Det betyder att vi har gått förbi ett segment på skivan.
					if (((tempDistance <= 150) && (dDist > 150)) | ((tempDistance >= 150) && (dDist < 150)))
					{
						Distance = Distance + 1;
					}
					ADMUX = i; //Återgå till gammal i
					counter_distance = 0;
				}//counter_distance >= 2
				
			}//gyroflag == 0
			else if(gyroflag == 1)
			{
				signed int bigvalue = 83;
				signed int smallvalue = -83;
				dGyro = ADC >> 2;
				if((dGyro < gyroref + 2) && (dGyro > gyroref - 2))
				{
					angle = angle;
				}
				else
				{
					angle +=  (dGyro - gyroref)*5/256;
				}

				//Kolla om vi kommit fram till önskat värde
	
				if(angle >= bigvalue)
				{
					sendGyro = 1;
				}
				else if (angle <= smallvalue)
				{
					sendGyro = 2;
				}
				else
				{
					sendGyro = 0;
				}

			}//gyroflag == 1
			//Starta samplingsräknare
			TCCR1B = 0x03;
		}//ad_complete == 1
	}//while
}//main

//Avbrott för sampletid
ISR(TIMER1_COMPB_vect)
{
	TCCR1B = 0x00;
	TCNT1 = 0x00;
	start_sample = 1;
	ADCSRA = 0b11001011;
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
		Distance = 0;

	}
	else if (selection == gyro)
	{
		SPDR = sendGyro;

		gyroflag = 1;
		ADMUX = 6;
		asm("");
	}
	else if (selection == gyrostop) // här är den riktiga gyrostop
	{
		SPDR = 0;
		angle = 0;
		sendGyro = 0;
		gyroflag = 0;
		ADMUX = i;
		dummy = 1;
	}
	else if (selection == RFID)
	{
		SPDR = isRFID;
	}
	else if (selection == stop)
	{
		// behöver förmodligen inte göra något här
	}
}

ISR(USART0_RX_vect)
{
	rfid_data = UDR0;
	if(rfid_data == 0x0A)
	{
		isRFID = 1;
	}
	dummy = 0;
}
