/*
*Kartritning.cc
*
*marek588
*
*2014-05-07
*
*
********************* 29 x 15 - matris med nollor********************
#include <iostream>




{int themap[29][15], i, j;

for (i = 0; i < 29; x++)
  
  {for (j = 0; j < 15; j++)
  
    {themap[i][j] = 0;
     cout << themap[i][j];
    }
    
  }
  
} Jag vet inte om denna ens behövs eftersom vi kommer rita ut ett fönster med RenderWindow.
Sen borde man kunna använda Rectangle Shape för att göra kartans linjer. Det är ju bara att fylla rektangeln med svart.

 */
 
//Rita ut ett tomt (vitt) fönster, pixlar eller?, i 29*15-storlek

//Rita ut roboten på kartan, i punkten (15,0). Typ en svart fyrkant eller nåt
 
/*Kolla vilket case det är och därmed i vilken riktning roboten rör sig för att veta var en eventuell vägg ska vara
Det finns typ i Search.c, borde funka med typ samma kod även i c++.
Ny idé: styrmodulen vet ju var vi har setwall. Fråga var vi ska setwall och sätt wall där.*/


//måste fråga roboten var den är varje gång den förflyttat sig en ruta för att dess position ska kunna uppdateras på kartan
