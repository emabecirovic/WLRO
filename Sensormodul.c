#include <avr/io.h>
#include <avr/interrupt.h>


// Namn istället för massa jobbiga siffror /Robert
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveldist = 0b00000110;
char gyro = 0b00000111;
char RFID = 0b00001000;
char stop = 0x00; //Stopbyte

char selection; // Används i skicka avbrottet


char i = 0; //Vilken sensor jag använder
char m = 0; //Hur många gånger jag har gått igenom sensorn.
char calibrated = 0;

unsigned char dFront[10];
unsigned char dRight_Front[10];
unsigned char dRight_Back[10];
unsigned char dLeft_Front[10];
unsigned char dLeft_Back[10];
unsigned char sortedValues[5];
unsigned char dDist;
unsigned char tempDistance;
char Distance = 0;
char dGyro;

unsigned char rfid_data;
char isRFID = 0; //ETTA ELLER NOLLA!

char gyroref;
signed int sendGyro = 0;
float dummy;
long n = 0;
long timer = 0;
long j = 0;
char start_sample = 0;
char ad_complete = 0;

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


void SlaveInit(void)
{
	/* Set MISO output, all others input */
	DDRB = (1<<DDB6);

	/* Enable SPI */
	SPCR = (1<<SPE)|(1<<SPIE);

	/* Enable External Interrupt */
//	sei();
}

char SlaveRecieve(void) // Används inte just nu men....
{
	/*Wait for reception complete */
	while(!(SPSR & (1<<SPIF)))
	;

	/* Return Data Register */
	return SPDR;
}


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


void initiate_sensormodul(void)
{
	MCUCR = 0b00000000;
	EIMSK = 0b00000001;
	EICRA = 0b00000011;

	ADMUX = 6;
	DDRA = 0x00;
	DDRD = 0x00;
	ADCSRA = 0b10001011;

	sei();
	USART_Init(25);
	UCSR0B = (0<<RXEN0)|(0<<TXEN0); //Stäng av USART

	ADCSRA = 0b11001011;
	ADMUX = 0;

	TCCR0B = 0x00; //stop
	TCNT0 = 0x00; //set count
	OCR0B  = 0x04;  //set compare
	TCCR0B = 0x03; //start timer
}

void find_RFID(void) //Vet inte riktigt hur vi ska leta RFID, men det är ett mycket senare problem.
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

void initiate_sample_timer()
{
	TIMSK1 = 0b00000100; //Enable interupt vid matchning med OCR1B		TCCR1B =0x00;
	TCNT1 = 0x00;
	TCCR1B = 0x03; //Starta samplingsräknare, presscale 64.
	OCR1BH = 0x00;
	OCR1BL = 0x60; //RANDOM! När ska comparen triggas? SAMPLING
}

int main(void)
{

	initiate_sensormodul();
	initiate_sample_timer();
	while(1)
	{
		if(start_sample == 1)
		{
			start_sample = 0;
			//find_RFID();
		}
		if(ad_complete == 1)
		{	
			ad_complete = 0;
			
			if(calibrated == 0)
			{
				gyroref = ADC >> 2;
				ADMUX = 0;
				calibrated = 1;
			}
			else
			{
				if(i == 0)
				{
					dFront[m] = ADC >> 2;
					if (m == 9)
					{
						bubble_sort(dFront, 10);
						sortedValues[0] = dFront[2];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				if(i == 1)
				{
					dRight_Front[m] = ADC >> 2;
					if (m == 9)
					{
						bubble_sort(dRight_Front, 10);
						sortedValues[1] = dRight_Front[2];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				if(i == 2)
				{
					dRight_Back[m] = ADC >> 2;
					if (m == 9)
					{
						bubble_sort(dRight_Back, 10);
						sortedValues[2] = dRight_Back[2];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				if(i == 3)
				{
					dLeft_Front[m] = ADC >> 2;
					if (m == 9)
					{
						bubble_sort(dLeft_Front, 10);
						sortedValues[3] = dLeft_Front[2];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				if(i ==	4)
				{
					dLeft_Back[m] = ADC >> 2;
					if (m == 9)
					{
						bubble_sort(dLeft_Back, 10);
						sortedValues[4] = dLeft_Back[2];
						i = i + 1;
						m = 0;
						ADMUX = i;
					}
					else
					{
						m = m + 1;
					}
				}
				if(i == 5)
				{
					tempDistance = dDist;
					dDist = ADC >> 2;
					if (((tempDistance <= 150) && (dDist > 150)) | ((tempDistance >= 150) && (dDist < 150)))
					{
						Distance = Distance + 1;
					}
					i = i + 1;
					ADMUX = i;
				}
				if(i == 6)
				{
					dGyro = ADC >> 2;
					if ((dGyro < gyroref + 2) && (dGyro > gyroref - 2))
					{
						sendGyro = sendGyro;
					}
					else
					{
						dGyro= dGyro - gyroref;
						timer = TCNT0;
						sendGyro = sendGyro + dGyro * timer; //64 är prescalen på timern
						TCNT0 = 0x00; //set count
					}
					i = 0;
					ADMUX = i;
				}
			}
		}
	}
}

ISR(TIMER1_COMPB_vect)
{
	TCNT1 = 0x00;
	start_sample = 1;
	ADCSRA = 0b11001011;	
}


ISR(INT0_vect) //knapp ska vi inte ha irl, men ja.
{
	//:)
}




ISR(ADC_vect)
{
	ad_complete = 1;
	ADCSRA = 0b10001011;
}



ISR(SPI_STC_vect) // Skicka på buss!! // Robert
{
	char selection = SPDR;

	if(selection == front)
	{
		SPDR = sortedValues[0];
	}
	else if (selection == rightfront)
	{
		SPDR = sortedValues[1];
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
		asm("");
		sendGyro = 0;
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

ICENTIMETER = ((1/DIGITALT) -  0,001086689563586) / 0,000191822821525;

*/

//I PATRIKS KOD BORDE SIDOSENSORN BETE SIG SÅHÄR TYP
/*

ICENTIMETER = ((1/DIGITALT) - 0,000741938763948) / 0,001637008132828;

*/
