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

public:

Bluetooth_Serial_comm();
~Bluetooth_Serial_comm();

bool IsOpen();

unsigned char Read_from_BT();
void Send_to_Bt(unsigned int);

};

#endif // GRAF_H
