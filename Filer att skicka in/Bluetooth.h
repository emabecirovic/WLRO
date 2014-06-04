/*
* FILNAMN: Bluetooth.h
* PROGRAMMERARE: Grupp 9 - WLRO
* DATUM: 2014-06-04
*
* .h-fil till klassen Bluetooth
*
*
*/

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>
#include <SFML/System.hpp>

class Bluetooth_Serial_comm{

private:
    HANDLE hSerial;
    OVERLAPPED osReadWrite = {0};
    HANDLE Settup_BT();
    bool connected;
    unsigned char szBuff2[2];
    unsigned char szBuff[2];
    DWORD dwByte;
    bool fWaitingOnRead;
    DWORD dwRes;
public:

Bluetooth_Serial_comm();
~Bluetooth_Serial_comm();

bool IsOpen();

unsigned char Read_from_BT();
void Send_to_Bt(int);
void disconnect();

};

#endif // GRAF_H
