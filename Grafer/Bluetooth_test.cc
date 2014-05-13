#include <bitset>
#include <iostream>

#include "Bluetooth.h"
//#include "Map.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <cmath>

using namespace std;

bool remote_controll(Bluetooth_Serial_comm* comm_port)
{
    bool remote = true;
    while(remote)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::K))
        {
            comm_port->Send_to_Bt('K');
            remote = false;
            // comm_port->~Bluetooth_Serial_comm();
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            comm_port->Send_to_Bt(1);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            comm_port->Send_to_Bt(2);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            comm_port->Send_to_Bt(3);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            comm_port->Send_to_Bt(4);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        {
            comm_port->Send_to_Bt(5);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        {
            comm_port->Send_to_Bt(6);
            //comm_port.~Bluetooth_Serial_comm();
        }
        else
        {
            comm_port->Send_to_Bt(0);
        }

    }
    return false;
}

bool autonom(Bluetooth_Serial_comm* comm_port)
{
    bool remote = false;
    int test;


    test = comm_port->Read_from_BT();
    float yt;


    //Initiering av variabler
    sf::RenderWindow* myWindow = new sf::RenderWindow(sf::VideoMode(800,600), "Who let the robot out?");
    myWindow->setFramerateLimit(60);


    sf::Clock myClock;
    sf::Event event;
    sf::Texture pixelTexture;
    pixelTexture.loadFromFile("pixel1.png");

    sf::Texture bgTexture;
    bgTexture.loadFromFile("background.png");
    sf::Sprite bgSprite(bgTexture);

    sf::Sprite newSprite(pixelTexture);
    //sf::Rect PixelRectangle(1,1,1,1);
    //sf::Sprite newSprite();

    sf::Font arial;
    arial.loadFromFile("arial.ttf");

    float x = 0;
    float y1 = 0;
    float y2 = 0;
    float y3 = 0;
    float y4 = 0;
    int windows_passed = 0;

    //myWindow->draw(bgSprite);

    while (myWindow->isOpen())
    {
        //Alla events
        while(myWindow->pollEvent(event))
        {
            switch(event.type)
            {
            case sf::Event::Closed:
            {
                myWindow->close();
                break;
            }
            case event.MouseButtonPressed:
            {
                int xpressed = event.mouseButton.x;
                int ypressed = event.mouseButton.y;
            }
            }
            if(test == 1)
            {
                yt = comm_port->Read_from_BT();
                if(yt != 0)
                {

                    y1 = ((1/yt) - 0.001086689563586)/0.000191822821525;

                }
                cout << "Fram: "<< y1 << " ";
            }
            else if(test == 2)
            {
                yt = comm_port->Read_from_BT();
                if(yt != 0)
                {
                    y2 = ((1/yt) - 0.000741938763948)/0.001637008132828;

                }
                cout << "Höger Fram: "<< y2 << " ";

            }
            else if(test == 3)
            {
                yt = comm_port->Read_from_BT();
                if(yt != 0)
                {
                    y3 = ((1/yt) - 0.000741938763948)/0.001637008132828;

                }
                cout << "Höger Bak: "<< y3 << " ";

            }
            else if(test == 4)
            {
                yt = comm_port->Read_from_BT();
                if(yt != 0)
                {
                    y4 = ((1/yt) - 0.000741938763948)/0.001637008132828;

                }
                //    cout << "Vänster Fram: "<< y4 << " ";
            }

            else if(test == 5)
            {
                test = comm_port->Read_from_BT();
                // cout << "Vänster Bak: "<< test << "\n";
            }
            else if(test == 6)
            {
                test = comm_port->Read_from_BT();
                // cout << "Avstånd: "<< test << " ";
            }
            else if(test == 7)
            {
                test = comm_port->Read_from_BT();
                cout << "Gyro: "<< test << "\n";
            }
            else if(test == 8)
            {
                test = comm_port->Read_from_BT();


                //  cout << "RFID: "<< test << "\n";

            }
            else if(test == 9)
            {
                test = comm_port->Read_from_BT();
                // cout << "Fram eller bakåt: "<< test << " ";
            }
            else if(test == 10)
            {
                test = comm_port->Read_from_BT();
                //  cout << "Styrsignal Höger: "<< test << " ";
            }
            else if(test == 11)
            {
                test = comm_port->Read_from_BT();
                //  cout << "Styrsignal Vänster: "<< test << "\n";
            }
            else
            {
                // cout << "test fel värde: " << test << "\n";
            }
        }

        x = round(myClock.getElapsedTime().asSeconds()*40);
        windows_passed = floor(x/370);


        newSprite.setPosition(x - 370*windows_passed + 20,290 - y1);
        myWindow->draw(newSprite);

        newSprite.setPosition(x - 370*windows_passed + 410,290 - y2);
        myWindow->draw(newSprite);

        newSprite.setPosition(x - 370*windows_passed + 20,580 - y3);
        myWindow->draw(newSprite);

        newSprite.setPosition(x - 370*windows_passed + 410,580 - y4);
        myWindow->draw(newSprite);

        //cout >> x >> " , " >> y >> endl;

        myWindow->display();
        if(fmod(x,370) <= 1)
        {
            myWindow->clear();
            myWindow->draw(bgSprite);
            newSprite.setColor(sf::Color::Yellow);
            std::cout << "comm_port" << std::endl;
        }
        // myWindow->clear();
    }



    return true;
}
int main()
{

    Bluetooth_Serial_comm comm_port;
    bool remote = true;

    if(remote == true)
    {
        remote = remote_controll(&comm_port);
    }
    else
    {
        remote = autonom(&comm_port);
    }



    comm_port.~Bluetooth_Serial_comm();

    return 0;
}


