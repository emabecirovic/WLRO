#include <avr/io.h>
#include <avr/interrupt.h>


char i = 0; //Vilken sensor jag använder

char dFront;
char dRight_Front;
char dRight_Back;
char dLeft_Front;
char dLeft_Back;
char dDist;
float dGyro;

char gyroref;
float testgyro = 0;
float dummy;
long n = 0;
long hej = 0;
long j = 0;

int main(void)
{
	//MCUCR = 0b00000011; //atmega16
	//GICR = 0b01000000; //atmega16

	MCUCR = 0b00000000;
	EIMSK = 0b00000001;
	EICRA = 0b00000011;

	ADMUX = 0;
	DDRA = 0x00;
	//DDRC = 0x06;
	//DDRD = 0x00;
	ADCSRA = 0b10001011;





	sei();

	while(1)
	{
		//TODO:: Please write your application code
	}
}

ISR(INT0_vect) //knapp
{
	ADMUX = 6;
	//ADMUX = 0;
	ADCSRA = 0b11001011;
	TCCR0B = 0x00; //stop
	TCNT0 = 0x00; //set count
	OCR0B  = 0x04;  //set compare
	TCCR0B = 0x01; //start timer
}

ISR(ADC_vect)
{
	
	
	//clock_t start = clock(), diff;
	
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
			hej = TCNT0;
			dGyro = dGyro * hej;
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
		//diff = clock() - start;
		//int msec = diff * 1000 / CLOCKS_PER_SEC;
		//hej = TCNT0;
		//testgyro = testgyro / n;

		/*if(testgyro >= 0)
		{
			dummy = testgyro * 9.33;
		}
		else
		{
			dummy = testgyro * 5,04;
		}*/
		testgyro /= 244000;
		
		/*while (j < 700)
		{
			j++;
		}*/
		dummy = testgyro;
		gyroref = 0;
		ADCSRA = 0b10001011;
		
		n = 0;
		testgyro = 0;
	}	
}


/*ISR(ADC_vect)
{

	i = i + 1;
	if(i == 1)
	{
		dFront = ADC >> 2;
	}
	if(i == 2)
	{
		dRight_Front = ADC >> 2;
	}
	if(i == 3)
	{
		dRight_Back = ADC >> 2;
	}
	if(i == 4)
	{
		dLeft_Front = ADC >> 2;
	}
	if(i == 5)
	{
		dLeft_Back = ADC >> 2;
	}
	if(i == 6)
	{
		dDist = ADC >> 2;
	}
	if(i == 7)
	{
		dGyro = ADC >> 2;
		i = 0;
		ADCSRA = 0b10001011;
	}
	else
	{
		ADMUX = i;
		ADCSRA = 0b11001011;
	}

}*/



void send_front()
{
	//skicka f?rst 1;
}

void send_fight_front()
{
	//skicka f?rst 2;
}

void send_right_back()
{
	//skicka f?rst 3;
}

void send_left_front()
{
	//skicka f?rst 4;
}

void send_left_back()
{
	//skicka f?rst 5;
}

void send_dist()
{
	//skicka f?rst 6;
}

void send_gyro()
{
	//skicka f?rst 7;
}

void send_RFID()
{
	//skicka f?rst 8;
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
