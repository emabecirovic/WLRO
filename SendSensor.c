/*
 * SendSensor.c
 *
 * Created: 4/7/2014 8:44:12 AM
 *  Author: robsv107
 */ 


#include <avr/io.h>

/***********************MASTER Styr*****************************/

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

//Control signals
char right = 1;
char left = 2;
char turn = 3;
bool remoteControl = false;  // Change to Port connected to switch
bool regulateright = false;
bool regulateleft = false;
bool regulateturn = false;

unsigned char storedValues[11];


void TransmitSensor(char invalue)
{
  PORTB &= 0b11101111; // ss2 low
  
  MasterTransmit(RFID);
  //First communication will contain crap on shiftregister
  MasterTransmit(traveldist); // Request front sensor
  storedValues[7] = SPDR; // SensorRFID
  MasterTransmit(front); // Request front sensor
  storedValues[5] = SPDR; // Distance
  
  if(invalue == right)
  {
    MasterTransmit(rightfront);
    storedValues[0] = SPDR; // Front
    MasterTransmit(rightback);
    storedValues[1] = SPDR; // Right front
    MasterTransmit(stop);
    storedValues[2] = SPDR; // Right back
  }
  else if(invalue == left)
  {
    MasterTransmit(leftfront);
    storedValues[0] = SPDR; // Front
    MasterTransmit(leftback);
    storedValues[3] = SPDR; // Left front
    MasterTransmit(stop);
    storedValues[4] = SPDR; // Left back
  }
  else if(invalue == turn)
  {
    MasterTransmit(gyro);
    storedValues[0] = SPDR; // Front
    MasterTransmit(stop);
    storedValues[6] = SPDR; // Gyro
  }
  else
  {
    MasterTransmit(stop);
    storedValues[0] = SPDR; // Front
  }
  
  PORTB ^= 0b00010000; // ss2 high
}


void TransmitComm(bool invalue)
{
  PORTB &= 0b11110111;
  
  if(invalue)
  {
    
  }
  else
  {
    for(int i = 0; i < 11;i++){
    MasterTransmit(storedValues[i]);
    }
  }
  
  PORTB ^= 0b00001000; 
  
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
    
  TransmitComm(remoteControl)
}

/***********************SLAVE komm*****************************/

typedef int bool;
enum {false, true};

//Lables for transmition
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

bool remoteControl = false; // Change to Port connected to switch

unsigned char storedValues[11];

int main(void)
{
  SlaveInit();
  if(remoteControl)
  {
    
  }
  else
  {
    while(0<<PORTB3)
    {
      for(int i = 0; i < 11;i++)
      {
        storedValues[i] = SlaveRecieve();
      }
    }
  }
}


/***********************SLAVE Sensor*****************************/

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

