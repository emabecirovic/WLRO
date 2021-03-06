#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>
#include <SFML/System.hpp>
#include "Bluetooth.h"


using namespace std;

Bluetooth_Serial_comm::Bluetooth_Serial_comm()
{
    hSerial = Settup_BT();
    osReadWrite.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

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
                         FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
                         NULL);
    if(hSerial==INVALID_HANDLE_VALUE)
    {
        if(GetLastError()==ERROR_FILE_NOT_FOUND)
        {
            cout << "No serial port\n";
        }
        cout << "other error occured hSerial Write\n";
    }


    /****************************** Timeouts **************************/
    COMMTIMEOUTS timeouts= {0};
    timeouts.ReadIntervalTimeout=50;
    timeouts.ReadTotalTimeoutConstant=50;
    timeouts.ReadTotalTimeoutMultiplier=10;
    timeouts.WriteTotalTimeoutConstant=50;
    timeouts.WriteTotalTimeoutMultiplier=50;

    /******************* Settup för hSerial **************************/

    if(!SetCommTimeouts(hSerial,&timeouts))
    {
        cout << "SetCommTimeout error!!\n";
    }
    /********************* DCB ****************************/
    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = CBR_115200;     // set the baud rate
    dcb.ByteSize = 8;             // data size, xmit, and rcv
    dcb.Parity = NOPARITY;        // no parity bit
    dcb.StopBits = ONESTOPBIT;    // one stop bit
    dcb.fBinary = true;

    if(!(SetCommState(hSerial, &dcb)))
    {
        cout << "SetCommState error !! \n";
    }


    if (INVALID_SET_FILE_POINTER != SetFilePointer(hSerial,
            0,
            0,
            FILE_BEGIN));



    return hSerial;
}


/****************** Writefile *************************************/
void Bluetooth_Serial_comm::Send_to_Bt(unsigned int t)
{
    bool fWaitingOnRead = false;
    DWORD dwByte;
    unsigned char szBuff[2] = {t};
    if(!fWaitingOnRead)
    {
        if(!WriteFile(hSerial,szBuff,1,&dwByte,&osReadWrite))
        {
            if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
                // Error in communications; report it.
            {
                cout << "error in communication \n";
            }
            else
            {
                fWaitingOnRead = true;
                //cout << "io pendeling\n";
            }
        }
    }
    FlushFileBuffers(hSerial);
}


/************************' Readfile **********************************/
unsigned char Bluetooth_Serial_comm::Read_from_BT()
{
    //ReadCompletion
    BOOL fWaitingOnRead = false;
    DWORD dwByte;
    unsigned char szBuff2[2];
    if(!fWaitingOnRead)
    {
        if(!ReadFile(hSerial,szBuff2,1,&dwByte,&osReadWrite))
        {
            if (GetLastError() != ERROR_IO_PENDING)     // read not delayed?
                // Error in communications; report it.
            {
                cout << "error in communication \n";
            }

            else
            {
                fWaitingOnRead = true;
                //cout << "io pendeling\n";

            }
        }
        else
        {
            // cout << "read complete imidietly!! \n";
        }
    }

    /*************************************TESTER************************************************/



#define READ_TIMEOUT      500      // milliseconds
DWORD dwRes;

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
            } // Error in communications; report it.
            else
            {
                // cout << "read complete success\n";
                // cout << dwRes << "\n";
                // Read completed successfully.
                //  Reset flag so that another opertion can be issued.
                fWaitingOnRead = FALSE;
            }
            break;

        case WAIT_TIMEOUT:
            // Operation isn't complete yet. fWaitingOnRead flag isn't
            // changed since I'll loop back around, and I don't want
            // to issue another read until the first one finishes.
            //
            // This is a good time to do some background work.
            cout << "operation not complete Wait_TIMEOUT\n";
            break;

        default:
            // Error in the WaitForSingleObject; abort.
            // This indicates a problem with the OVERLAPPED structure's
            // event handle.
            cout << "Error in wait for single object \n";
            break;
        }
    }
    /************************************************************************************************/
    //FlushFileBuffers(hSerial);
    return szBuff2[0];
}
/*****************************************************************/
