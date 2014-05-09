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

using namespace std;

void remote_controll(Bluetooth_Serial_comm HEJ)
{
    bool remote = true;
    while(remote)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::K))
        {
            HEJ.Send_to_Bt('K');
            remote = false;
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            HEJ.Send_to_Bt(1);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            HEJ.Send_to_Bt(2);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            HEJ.Send_to_Bt(3);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            HEJ.Send_to_Bt(4);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
        {
            HEJ.Send_to_Bt(5);
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
        {
            HEJ.Send_to_Bt(6);
        }
        else
        {
            HEJ.Send_to_Bt(0);
        }
    }
}
int main()
{
   unsigned int test;
    Bluetooth_Serial_comm HEJ;
    bool remote = true;






        //Initiering av variabler
  /*  sf::RenderWindow* myWindow = new sf::RenderWindow(sf::VideoMode(800,800), "Who let the robot out?");
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
    arial.loadFromFile("arial.ttf");*/

 //   float x = 0;
    float y1 = 0;
    float y2 = 0;
    float y3 = 0;
    float y4 = 0;
 //   int windows_passed = 0;


    //unsigned char test = {0};

/*
    while(1)
    {
            test = HEJ.Read_from_BT();

            if(test == 1)
            {
                test = HEJ.Read_from_BT();
                cout << "Fram: "<< test << "\n";
            }
            else if(test == 2)
            {
                test = HEJ.Read_from_BT();
                cout << "Höger Fram: "<< test << "\n";
            }
            else if(test == 3)
            {
                test = HEJ.Read_from_BT();
                cout << "Höger Bak: "<< test << "\n";
            }
            else if(test == 4)
            {
                test = HEJ.Read_from_BT();
                cout << "Vänster Fram: "<< test << "\n";
            }
            else if(test == 5)
            {
                test = HEJ.Read_from_BT();
                cout << "Vänster Bak: "<< test << "\n";
            }
            else if(test == 6)
            {
                test = HEJ.Read_from_BT();
                cout << "Avstånd: "<< test << "\n";
            }
            else if(test == 7)
            {
                test = HEJ.Read_from_BT();
                cout << "Gyro: "<< test << "\n";
            }
            else if(test == 8)
            {
                test = HEJ.Read_from_BT();
                cout << "RFID: "<< test << "\n";
            }
            else if(test == 9)
            {
                test = HEJ.Read_from_BT();
                cout << "Fram eller bakåt: "<< test << "\n";
            }
            else if(test == 10)
            {
                test = HEJ.Read_from_BT();
                cout << "Styrsignal Höger: "<< test << "\n";
            }
            else if(test == 11)
            {
                test = HEJ.Read_from_BT();
                cout << "Styrsignal Vänster: "<< test << "\n";
            }
            else
            {
               // cout << "test fel värde: " << test << "\n";
            }
        }*/

    //myWindow->draw(bgSprite);
    while (1)
    {

        test = HEJ.Read_from_BT();


            if(test == 1)
            {
                test = HEJ.Read_from_BT();
                if(test != 0)
                {
                    y1 = ((1/test) - 0.001086689563586)/0.000191822821525;

                }
                cout << "Fram: "<< test << " ";
            }
            else if(test == 2)
            {
                test = HEJ.Read_from_BT();
                if(test != 0)
                {
                    y2 = 9 + ((1/test) - 0.000741938763948)/0.0001637008132828;

                }
                cout << "Höger Fram: "<< test << " ";

            }
            else if(test == 3)
            {
                test = HEJ.Read_from_BT();
                if(test != 0)
                {
                    y3 = 9 + ((1/test) - 0.000741938763948)/0.0001637008132828;

                }
                cout << "Höger Bak: "<< test << " ";

            }
            else if(test == 4)
            {
                test = HEJ.Read_from_BT();
                if(test != 0)
                {
                   y4 = 9 + ((1/test) - 0.000741938763948)/0.0001637008132828;

                }
                cout << "Vänster Fram: "<< test << "\n";
            }

            else if(test == 5)
            {
                test = HEJ.Read_from_BT();
              // cout << "Vänster Bak: "<< test << "\n";
            }
            else if(test == 6)
            {
                test = HEJ.Read_from_BT();
               // cout << "Avstånd: "<< test << " ";
            }
            else if(test == 7)
            {
                test = HEJ.Read_from_BT();
               // cout << "Gyro: "<< test << " ";
            }
            else if(test == 8)
            {
                test = HEJ.Read_from_BT();
               // cout << "RFID: "<< test << " ";
            }
            else if(test == 9)
            {
                test = HEJ.Read_from_BT();
               // cout << "Fram eller bakåt: "<< test << " ";
            }
            else if(test == 10)
            {
                test = HEJ.Read_from_BT();
              //  cout << "Styrsignal Höger: "<< test << " ";
            }
            else if(test == 11)
            {
                test = HEJ.Read_from_BT();
              //  cout << "Styrsignal Vänster: "<< test << "\n";
            }
             else
            {
              // cout << "test fel värde: " << test << "\n";
            }
    }

/*
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
            case event.KeyPressed:
            {
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num0))
                {
                    y1 = 0;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
                {
                    y1 = 20;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
                {
                    y1 = 40;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num3))
                {
                    y1 = 60;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num4))
                {
                    y1 = 80;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num5))
                {
                    y1 = 100;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num6))
                {
                    y1 = 120;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num7))
                {
                    y1 = 140;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num8))
                {
                    y1 = 160;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Num9))
                {
                    y1 = 170;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                {
                    y1 = y1 + 1;
                    y2 = y2 + 3;
                    y3 = y3 + 1;
                    y4 = y4 - 2;
                }
                else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                {
                    y1 = y1 - 1;
                    y2 = y2 - 3;
                    y3 = y3 - 1;
                    y4 = y4 + 2;
                }
            }*/
    /*        }
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
            //std::cout << "hej" << std::endl;
        }
        //myWindow->clear();
    }
*/

    HEJ.~Bluetooth_Serial_comm();

    return 0;
}

/*
sf::RenderWindow window(sf::VideoMode(800, 600), "My window");






while(window.isOpen()) //(int i = 0; i < 100; i++)
{


   sf::Event event;
    while (window.pollEvent(event))
    {
        // "close requested" event: we close the window
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }

    }
    // clear the window with black color
    window.clear(sf::Color::Black);
    if(!test == 0)
    {
       // cout << test << "\n";
    }
    window.display();

}


*/

