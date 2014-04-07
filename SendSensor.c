/*
 * SendSensor.c
 *
 * Created: 4/7/2014 8:44:12 AM
 *  Author: robsv107
 */ 


#include <avr/io.h>

// Master

//Start bytes for transmition
char front = 0b00000001;
char rightfront = 0b00000010;
char rightback = 0b00000011;
char leftfront = 0b00000100;
char leftback = 0b00000101;
char traveleddist = 0b00000110;
char gyro = 0b00000111;
char RFID = 0b00001000;
char direction = 0b00001001;
char rightspeed = 0b00001010; 
char leftspeed = 0b00001011;
char stop = 0x00; //Stopbyte

void TransmitSensor(char invalue)
{
  PORTB &= 0b11101111; // ss2 low
  
  MasterTransmit(RFID);
  //First communication will contain crap on shiftregister
  MasterTransmit(front); // Request front sensor
  SensorRFID = SPDR; 
  
  if(invalue == hoger)
  {
    MasterTransmit(rightfront);
    SensorFront = SPDR; // Change so that value is stored in array or whatever Patrik want to use
    MasterTransmit(rightback);
    SensorRightFront = SPDR;
    MasterTransmit(stop);
    SensorRightBack = SPDR;
  }
  else if(invalue == vanster)
  {
    MasterTransmit(leftfront);
    SensorFront = SPDR;
    MasterTransmit(leftback);
    SensorLeftBack = SPDR;
    MasterTransmit(stop);
    SensorLeftBack = SPDR;
  }
  else if(invalue == svanga)
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
  if(reglerarhoger)
    TransmitSensor(hoger);
  else if(reglervanster)
    TransmitSensor(vanster);
  else if(reglersvanga)
    TransmitSensor(svanga);
  else
    TransmitSensor(0x00);
}

