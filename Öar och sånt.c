bool isoutwall = true;
bool outwall()
{
	TransmitComm(outwall);
	return 
}

void secondlap()
{
	if(sensorleft < 260 || sensorleft >80)
	{
		outwall();
		if(!isoutwall)
		{
			isoutwall = true;
			Island();
		}
	}
	
	regulateright();
}


void traveledDist()
{
	if(posdistance > 32)  //40/1.27)*0.9
		{
			updatepos();
		}
}

void gotoIsland()
{
	while(frontsensor < 15)
		{
			traveledDist();
			driveF();
		}
		
		rotate90left();
		straight();
}

void storepos()
{
	storeposX = myposX;
	storeposY = mysposY;
}

void throwpos()
{
	storeposX = 42;
	storeposY = 42;
}

bool out = false;
bool Islandstart = true; // För att vi inte ska vara hemma när

bool alreadyDone()
{
		
		
}

void Island()
{
	if(!(alreadyDone()))
	{
		out = true;
		
		straight();
		rotate90left();
		
		gotoIsland();
		
		// Around Island
		if(out)
		{
			storepos();
			
			while(myposX != storeposX && mysposY != storeposY)
			{
				traveledDist();
				regulateright();
			}
			
			straight();
			rotate90left();
			out = false;
		}
		else
		{
			throwpos();
		}
}




}