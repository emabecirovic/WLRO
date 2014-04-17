#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>
#include <SFML/System.hpp>


class Bluetooth_Serial_comm{

private:
    HANDLE hSerial;
    OVERLAPPED osReadWrite = {0};
    HANDLE Settup_BT();

public:

Bluetooth_Serial_comm();
~Bluetooth_Serial_comm();


unsigned char Read_from_BT();
void Send_to_Bt(unsigned int);
//hSerial_send = Settup_Send_BT();

};
