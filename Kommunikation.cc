#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>
#include <bitset>
#include <SFML/System.hpp>

//#include <SFML/Network.hpp>

using namespace std;
HANDLE Settup_BT()
{


/***************************Hanle serial 2 st för com 8 (ut) och com 9 (in) *****/
    HANDLE hSerial;
    hSerial = CreateFile("COM8",
                            GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
                         NULL);
    if(hSerial==INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND)
        {
//serial port does not exist. Inform user.
        cout << "No serial port\n";
        }
//some other error occurred. Inform user.
        cout << "other error occured\n";
    }
/****************************** Timeouts **************************/
COMMTIMEOUTS timeouts={0};
timeouts.ReadIntervalTimeout=50;
timeouts.ReadTotalTimeoutConstant=50;
timeouts.ReadTotalTimeoutMultiplier=10;
timeouts.WriteTotalTimeoutConstant=50;
timeouts.WriteTotalTimeoutMultiplier=50;

/******************* Sätter timeouts för hSerial **************************/

if(!SetCommTimeouts(hSerial,&timeouts))
{
    cout << "SetCommTimeout error!!\n";
}
/**************** OVERLAPPED *****************************/ //denna del behövs ej!
/*OVERLAPPED osWrite;
osWrite = {0};
osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
osWrite.Offset = 0xFFFFFFFF;
osWrite.OffsetHigh = 0xFFFFFFFF;


/********************* DCB ****************************/
DCB dcb = {0};
dcb.DCBlength = sizeof(dcb);
dcb.BaudRate = 115200;     // set the baud rate
dcb.ByteSize = 8;             // data size, xmit, and rcv
dcb.Parity = NOPARITY;        // no parity bit
dcb.StopBits = ONESTOPBIT;    // one stop bit
dcb.fBinary = true;

   if(!(SetCommState(hSerial, &dcb)))
   {
       cout << "SetCommState error !! \n";
   }
 //DWORD dwBytesWrite=0;


/***************************Hanle seria2l 2 st för com 9 (in) och com 9 (in) *****/

    HANDLE hSerial2; //seriiell inport
    hSerial2 = CreateFile("COM9",
                            GENERIC_READ,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
                         NULL);
    if(hSerial2==INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND)
        {
//serial port does not exist. Inform user.
        cout << "No serial port\n";
        }
//some other error occurred. Inform user.
        cout << "other error occured\n";
    }

/******************* Settup för hSerial **************************/

if(!SetCommTimeouts(hSerial2,&timeouts))
{
    cout << "SetCommTimeout error!!\n";
}
/**************** OVERLAPPED *****************************/

OVERLAPPED osRead = {0};
//osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

LPOVERLAPPED_COMPLETION_ROUTINE ReadCompletion; // denna Overlapped kommer antagligen behövas för ReadFileEx

/*********************************************************/
   if(!(SetCommState(hSerial2, &dcb)))
   {
       cout << "SetCommState error !! \n";
   }

//DWORD dwBytesRead=0;
   if (INVALID_SET_FILE_POINTER != SetFilePointer(hSerial,
                                                           0,
                                                           0,
                                                           FILE_BEGIN));


return hSerial;

}


/****************** Writefile *************************************/
void Send_to_Bt(unsigned int t, HANDLE hSerial , OVERLAPPED osWrite)
{


unsigned char szBuff[2] = {t};
   cout << "szBuff innan: " << szBuff << "\n";
    cout << "szBuff[1] innan: " << szBuff[1] << " szBuff[2] innan: " << szBuff[2] << "\n";
    if(!WriteFileEx(hSerial,szBuff,1,&osWrite,NULL))
    {
        cout << "Writefile error \n";
    }
    else
    {
    }
    cout <<"szBuff[1] efter:  " << szBuff[1] <<"szBuff[2] efter :"<< szBuff[2] << "\n";
/*****************************************************************/

FlushFileBuffers(hSerial);
}
//FlushFileBuffers(hSerial2);


/************************' Readfile **********************************/
  /*  unsigned char szBuff2[2] = {0};
    if(!ReadFileEx(hSerial2,szBuff2,1,&osRead,ReadCompletion))
    {
        cout << "READFILE error \n";
    }
    else
    {

    }
cout << "szBuff2 efter: " << szBuff2 << "\n";
 cout << "szBuff2[1] : " << szBuff2[1] << "  szBuff2[2] : " << szBuff2[2] <<  "\n";

/*****************************************************************/
/****** Stäng serieportar ****************/






int main()
{

   HANDLE hSerial;
   OVERLAPPED osWrite = {0};
    hSerial = Settup_BT();

while(1)
{

if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
       Send_to_Bt(1,hSerial,osWrite);
    }
else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
       Send_to_Bt(2,hSerial,osWrite);
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
       Send_to_Bt(3,hSerial,osWrite);
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        Send_to_Bt(4,hSerial,osWrite);
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
    {
        Send_to_Bt(5,hSerial,osWrite);
    }
    else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
    {
        Send_to_Bt(6,hSerial,osWrite);
    }
    else
    {
      // Send_to_Bt(0,hSerial,osWrite);
    }
}
CloseHandle(hSerial);

return 0;

}
