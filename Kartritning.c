/*
*Kartitning.c
*
*marek588
*
*2014-04-17
*
*/



/******************* 29x15-matris med nollor *******************/

int main ()

{int map[29][15], i, j;

  for (i = 0; i < 29; i++)
  
  {for (j = 0; j < 15; j++)
  
    {map[i][j] = 0;
    
    printf("map[%d][%d] = %d \n", i, j, map[i][j]);
    
    //alt: printf("%d ", map[i][j]); } printf("\n");
    }
    
  }
  
  return 0;
  
}
  
  
