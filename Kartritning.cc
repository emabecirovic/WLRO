/*
*
*Kartritning.cc
*
*marek588
*
*2014-05-08
*/


//Rita ut ett tomt (vitt) fönster, pixlar eller?, i 29*15-storlek, med RenderWindow

//Rita ut roboten på kartan, i punkten (15,0) kommer den börja. Typ en fylld fyrkant eller nåt.
sf::RectangleShape robotposition;
robotposition.setSize(sf::Vector2f(10, 10)); //fett osäker på storleken..
robotposition.setOutlineColor(sf::Color::Blue);
robotposition.setOutlineThickness(5); // och tjockleken.
robotposition.setPosition(myposX, myposY);

window.draw(robotposition);

 
//anvönd setfillcolor för att fylla rektangel

//bör rita ut väggar på rätt plats när storleken är fixad
sf::RectangleShape wall;
wall.setSize(sf::Vector2f(10, 10));
wall.setOutlineColor(sf::Color::Black);
wall.setOutlineThickness(1);
wall.setPosition(x, y); //samma x och y som i setwall 

window.draw(wall);


//bör rita ut brandhärden
if (isRFID==1)
{
sf::CircleShape fire;
fire.setRadius(5);
fire.setOutlineColor(sf::Color::Red);
fire.setOutlineThickness(5);
fire.setPosition(,);

window.draw(fire);
}


