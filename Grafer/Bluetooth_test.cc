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

void Drawmap(sf::RenderWindow* myWindow, char room[29][15],sf::RectangleShape* fire, sf::RectangleShape* robotposition ,sf::RectangleShape* wall)
{
    /************ Kartritning ***********************/

//Rita ut ett tomt (vitt) fönster, pixlar eller?, i 29*15-storlek, med RenderWindow. Finns redan i nån main

//Rita ut roboten på kartan, i punkten (15,0) kommer den börja. Typ en fylld fyrkant eller nåt.
    for(int i = 0; i < 29; i++)
    {
        for(int j = 0; j < 15; j++)
        {

            if(room[i][j] == 2) //Det här är inte rätt. vi borde bara ha typ ...setPosition(myposX,myposY) eller nåt.
            //2 är ju avsökt område
            {
                robotposition->setPosition(120 + 20*i , 590 -(20 + 20*j));
                myWindow->draw(*robotposition);
            }
            else if(room[i][j] == 1)
            {
                wall->setPosition(120 + 20*i, 590 -(20 + 20*j)); //samma x och y som i setwall
                myWindow->draw(*wall);
            }
            else if(room[i][j] == 4)
            {
                fire->setPosition(120 + 20*i, 590 -(20 + 20*j));
                myWindow->draw(*fire);
            }
            else if(room[i][j] == 2)
            {
                searched_area.setPosition(120 + 20*i, 590 -(20 + 20*j));
                window->draw(*searched_area);
            }
            else
            {
                wall->setPosition(120 + 20*i, 590 -(20 + 20*j)); //samma x och y som i setwall
                myWindow->draw(*wall);
            }
            
        }

    }

//bör rita ut väggar på rätt plats när storleken är fixad
}


bool remote_controll(Bluetooth_Serial_comm* Com_Port)
{
    bool remote = true;
    while(remote)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::K))
        {
            Com_Port->Send_to_Bt('K');
            remote = false;
            // Com_Port->~Bluetooth_Serial_comm();
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
            //Com_Port.~Bluetooth_Serial_comm();
        }
        else
        {
            Com_Port->Send_to_Bt(0);
        }

    }
    return false;
}

bool autonom(Bluetooth_Serial_comm* Com_Port)
{
    bool remote = false;
    int test;

    float yt;


    //Initiering av variabler
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

    char room[29][15]={{1,0,0,0,1,2,2,1,1,1,1},
                    {1,2,1,1,2,4}};

/****************Kartritande variabler *********************/

sf::RectangleShape robotposition;
robotposition.setSize(sf::Vector2f(20, 20)); //fett osäker på storleken..
robotposition.setOutlineColor(sf::Color::White);
robotposition.setFillColor(sf::Color::Magenta);
robotposition.setOutlineThickness(1); // och tjockleken.

sf::RectangleShape wall;
wall.setSize(sf::Vector2f(20, 20));
wall.setOutlineColor(sf::Color::White);
wall.setFillColor(sf::Color::Black);
wall.setOutlineThickness(1);

sf::RectangleShape fire;
fire.setSize(sf::Vector2f(20,20));
fire.setOutlineColor(sf::Color::White);
fire.setFillColor(sf::Color::Red);
fire.setOutlineThickness(1);

sf::RectangleShape searched_area;
searched_area.setSize(sf::Vector2f(20, 20));
searched_area.setOutlineColor(sf::Color::White);
searched_area.setFillColor(sf::Color::White);
searched_area.setOutlineThickness(1);

int myposX;
int myposY = 1;
int x2;
int y;

bool isRFID = false;


/***********************************************************/



   // while (myWindow->isOpen())
   while(remote == false)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::L))
        {
            remote = true;
   //         myWindow->close();
        }

        test = Com_Port->Read_from_BT();

         if(test == 1)
            {
                test = Com_Port->Read_from_BT();
                yt= test;
                if(yt != 0)
                {

                    y1 = ((1/yt) - 0.001086689563586)/0.000191822821525;

                }
                cout << "Fram: "<< y1 << " ";
            }
            else if(test == 2)
            {
                yt = Com_Port->Read_from_BT();
                if(yt != 0)
                {
                    y2 = ((1/yt) - 0.000741938763948)/0.001637008132828;

                }
                cout << "Höger Fram: "<< y2 << " ";

            }
            else if(test == 3)
            {
                yt = Com_Port->Read_from_BT();
                if(yt != 0)
                {
                    y3 = ((1/yt) - 0.000741938763948)/0.001637008132828;

                }
                cout << "Höger Bak: "<< y3 << "\n";

            }
            else if(test == 4)
            {
                yt = Com_Port->Read_from_BT();
                if(yt != 0)
                {
                    y4 = ((1/yt) - 0.000741938763948)/0.001637008132828;

                }
               //     cout << "Vänster Fram: "<< y4 << "\n";
            }

            else if(test == 5)
            {
                yt = Com_Port->Read_from_BT();
                if(yt != 0)
                {
                    y5 = ((1/yt) - 0.000741938763948)/0.001637008132828;

                }
                // cout << "Vänster Bak: "<< test << "\n";
            }
            else if(test == 6)
            {
                test = Com_Port->Read_from_BT();
                // cout << "Avstånd: "<< test << " ";
            }
            else if(test == 7)
            {
                test = Com_Port->Read_from_BT();
              //  cout << "Gyro: "<< test << "\n";
            }
            else if(test == 8)
            {
                test = Com_Port->Read_from_BT();


                //  cout << "RFID: "<< test << "\n";

            }
            else if(test == 9)
            {
                test = Com_Port->Read_from_BT();
                // cout << "Fram eller bakåt: "<< test << " ";
            }
            else if(test == 10)
            {
                test = Com_Port->Read_from_BT();
                //  cout << "Styrsignal Höger: "<< test << " ";
            }
            else if(test == 11)
            {
                test = Com_Port->Read_from_BT();
                //  cout << "Styrsignal Vänster: "<< test << "\n";
            }
            else
            {
                // cout << "test fel värde: " << test << "\n";
            }


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

        }
        x = round(myClock.getElapsedTime().asSeconds()*40);
        windows_passed = floor(x/370);


/********************************************************/

myWindow->clear();

            myWindow->draw(bgSprite);




        /************* Höger Fram****************/
        cirkle.setPosition(155,185 - (y2* 6));
        myWindow->draw(cirkle);
        /************* Höger Bak****************/
        cirkle.setPosition(267,185 - (y3* 6));
        myWindow->draw(cirkle);
        /************* Vänster Fram****************/
        cirkle.setPosition(385,185 - (y4* 6));
        myWindow->draw(cirkle);
        /************* Vänster Bak****************/
        cirkle.setPosition(497,185 - (y5* 6));
        myWindow->draw(cirkle);




/*************************************************/

Drawmap(myWindow,room,&fire,&robotposition,&wall);


        myWindow->display();

        if(fmod(x,370) <= 1)
        {
          //  myWindow->clear();
          //  myWindow->draw(bgSprite);
            newSprite.setColor(sf::Color::Yellow);
        //    std::cout << "hej" << std::endl;
        }
    }



    return true;
}



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

    Com_Port.~Bluetooth_Serial_comm();

    return 0;
}
