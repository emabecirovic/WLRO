#include <avr/io.h>
#include <avr/interupt.h>

//Börjar med att klistra in från AVR-labben
int main(void)
{
    MCUCR = 0b00000011;
    GICR = 0b01000000;
    ADMUX = 0;
    DDRA = 0x00;
    //DDRC = 0x06;
    DDRD = 0xFF;
    ADCSRA = 0b10001011;

    char i = 0; //Vilken sensor jag använder

    char dFront;
    char dRight_Front;
    char dRight_Back;
    char dLeft_Front;
    char dLeft_Back;
    char dDist;
    char dGyro;


    sei();
    while(1) {}

    if(/*Master säger att jag ska skicka*/)
    {
        //ADMUX = i;
        //ADCSRA = 0b11001011;
    }
}

ISR(INT0_vect) //knapp
{
    ADMUX = 0;
    ADCSRA = 0b11001011;
}

ISR(ADC_vect)
{

    i = i + 1;
    if(i == 1)
    {
        dFront = ADC >> 2;
    }
    if(i == 2)
    {
        dRifht_Front = ADC >> 2;
    }
    if(i == 3)
    {
        dRifht_Back = ADC >> 2;
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
        dDist = ADC >> 2;
    }
    if(i == 7)
    {
        dGyro = ADC >> 2;
    }
    if(i== 7)
    {
        i = 0;
        ADCSRA = 0b10001011;
    }
    else
    {
        ADMUX = i;
        ADCSRA = 0b11001011;
    }

}

void send_front()
{
    //skicka först 1;
}

void send_fight_front()
{
    //skicka först 2;
}

void send_right_back()
{
    //skicka först 3;
}

void send_left_front()
{
    //skicka först 4;
}

void send_left_back()
{
    //skicka först 5;
}

void send_dist()
{
    //skicka först 6;
}

void send_gyro()
{
    //skicka först 7;
}

void send_RFID()
{
    //skicka först 8;
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






