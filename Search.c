/*
 * Search.c
 *
 * Created: 4/29/2014 9:26 AM
 *  Author: robsv107
 */
 
 
 typedef int bool;
 enum{false, true};
 
 bool onelap = false; //1 då yttervarvet körts
 bool finished = false; //1 då hela kartan utforskad
 bool home = false; //1 då robten återvänt till startposition
 char mydirection = 2; //1=X+ ; 2=Y+ ; 3=X- ; 4=Y-
 char traveled=0; //Hur långt roboten har färdats (sedan senast vi tog emot värden från sensor?)
 unsigned int myposX=0; //Robotens position i X-led
 unsigned int myposY=0; //Robotens position i Y-led
 unsigned int mypos[2]={15,0}; //Vektor med robotens koordinater; (x,y)
 unsigned int startpos[2]={15,0}; //Startpositionen sätts till mitten på nedre lånsidan
 int firstzero; //Första nollan om man läser matrisen uppifrån och ned
 
 int[][] room = createMatrix();

int[][] createMatrix()
{
  int matrix[29][15], i, j;

  for (i = 0; i < 29; i++)
    {
      for (j = 0; j < 15; j++)
      {
        matrix[i][j] = 0;
      }
    }
    return matrix;
}

void firstlap();
{
 //REGLERING
			//Omvandling till centimeter
			//sensor1r = ((1/storedValues[1]) - 0.000741938763948) / 0.001637008132828;
			sensor1r = storedValues[1];
			sensor1r = 1/sensor1r;
			sensor1r = sensor1r - 0.000741938763948;
			sensor1r = sensor1r / 0.001637008132828;

			//sensor2r = ((1/storedValues[2]) - 0.000741938763948) / 0.001637008132828;
			sensor2r = storedValues[2];
			sensor2r= 1/sensor2r;
			sensor2r = sensor2r - 0.000741938763948;
			sensor2r = sensor2r / 0.001637008132828;	

			sensorfront = storedValues[0]; 
			sensormeanr = (sensor1r + sensor2r) / 2;
			//till PD-reglering
			float sensormeanr_old;
			int Td = 1;
 
 
}


int main()
{
  MasterInit();
 	Initiation();

 	int fjarrstyrt = (PIND & 0x01); //1 då roboten är i fjärrstyrt läge

 	if(fjarrstyrt==1)
 	 {
 	 	int button=PINB;
 	 	fjarrstyr(button);
	  }
 	else
  	{	
  	 while(1)
  	 {
  	  
  	  if(start_request == 1)
	    	{
	   	 	//writechar(0b01001100); //L
	    		start_request = 0;
	    		if(regulateright)
		    	TransmitSensor(right);
	    		else if(regulateleft)
	    		TransmitSensor(left);
	    		else if(regulateturn)
	    		TransmitSensor(turn);
	    		else
	    		TransmitSensor(0x00);
 
		    	start_request =	0;
	    		TCCR0B = 0b0000101; // Start timer
    		}
    		
    		if(onelap)
    		{
    		 regulateright=true;
    		 firstlap();
    		}
    		
    		
    		
  	  
  	  while(home==0)
   		 {
	   	 	
	   	 	if(oneSquare)
	   	 	updatepos();
	   	 	
	   	 	
	 	   	if(onelap==0)
	      	{
 	     		firstlap();
 	     	}
	     	else if(finished==0)
			     {
 			     	//gör massa skit
		     	}
	     	else
		     	{
 		     	returntostart();
			     }
	   	 }		
 	  }
  	}
	return 0;
}
