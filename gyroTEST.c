#include <avr/io.h>
#include <avr/interrupt.h>

char calibrated = 0;
char dGyro;
long double hejsan;
char gyroref;
char sendGyro = 0x40;
long double digital_angle = 0;
long double angle = 0;
float ef = 5;
long timer = 0;
volatile long overflow;

volatile char ad_complete = 0;
volatile char dummy = 0;

long n = 0;
long double testgyro;



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
	ADMUX = 6;

}

void initiate_sample_timer()
{
	TIMSK1 = 0b00000100; //Enable interupt vid matchning med OCR1B TCCR1B =0x00;
	TCNT1 = 0x00;
	OCR1BH = 0x00;
	OCR1BL = 0x60; //RANDOM! När ska comparen triggas? SAMPLING
}

void initiate_gyro_timer()
{
	TIMSK0 = 0b00000001; //Enable interupt vid overflow
	TCCR0B = 0x00; //stop
	TCNT0 = 0x00; //set count
	TCCR0B = 0x03; //start timer prescale 64
}

void calculate_angle()
{
	if(digital_angle >= 0)
	{
		digital_angle = digital_angle * 0.02860048303038;
	}
	else
	{
		digital_angle = digital_angle * 0.02981401732052;
	}

	angle = angle + digital_angle;
}

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

int main(void)
{
	initiate_sensormodul();
	initiate_gyro_timer();
	initiate_sample_timer();
	while(1)
	{
		
		if(ad_complete == 1)
		{
			ad_complete = 0;
			if(calibrated == 0)
			{
				gyroref = ADC >> 2;
				ADMUX = 0;
				calibrated = 1;
				TCCR1B = 0x03;
			}
			else
			{
				dGyro = ADC >> 2;
				if ((dGyro < gyroref + 2) && (dGyro > gyroref - 2))
				{
					digital_angle = 0;
					TCNT0 = 0x00;
					overflow = 0;
				}
				else
				{
					digital_angle = (dGyro - gyroref) *5/256; //5V, 8 bitar
					timer = TCNT0;
					digital_angle = digital_angle * (timer + overflow * 64); //64 är prescalen på timern
					overflow = 0;
					TCNT0 = 0x00; //set count
				}
				calculate_angle();
				calculate_sendGyro();
				ADMUX = 6;
			}
			TCCR1B = 0x03; //Återstarta sampletimer
		}
	}
}

ISR(TIMER1_COMPB_vect)
{
	TCCR1B = 0x00;
	TCNT1 = 0x00;
	ADCSRA = 0b11001011;
}

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 0;
	overflow++;
}

ISR(INT0_vect) //knapp ska vi inte ha irl, men ja.
{
	dummy = 1;
	//:)
}

ISR(ADC_vect)
{
	ADCSRA = 0b10001011;
	ad_complete = 1;
}

//Gammal testkod som ska omtestas för omkalibrering av gyrot.
/*
ISR(ADC_vect)
{
	if(n == 0)
	{
		gyroref = ADC >> 2;
	}
	if(n < 20000)
	{
		n = n + 1;
		dGyro = (ADC >> 2);
		if ((dGyro < gyroref + 2) && (dGyro > gyroref - 2))
		{
			testgyro = testgyro;
		}
		else
		{
			hejsan = (dGyro - gyroref)*5/256; // 5V, 256 "delar"
			timer = TCNT0;
			hejsan = hejsan * (timer + 64 * overflow);
			overflow = 0;
			if(n !=19999)
			{
				TCNT0 = 0x00; //set count
			}
			if(hejsan >= 0)
			{
				hejsan = hejsan * 0.02860048303038;
			}
			else
			{
				hejsan = hejsan * 0.02981401732052;
			}
			testgyro = testgyro + hejsan;
		}
		ADCSRA = 0b11001011;
	}
	else
	{
		//testgyro /= 244000;
		dummy = testgyro;
		gyroref = 0;
		ADCSRA = 0b10001011;
		n = 0;
		testgyro = 0;
	}
}*/
