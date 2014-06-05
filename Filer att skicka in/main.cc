/*
* FILNAMN: main.cc
* PROGRAMMERARE: Grupp 9 - WLRO
* DATUM: 2014-06-04
*
* mainfil för det kartritande programmet och för att styra roboten autonomt
*
*
*/

#include <bitset>
#include <iostream>

#include "Bluetooth.h"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <cmath>


using namespace std;


/************ Kartritning ***********************/
void Drawmap(sf::RenderWindow* myWindow, int room[31][17],sf::RectangleShape* robotposition)
{
    for(int i = 0; i < 31; i++)
    {
        for(int j = 0; j < 17; j++)
        {

            if(room[i][j] == 3)
            {
                robotposition->setFillColor(sf::Color::Magenta);
                robotposition->setPosition(100 + 20*i , 590 -(20 + 20*j));
                myWindow->draw(*robotposition);
            }
            else if(room[i][j] == 2)
            {
                robotposition->setFillColor(sf::Color::White);
                robotposition->setPosition(100 + 20*i, 590 -(20 + 20*j));
                myWindow->draw(*robotposition);
            }
            else if(room[i][j] == 4)
            {
                robotposition->setFillColor(sf::Color::Red);
                robotposition->setPosition(100 + 20*i, 590 -(20 + 20*j));
                myWindow->draw(*robotposition);
            }
            else if(room[i][j] == 1)
            {
                robotposition->setFillColor(sf::Color::Black);
                robotposition->setPosition(100 + 20*i, 590 -(20 + 20*j));
                myWindow->draw(*robotposition);
            }
            else
            {
                robotposition->setFillColor(sf::Color::Yellow);
                robotposition->setPosition(100 + 20*i, 590 -(20 + 20*j));
                myWindow->draw(*robotposition);
            }
        }
    }
}

/********Fjärrstyrt läge *****************/
bool remote_controll(Bluetooth_Serial_comm* Com_Port)
{
    bool remote = true;
    while(remote)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::K))
        {
            remote = false;
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            Com_Port->Send_to_Bt(1);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            Com_Port->Send_to_Bt(2);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            Com_Port->Send_to_Bt(3);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            Com_Port->Send_to_Bt(4);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        {
            Com_Port->Send_to_Bt(5);

        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        {
            Com_Port->Send_to_Bt(6);
        }
        else
        {
            Com_Port->Send_to_Bt(0);
        }
    }
    return false;
}
/*************Autonomt läge*****************/
bool autonom(Bluetooth_Serial_comm* Com_Port)
{


    //Initiering av variabler
    bool remote = false;
    int test;
    int direction;
    float yt;

    sf::RenderWindow* myWindow = new sf::RenderWindow(sf::VideoMode(800,600), "Who let the robot out?");
    myWindow->setFramerateLimit(60);

    sf::CircleShape cirkle;
    cirkle.setRadius(6);
    cirkle.setFillColor(sf::Color::Red);


    sf::Clock myClock;
    sf::Event event;
    sf::Texture pixelTexture;
    pixelTexture.loadFromFile("pixel1.png");

    sf::Texture bgTexture;
    bgTexture.loadFromFile("background.png");
    sf::Sprite bgSprite(bgTexture);
    sf::Text Numbers_To_Draw;

    sf::Sprite newSprite(pixelTexture);
    sf::Sprite y1Sprite(int num);


    sf::Font arial;
    arial.loadFromFile("arial.ttf");

    float x = 0;
    float y1 = 0;
    float y2 = 0;
    float y3 = 0;
    float y4 = 0;
    float y5 = 0;
    int windows_passed = 0;
    int delayi = 0;
    int rfidx = 42;
    int rfidy = 42;
    int room[31][17]; //Kartan


    /****************Kartritande variabler *********************/

    sf::RectangleShape robotposition;
    robotposition.setSize(sf::Vector2f(20, 20));
    robotposition.setOutlineColor(sf::Color::White);
    robotposition.setFillColor(sf::Color::Magenta);
    robotposition.setOutlineThickness(1);

    int myposX = 15;
    int myposY = 1;
    int myposXtemp = myposX;
    int myposYtemp = myposY;
    int x2;
    int y;
    int type = 2;

    bool isRFID = true;


    /***********************************************************/


    while(myWindow->isOpen())
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::K))
        {
            myWindow->close();
        }
        //Resetknapp
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        {
            for(int i = 0; i < 31; i++)
            {
                for(int j = 0; j < 17; j++)
                {
                    room[i][j] = 0;
                }
            }
            myposX = 15;
            myposY = 1;
            rfidx = 42;
            rfidy = 42;
            isRFID = true;
        }
/************************ Hämta värden från Serieporten ***************/
        /*Här hämtas värden från roboten beroende vilken label de har*/

        test = Com_Port->Read_from_BT();
        //Vänster
        if(test == 1)
        {
            test = Com_Port->Read_from_BT();
            yt= test;
            if(yt != 0)
            {

                y1 = ((1/yt) - 0.001086689563586)/0.000191822821525;

            }
        }
        //Höger fram
        else if(test == 2)
        {
            yt = Com_Port->Read_from_BT();
            if(yt != 0)
            {
                y2 = ((1/yt) - 0.000741938763948)/0.001637008132828;

            }

        }
        //Höger bak
        else if(test == 3)
        {
            yt = Com_Port->Read_from_BT();
            if(yt != 0)
            {
                y3 = ((1/yt) - 0.000741938763948)/0.001637008132828;

            }
        }
        //Fram
        else if(test == 4)
        {
            yt = Com_Port->Read_from_BT();
            if(yt != 0)
            {
                y4 = ((1/yt) - 0.000741938763948)/0.001637008132828;

            }
        }
        //ledig ruta för att skicka värden som roboten kan testa , här låg förut vänster sensor bak
        else if(test == 5)
        {
            yt = Com_Port->Read_from_BT();
            if(yt != 0)
            {
                y5 = ((1/yt) - 0.000741938763948)/0.001637008132828;
            }
        }
        //Färdad distans
        else if(test == 6)
        {
            test = Com_Port->Read_from_BT();
            //   cout << "distance: "<< test << " ";
        }
        //Gyrovärde
        else if(test == 7)
        {
            test = Com_Port->Read_from_BT();
        }
        //Rfid detekterad för tester
        else if(test == 8)
        {
            test = Com_Port->Read_from_BT();

        }
        //Rikting roboten färdas på kartan
        else if(test == 9)
        {
            test = Com_Port->Read_from_BT();
            direction = test;
        }
        //typ av ruta som roboten befinner sig i (vägg, vanliug eller RFID) för tester
        else if(test == 10)
        {
            test = Com_Port->Read_from_BT();

        }
        //styrvärde för tester
        else if(test == 11)
        {
            test = Com_Port->Read_from_BT();
        }
        // X position för roboten
        else if(test == 12)
        {
            test = Com_Port->Read_from_BT();
            myposX = test;
        }
        //Y position för roboten
        else if(test == 13)
        {
            test = Com_Port->Read_from_BT();
            myposY = test;
        }
        //Rfid's X och Y position
        else if(test == 14)
        {
            rfidx = Com_Port->Read_from_BT();
        }
        else if(test == 15)
        {
            rfidy = Com_Port->Read_from_BT();

        }
        //Hela kartan skickas från roboten
        else if (test == 16)
        {
            for(int i = 0; i < 31; i++)
            {
                for(int j = 0; j < 17; j++)
                {
                    test = Com_Port->Read_from_BT();
                    room[i][j] = test;

                }
            }
        }
/*******************************************************************/

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

        }


        /***********************Rita på myWindow*********************************/

        myWindow->clear();

        myWindow->draw(bgSprite);
        /*****************Vänster Fram********************/
        cirkle.setPosition(755,395 - y1);
        myWindow->draw(cirkle);
        /************* Höger Fram****************/
        cirkle.setPosition(155,185 - (y2* 6));
        myWindow->draw(cirkle);
        /************* Höger Bak****************/
        cirkle.setPosition(267,185 - (y3* 6));
        myWindow->draw(cirkle);
        /************* Fram****************/
        cirkle.setPosition(385,185 - (y4* 6));
        myWindow->draw(cirkle);


        /**********************kartritning************************/
        if(isRFID == true)
        {
            if((rfidx != 42) and (rfidy != 42))
            {
                room[rfidx][rfidy] = 4;
                isRFID = false;
            }
        }

        if((room[myposX][myposY] != 4) and (room[myposX][myposY] != 1))
        {
            room[myposX][myposY] = 3;
        }


        Drawmap(myWindow,room,&robotposition);


        /*************************************************************/
        myWindow->display();


        if(room[myposX][myposY] == 3)
        {
            room[myposX][myposY] = 2;
        }

    }



    return true;
}
/*********************************/
int main()
{

    Bluetooth_Serial_comm Com_Port;
    bool remote = false;
    while(1)
    {


        if(remote == true)
        {
            remote = remote_controll(&Com_Port);
        }
        else
        {
            remote = autonom(&Com_Port);
        }

    }

    Com_Port.disconnect();

    return 0;
}
