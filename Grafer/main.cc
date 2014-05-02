#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <sstream>

int main()
{
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
            }
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
            std::cout << "hej" << std::endl;
        }
        //myWindow->clear();
    }
    return 0;
}






