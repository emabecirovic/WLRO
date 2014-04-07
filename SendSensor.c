/*
 * SendSensor.c
 *
 * Created: 4/7/2014 8:44:12 AM
 *  Author: robsv107
 */ 


#include <avr/io.h>

/***********************MASTER*****************************/

typedef int bool;
enum {false, true};

//Start bytes for transmition
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveldist = 0b00000110;
char gyro = 0b00000111;
char RFID = 0b00001000;
char direction = 0b00001001;
char rightspeed = 0b00001010; 
char leftspeed = 0b00001011;
char stop = 0x00; //Stopbyte

//Control characters
bool regulateright = false;
bool regulateleft = false;
bool regulateturn = false;


void TransmitSensor(char invalue)
{
  PORTB &= 0b11101111; // ss2 low
  
  MasterTransmit(RFID);
  //First communication will contain crap on shiftregister
  MasterTransmit(front); // Request front sensor
  SensorRFID = SPDR; 
  
  if(invalue == right)
  {
    MasterTransmit(rightfront);
    SensorFront = SPDR; // Change so that value is stored in array or whatever Patrik wants to use
    MasterTransmit(rightback);
    SensorRightFront = SPDR;
    MasterTransmit(stop);
    SensorRightBack = SPDR;
  }
  else if(invalue == left)
  {
    MasterTransmit(leftfront);
    SensorFront = SPDR;
    MasterTransmit(leftback);
    SensorLeftBack = SPDR;
    MasterTransmit(stop);
    SensorLeftBack = SPDR;
  }
  else if(invalue == turn)
  {
    MasterTransmit(gyro)
    SensorFront = SPDR;
    MasterTransmit(stop)
    SensorGyro = SPDR;
  }
  else
  {
    MasterTransmit(stop);
  }
  
  PORTB ^= 0b00010000; // ss2 high
}



int main(void)
{
  if(regulateright)
    TransmitSensor(right);
  else if(regulateleft)
    TransmitSensor(left);
  else if(regulateturn)
    TransmitSensor(turn);
  else
    TransmitSensor(0x00);
}


/***********************SLAVE*****************************/

//Start bytes for transmition
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveldist = 0b00000110;
char gyro = 0b00000111;
char RFID = 0b00001000;
char stop = 0x00; //Stopbyte

Void Recieve




int main(void)
{
  
  while(0<<PORTB4) // slave selected
  {
    selection = SlaveRecieve();
    
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
      Distance = 0;
    }
    else if (selection == gyro)
    {
      SPDR = sendGyro;
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
}

