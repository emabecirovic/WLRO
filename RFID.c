#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char data;
char isRFID = 0;
char taggen;
char dummy;


unsigned char tagg[12];
unsigned char B90[12] = {0x0A, 0x30, 0x31, 0x30, 0x30, 0x41, 0x36, 0x34, 0x38, 0x34, 0x42, 0x0D};
unsigned char B91[12] = {0x0A, 0x30, 0x31, 0x30, 0x30, 0x41, 0x36, 0x34, 0x32, 0x46, 0x44, 0x0D};
unsigned char B92[12] = {0x0A, 0x30, 0x31, 0x30, 0x30, 0x41, 0x36, 0x32, 0x33, 0x38, 0x39, 0x0D};
unsigned char B93[12] = {0x0A, 0x30, 0x31, 0x30, 0x30, 0x45, 0x31, 0x31, 0x39, 0x35, 0x33, 0x0D};
unsigned char B94[12] = {0x0A, 0x30, 0x31, 0x30, 0x30, 0x41, 0x36, 0x33, 0x41, 0x31, 0x44, 0x0D};
unsigned char B95[12] = {0x0A, 0x30, 0x31, 0x30, 0x30, 0x41, 0x37, 0x37, 0x33, 0x32, 0x42, 0x0D};

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

char compare_rfid(unsigned char a[], unsigned char b[])
{
	char is = 1;
	for (char i = 0; i < 12; i++)
	{
		if (a[i] != b[i])
		{
			is = 0;
		}
	}
	return is;
}

int main(void)
{
	MCUCR = 0b00000000;
	EIMSK = 0b00000001;
	EICRA = 0b00000011;
	DDRD = 0x00;
	ADCSRA = 0b10001011;

	sei();

	USART_Init(25);
	while(1)
	{
		data = USART_Receive();
		if (data == 0x0A)
		{
			data = 0;
			isRFID = 1;
			tagg[0]=0x0A;
			tagg[1]=USART_Receive();
			tagg[2]=USART_Receive();
			tagg[3]=USART_Receive();
			tagg[4]=USART_Receive();
			tagg[5]=USART_Receive();
			tagg[6]=USART_Receive();
			tagg[7]=USART_Receive();
			tagg[8]=USART_Receive();
			tagg[9]=USART_Receive();
			tagg[10]=USART_Receive();
			tagg[11]=USART_Receive();
			UCSR0B = (0<<RXEN0)|(0<<TXEN0);
			if (compare_rfid(tagg, B90))
			{
				taggen = 90;
			}
			else if (compare_rfid(tagg, B91))
			{
				taggen = 91;
			}
			else if (compare_rfid(tagg, B92))
			{
				taggen = 92;
			}
			else if (compare_rfid(tagg, B93))
			{
				taggen = 93;
			}
			else if (compare_rfid(tagg, B94))
			{
				taggen = 94;
			}
			else if (compare_rfid(tagg, B95))
			{
				taggen = 95;
			}
			else
			{
 				taggen = 0;
			}
			for (int n = 0; n < 200; n++)
			{
			}
			dummy = 0;
			dummy = 1;
			data = 0;
			for (char i = 0; i < 12; i++)
			{
				tagg[i] = 0;
			}
			UCSR0B = (1<<RXEN0)|(1<<TXEN0);
		}
	}
}


ISR(INT0_vect) //knapp
{
	taggen = 1;
}
