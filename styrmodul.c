if(sensorfront<45)
		{

			drive(35);
			updatepos();
			transmit();
			sensorright = sidesensor(storedValues[1]);
			if(sensorright>20)
			{
				rotate90right();
				setcursor(1);
				print_on_lcd(0xff);
				transmit();
				sensorfront = frontsensor(storedValues[0]);
				if(sensorfront<65)
				{
					drivefromstill(40);
					updatepos();
					leftturn();
				}
				else
				{
					drivefromstill(40);
					updatepos();
				}
			}
			else
			{
				leftturn();
			}
		}
