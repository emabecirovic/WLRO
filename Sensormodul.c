
//Börjar med att klistra in från AVR-labben
int main(void)
{
    MCUCR = 0b00000011;
    GICR = 0b01000000;
    ADMUX = 0;
    DDRB = 0xFF;
    ADCSRA = 0b10001011;

    sei();
    while(1) {}

    //Pseudokod typ fast inte
    /*Start
    Setup
    while (styr vill ha data)
    {
    if RFID
	minns rfid
    adomvandla sensorvärden
    omvandla till rätt enhet
    skicka till styr
    }*/

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





ISR(INT0_vect) //knapp
{
    ADCSRA = 0b11001011;
}

ISR(ADC_vect)
{
    PORTB = ADC >> 2;
}
