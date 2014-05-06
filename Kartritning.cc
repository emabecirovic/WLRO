/*
*Kartritning.cc
*
*marek588
*
*2014-04-17
*
*
********************* 29 x 15 - matris med nollor********************** 

#include <iostream>


int main()

{int array[29][15], i, j;

for (i = 0; i < 29; i++)
  
  {for (j = 0; j < 15; j++)
  
    {array[i][j] = 0;
     cout << array[i][j];
    }
    
  }
  
}
 */
 
//Rita ut ett tomt (vitt) fönster, pixlar eller?, i 29*15-storlek

//Rita ut roboten på kartan, i punkten (15,0)
 
/*Kolla vilket case det är och därmed i vilken riktning roboten rör sig för att veta var en eventuell vägg ska vara
Det finns typ i Search.c, borde funka med typ samma kod även i c++

void setwall(int x,int y)
		{
			room[x][y]=1;
		}

		void updatemap() // Kan väl bara gälla för yttervarvet?
{
	char w=30; //Hur långt ifrån vi ska vara för att säga att det är en vägg.

	int sensorfront;
	int sensormeanright;
	int sensormeanleft;

	switch(mydirection)
	{
		case (1): // X+
		if(sensormeanright<=w) //Vet inte vad som är en lämplig siffra här
		{
			setwall(myposX,myposY-1);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX,myposY+1);
		}
		break;

		case (2): // Y+
		if(sensormeanright<=w)
		{
			setwall(myposX+1,myposY);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX-1,myposY);
		}
  	break;

		case (3): // X-
		if(sensormeanright<=w)
		{
			setwall(myposX,myposY+1);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX,myposY-1);
		}
		break;

		case (4): // Y-
		if(sensormeanright<=w)
		{
			setwall(myposX-1,myposY);
		}
		else if(sensorfront<=w)
		{
			setwall(myposX,myposY-1);
		}
		else if(sensormeanleft<w)
		{
			setwall(myposX+1,myposY);
		}
		break;
	}
}

Jag tänker att det här "extended wall" som vi skrev för roboten inte behöver visas i datorn ju. väl?*/


//måste fråga roboten var den är varje gång den förflyttat sig en ruta för att dess position ska kunna uppdateras på kartan
