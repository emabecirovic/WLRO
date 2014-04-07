#include <avr/io.h>
#include <avr/interrupt.h>


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

char isRFID = 0; //ETTA ELLER NOLLA!

char gyroref;
signed int sendGyro = 0;
float dummy;
long n = 0;
long timer = 0;
long j = 0;

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
	
	ADCSRA = 0b11001011;
}

int main(void)
{

	initiate_sensormodul();
	while(1)
	{
		//TODO:: Please write your application code
	}
}


ISR(INT0_vect) //knapp
{
	ADMUX = 0;
	ADCSRA = 0b11001011;
	TCCR0B = 0x00; //stop
	TCNT0 = 0x00; //set count
	OCR0B  = 0x04;  //set compare
	TCCR0B = 0x03; //start timer
}




ISR(ADC_vect)
{
	if(calibrated == 0)
	{
		gyroref = ADC >> 2;
		ADMUX = 0;
		ADCSRA = 0b10001011;
		calibrated = 1;
	}
	else
	{
		if(i == 0)
		{
			dFront[m] = ADC >> 2;
		}
		if(i == 1)
		{
			dRight_Front[m] = ADC >> 2;
		}
		if(i == 2)
		{
			dRight_Back[m] = ADC >> 2;
		}
		if(i == 3)
		{
			dLeft_Front[m] = ADC >> 2;
		}
		if(i == 4)
		{
			dLeft_Back[m] = ADC >> 2;
		}
		if(i == 5)
		{
			tempDistance = dDist;
			dDist = ADC >> 2;
			if (((tempDistance <= 150) && (dDist > 150)) | ((tempDistance >= 150) && (dDist < 150)))
			{
				Distance = Distance + 1;
			}
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
		}
		if(i == 6)
		{
			i = 0;
			m = 0;
			bubble_sort(dFront, 10);
			bubble_sort(dRight_Front, 10);
			bubble_sort(dRight_Back, 10);
			bubble_sort(dLeft_Front, 10);
			bubble_sort(dLeft_Back, 10);
			sortedValues[0] = dFront[2];
			sortedValues[1] = dRight_Front[2];
			sortedValues[2] = dRight_Back[2];
			sortedValues[3] = dLeft_Front[2];
			sortedValues[4] = dLeft_Back[2];
			ADCSRA = 0b10001011;
		}
		else
		{
			if (m == 9)
			{
				i = i + 1;
				ADMUX = i;
				m = 0;
			}
			else
			{
				if(i < 5)
				{
					m = m + 1;
				}
				else
				{
					i = i + 1;
					ADMUX = i;
				}
			}
			ADCSRA = 0b11001011;
		}
	}
}


void send_front()
{
	//skicka f?rst 1;
	//sortedValues[0];
}

void send_right_front()
{
	//skicka f?rst 2;
	//sortedValues[1];
}

void send_right_back()
{
	//skicka f?rst 3;
	//sortedValues[2];
}

void send_left_front()
{
	//skicka f?rst 4;
	//sortedValues[3];
}

void send_left_back()
{
	//skicka f?rst 5;
	//sortedValues[4];
}

void send_dist()
{
	//skicka f?rst 6;
	//Distance;
	Distance = 0;
}

void send_gyro()
{
	//skicka f?rst 7;
	//sendGyro;
	sendGyro = 0;
}

void send_RFID()
{
	//skicka f?rst 8;
	//isRFID;
}

void convert_front()
{
}

void convert_fight_front()
{
}

void convert_right_back()
{
}

void convert_left_front()
{
}

void convert_left_back()
{
}

void convert_dist()
{
}

void convert_gyro()
{
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
