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

void Drawmap(sf::RenderWindow* myWindow, int room[31][17],sf::RectangleShape* robotposition)//,sf::RectangleShape* fire, sf::RectangleShape* robotposition ,sf::RectangleShape* wall)
{
    /************ Kartritning ***********************/

//Rita ut ett tomt (vitt) fönster, pixlar eller?, i 29*15-storlek, med RenderWindow. Finns redan i nån main

//Rita ut roboten på kartan, i punkten (15,0) kommer den börja. Typ en fylld fyrkant eller nåt.
    for(int i = 0; i < 31; i++)
    {
        for(int j = 0; j < 17; j++)
        {

            if(room[i][j] == 2)
            {
                robotposition->setFillColor(sf::Color::Magenta);
                robotposition->setPosition(100 + 20*i , 590 -(20 + 20*j));
                myWindow->draw(*robotposition);
            }
            else if(room[i][j] == 5)
            {
                robotposition->setFillColor(sf::Color::White);
                robotposition->setPosition(100 + 20*i, 590 -(20 + 20*j)); //samma x och y som i setwall
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
                robotposition->setPosition(100 + 20*i, 590 -(20 + 20*j)); //samma x och y som i setwall
                myWindow->draw(*robotposition);
            }
            else
            {
                robotposition->setFillColor(sf::Color::Yellow);
                robotposition->setPosition(100 + 20*i, 590 -(20 + 20*j)); //samma x och y som i setwall
                myWindow->draw(*robotposition);
            }

        }

    }

//bör rita ut väggar på rätt plats när storleken är fixad
}


bool remote_controll(Bluetooth_Serial_comm* Com_Port)
{
    bool remote = true;
    int i = 0;
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
            cout << "w ";
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            Com_Port->Send_to_Bt(2);
            cout << "d ";
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            Com_Port->Send_to_Bt(3);
            cout << "a ";
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            Com_Port->Send_to_Bt(4);
            cout << "s ";
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        {

            Com_Port->Send_to_Bt(5);
            cout << "e ";


        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        {

            Com_Port->Send_to_Bt(6);
            cout << "q ";

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

    int room[31][17];


    /****************Kartritande variabler *********************/

    sf::RectangleShape robotposition;
    robotposition.setSize(sf::Vector2f(20, 20)); //fett osäker på storleken..
    robotposition.setOutlineColor(sf::Color::White);
    robotposition.setFillColor(sf::Color::Magenta);
    robotposition.setOutlineThickness(1); // och tjockleken.

    /*
    sf::RectangleShape wall;
    wall.setSize(sf::Vector2f(20, 20));
    wall.setOutlineColor(sf::Color::White);
    wall.setFillColor(sf::Color::Black);
    wallsetOutlineThickness(1);

    sf::RectangleShape* fire;
    fire->setSize(sf::Vector2f(20,20));
    fire->setOutlineColor(sf::Color::White);
    fire->setFillColor(sf::Color::Red);
    fire->setOutlineThickness(1);
    /*
    sf::RectangleShape* searched_area;
    searched_area->setSize(sf::Vector2f(20, 20));
    searched_area->setOutlineColor(sf::Color::Black);
    searched_area->setFillColor(sf::Color::White);
    searched_area->setOutlineThickness(1);
    */
    int myposX = 15;
    int myposY = 1;
    int x2;
    int y;
    int type = 2;

    bool isRFID = false;


    /***********************************************************/



    // while (myWindow->isOpen())


    while(remote == false and myWindow->isOpen())
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::L))
        {
            remote = true;
            myWindow->close();

        }
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
            //   cout << "Fram: "<< y1 << " ";
        }
        else if(test == 2)
        {
            yt = Com_Port->Read_from_BT();
            if(yt != 0)
            {
                y2 = ((1/yt) - 0.000741938763948)/0.001637008132828;

            }
            //  cout << "Höger Fram: "<< y2 << " ";

        }
        else if(test == 3)
        {
            yt = Com_Port->Read_from_BT();
            if(yt != 0)
            {
                y3 = ((1/yt) - 0.000741938763948)/0.001637008132828;

            }
            // cout << "Höger Bak: "<< y3 << "\n";

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
              cout << "firstzeroX: "<< test << " ";
        }
        else if(test == 7)
        {
            test = Com_Port->Read_from_BT();
             cout << "forstzeroY: "<< test << " ";
        }
        else if(test == 8)
        {
            test = Com_Port->Read_from_BT();


            // cout << "RFID: "<< test << " ";


        }
        else if(test == 9)
        {
            test = Com_Port->Read_from_BT();
            // cout << "x: "<< test << " ";
        }
        else if(test == 10)
        {
            test = Com_Port->Read_from_BT();
            //cout << "type "<< test << " ";
            type = test;

        }
        else if(test == 11)
        {
            test = Com_Port->Read_from_BT();
            //  cout << "styrvärde V: "<< test << "\n";
        }

        else if(test == 12)
        {
            test = Com_Port->Read_from_BT();
          //  myposX = test;
              cout << "myposX: "<< test << " ";
        }
        else if(test == 13)
        {
            test = Com_Port->Read_from_BT();
            cout << "myposY: "<< test << "\n";
          //  myposY = test;
        }
        else if(test == 14)
        {
            for(int i = 0; i < 31; i++)
            {
                for(int j = 0; j < 17; j++)
                {
                    test = Com_Port->Read_from_BT();
                    room[i][j] = test;

                }
            }
            cout << "karta kommit\n";
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

        /*****************Fram********************/
        cirkle.setPosition(755,395 - (y1*2));
        myWindow->draw(cirkle);
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
        /*
        if(isRFID == true)
        {

        room[myposX][myposY] = 4;
        }
        else if(room[myposX][myposY] == 4)
        {

        }
        else
        */

        room[myposX][myposY] = type;


        Drawmap(myWindow,room,&robotposition);//,fire,robotposition,wall);

        myWindow->display();
        if(type != 4)
        {
            room[myposX][myposY] = 5;
        }
        else
        {

        }
        if(fmod(x,370) <= 1)
        {
            //  myWindow->clear();
            //  myWindow->draw(bgSprite);
            // newSprite.setColor(sf::Color::Yellow);
            //    std::cout << "hej" << std::endl;
        }
    }



    return true;
}


/*************tester*****************/
/*
char room2[31][17];

void setwall(int x,int y)
{
	room2[x][y]=1;
}

void updatemap(char w,char mydirection,float sensorright,float sensorfront,float sensorleft,int myposX, int myposY) // Kan väl bara gälla för yttervarvet?
{
	//char w=30; //Hur långt ifrån vi ska vara för att säga att det är en vägg.

	switch(mydirection)
	{
		case (1): // X+
		if(sensorright<=w) //Vet inte vad som är en lämplig siffra här
		{
			setwall(myposX,myposY-1);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensorleft<w)
		{
			setwall(myposX,myposY+1);
		}
		else if((sensorfront>=45) && (sensorfront<=55))
		{
			setwall(myposX+2, myposY);
		}
		else if((sensorfront>=85) && (sensorfront<=95))
		{
			setwall(myposX+3, myposY);
		}
		else if((sensorfront>=125) && (sensorfront<=135))
		{
			setwall(myposX+4, myposY);
		}
		else if((sensorfront>=165) && (sensorfront<=175))
		{
			setwall(myposX+5, myposY);
		}

		if (!((room2[myposX-1][myposY] == 1) || (room2[myposX-1][myposY]== 4)))
		{
			room2[myposX-1][myposY]=2;
		}
		break;

		case (2): // Y+
		if(sensorright<=w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensorleft<w)
		{
			setwall(myposX-1,myposY);
		}
		else if((sensorfront>=45) && (sensorfront<=55))
		{
			setwall(myposX, myposY+2);
		}
		else if((sensorfront>=85) && (sensorfront<=95))
		{
			setwall(myposX, myposY+3);
		}
		else if((sensorfront>=125) && (sensorfront<=135))
		{
			setwall(myposX, myposY+4);
		}
		else if((sensorfront>=165) && (sensorfront<=175))
		{
			setwall(myposX, myposY+5);
		}

		if (!((room2[myposX][myposY-1] == 1) || (room2[myposX][myposY-1]== 4)))
		{
			room2[myposX][myposY-1]=2;
		}
		break;

		case (3): // X-
		if(sensorright<=w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensorleft<w)
		{
			setwall(myposX,myposY-1);
		}
		else if((sensorfront>=45) && (sensorfront<=55))
		{
			setwall(myposX-2, myposY);
		}
		else if((sensorfront>=85) && (sensorfront<=95))
		{
			setwall(myposX-3, myposY);
		}
		else if((sensorfront>=125) && (sensorfront<=135))
		{
			setwall(myposX-4, myposY);
		}
		else if((sensorfront>=165) && (sensorfront<=175))
		{
			setwall(myposX-5, myposY);
		}

		if (!((room2[myposX+1][myposY]) || (room2[myposX + 1][myposY] == 4)))
		{
			room2[myposX+1][myposY]=2;
		}
		break;

		case (4): // Y-
		if(sensorright<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY-1);
		}
		else if(sensorleft<w)
		{
			setwall(myposX+1,myposY);
		}
		else if((sensorfront>=45) && (sensorfront<=55))
		{
			setwall(myposX, myposY-2);
		}
		else if((sensorfront>=85) && (sensorfront<=95))
		{
			setwall(myposX, myposY-3);
		}
		else if((sensorfront>=125) && (sensorfront<=135))
		{
			setwall(myposX, myposY-4);
		}
		else if((sensorfront>=165) && (sensorfront<=175))
		{
			setwall(myposX, myposY-5);
		}

		if (!((room2[myposX][myposY+1] == 1) || (room2[myposX][myposY+1] == 4)))
		{
			room2[myposX][myposY+1]=2;
		}
		break;
	}
/*	if(storedValues[7]==1)
	{
		room2[myposX][myposY]=4;
	}*/
//}

/*********************************/
int main()
{

    /***********tester****************/
    /*
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

        sf::RectangleShape robotposition;
    robotposition.setSize(sf::Vector2f(20, 20)); //fett osäker på storleken..
    robotposition.setOutlineColor(sf::Color::White);
    robotposition.setFillColor(sf::Color::Magenta);
    robotposition.setOutlineThickness(1); // och tjockleken.

    float frontsensor = 150;
            float rightfront = 10;
            float leftfront = 40;
            char mydirection = 1;
            int myposX = 16;
            int myposY = 1;
    int i = 0;
        while(myWindow->isOpen())
        {
            if(i< 25)
            {
                i++;
            }
            if(i <= 5)
            {
                myposX++;
            }

            if((i > 5) and (i <= 10))
            {
                mydirection = 2;
                myposY++;
            }
            if((i > 10) and (i <= 15))
            {
                mydirection = 3;
                myposX--;
            }
            if((i > 15) and (i <= 20))
            {
                mydirection = 4;
                myposY--;
            }



    updatemap(30,mydirection,rightfront,frontsensor,leftfront,myposX,myposY);


    Drawmap(myWindow,room2,&robotposition);
    myWindow->display();
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
        }

    /**********************************/




    Bluetooth_Serial_comm Com_Port;
    bool remote = false;


    if(remote == true)
    {
        remote = remote_controll(&Com_Port);
    }
    else
    {
        remote = autonom(&Com_Port);
    }



    Com_Port.disconnect();

    return 0;
}
