/*
* FILNAMN: Bluetooth.cc
* PROGRAMMERARE: Grupp 9 - WLRO
* DATUM: 2014-06-04
*
* .cc-fil för klassen Bluetooth som sköter kommunikationen med bluetooth via
* en serriell COM port.
*
*
*/


#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>
#include <SFML/System.hpp>
#include "Bluetooth.h"
#include <bitset>


using namespace std;

Bluetooth_Serial_comm::Bluetooth_Serial_comm()
{
    hSerial = Settup_BT();
    osReadWrite.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    osReadWrite.Offset = 0;

}

Bluetooth_Serial_comm::~Bluetooth_Serial_comm()
{
    CloseHandle(hSerial);
}


HANDLE Bluetooth_Serial_comm::Settup_BT()
{


    /***************************Hanle serial för com 8 *********************/
    HANDLE hSerial;
    hSerial = CreateFile("COM8",
                         GENERIC_WRITE|GENERIC_READ,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,
                         NULL);
    if(hSerial==INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND)
        {
            cout << "No serial port\n";
            connected = false;
        }
        cout << "other error occured in creating hSerial \n";
        connected = false;
    }


    /****************************** Timeouts **************************/
    COMMTIMEOUTS timeouts= {0};
    timeouts.ReadIntervalTimeout=10;
    timeouts.ReadTotalTimeoutConstant=10;
    timeouts.ReadTotalTimeoutMultiplier=50;
    timeouts.WriteTotalTimeoutConstant=50;
    timeouts.WriteTotalTimeoutMultiplier=100;

    /******************* Settup för hSerial **************************/

    if(!SetCommTimeouts(hSerial,&timeouts))
    {
        cout << "SetCommTimeout error!!\n";
        connected = false;
    }
    /********************* DCB ****************************/
    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = CBR_115200;     // set the baud rate
    dcb.ByteSize = 8;             // data size
    dcb.Parity = NOPARITY;        // no parity
    dcb.StopBits = ONESTOPBIT;    // one stop bit

    if(!(SetCommState(hSerial, &dcb)))
    {
        cout << "SetCommState error !! \n";
        connected = false;
    }


     (INVALID_SET_FILE_POINTER != SetFilePointer(hSerial,
            0,
            0,
            FILE_BEGIN));



    return hSerial;
}


/****************** Writefile *************************************/
void Bluetooth_Serial_comm::Send_to_Bt(int t)
{
    fWaitingOnRead = false;
 FlushFileBuffers(hSerial);
 szBuff[0] = t;

    if(!fWaitingOnRead)
    {
        if(!WriteFile(hSerial,szBuff,1,&dwByte,&osReadWrite))
        {
            if (GetLastError() != ERROR_IO_PENDING)     // read delayed?
                // Error in communications
            {
                cout << "error in communication write \n";
                cout << GetLastError() << "\n";
            }
            else
            {
                fWaitingOnRead = true;

            }
        }
    }

}


/************************' Readfile **********************************/
unsigned char Bluetooth_Serial_comm::Read_from_BT()
{
    FlushFileBuffers(hSerial);

    fWaitingOnRead = false;




if(!fWaitingOnRead)
    {
        if(!ReadFile(hSerial,szBuff2,1,&dwByte,&osReadWrite))
        {
            if (GetLastError() != ERROR_IO_PENDING)     // read delayed?
                // Error in communications
            {
               cout << "error in communication read \n";

            }

            else
            {
                fWaitingOnRead = true;


            }
        }

    }
    /*************************************TESTER************************************************/



#define READ_TIMEOUT      500


    if (fWaitingOnRead)
    {
        dwRes = WaitForSingleObject(osReadWrite.hEvent, READ_TIMEOUT);

        switch(dwRes)
        {
            // Read completed.
        case WAIT_OBJECT_0:
            if (!GetOverlappedResult(hSerial, &osReadWrite, &dwByte, FALSE))
            {
                cout << "error in communication \n";
            }
            else
            {

                // Read completed successfully.
                fWaitingOnRead = FALSE;

            }
            break;

        case WAIT_TIMEOUT:

            cout << "operation not complete Wait_TIMEOUT\n";
            break;

        default:

            cout << "Error in wait for single object \n";
            break;
        }
    }

    return szBuff2[0];
}

/*****************************************************************/

bool Bluetooth_Serial_comm::IsOpen()
{

        return connected;
}

void Bluetooth_Serial_comm::disconnect()
{
    CloseHandle(hSerial);
}

