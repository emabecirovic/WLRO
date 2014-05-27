bool alreadyDone()
{
	searched = false;
	TransmitComm(alrdydone);
	return searched;
}

void secondlap()
{
	if(sensorleft < 250 && sensorleft > 50)
	{
		if(!alreadydone())
		{
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
 // För att vi inte ska vara hemma när

void Island()
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
			
			throwpos();
			gotoIsland();
		}
}
