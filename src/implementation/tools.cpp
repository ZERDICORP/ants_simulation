#include "tools.h"

int sense(std::vector<int>& map, bool bFoundFood, std::vector<float>& phemap, float fY, float fX, float fAngle,
	float fSensorAngleOffset, float fSensorLength, float fSensorDistance)
{
	float fSensorAngle = fAngle + fSensorAngleOffset;
	float fSummaryWeight = 0;

	for (int i = 0; i < fSensorLength; ++i)
	{
		int iTempY = fY + sin(fSensorAngle) * (i + 1 + fSensorDistance);
		int iTempX = fX + cos(fSensorAngle) * (i + 1 + fSensorDistance);

		if (zer::athm::inRange2D(mWH, mWW, iTempY, iTempX))
		{
			if (map[mWW * iTempY + iTempX] == WALL_PLACE)
			{
				fSummaryWeight = 0;
				break;
			}
			else if ((!bFoundFood && map[mWW * iTempY + iTempX] == FOOD_PLACE) || bFoundFood && map[mWW * iTempY + iTempX] == ANTHILL_PLACE)
			{
				fSummaryWeight = -1;
				break;
			}

			fSummaryWeight += phemap[mWW * iTempY + iTempX];
		}
		else
		{
			fSummaryWeight = 0;
			break;
		}
	}

	return fSummaryWeight;
}

void setPixelToPixmap(std::vector<uint8_t>& pixmap, int iIndex, sf::Color cColor)
{
	pixmap[iIndex * 4 + 0] = cColor.r;
	pixmap[iIndex * 4 + 1] = cColor.g;
	pixmap[iIndex * 4 + 2] = cColor.b;
	pixmap[iIndex * 4 + 3] = cColor.a;
}

void setFillCircleOnPixmap(std::vector<uint8_t>& pixmap, int iCY, int iCX, sf::Color cColor, int iRadius)
{
	for (int r = 1; r < iRadius + 1; ++r)
	{
		for (float i = 0; i < 360; ++i)
		{
			float fAngle = i * (mPi / 180);
			
			int iY = iCY + sin(fAngle) * r;
			int iX = iCX + cos(fAngle) * r;

			if (zer::athm::inRange2D(mWH, mWW, iY, iX))
				setPixelToPixmap(pixmap, mWW * iY + iX, cColor);
		}
	}
}

void setFillCircleOnMap(std::vector<int>& map, int iCY, int iCX, int place, int iRadius)
{
	for (int r = 1; r < iRadius + 1; ++r)
	{
		for (float i = 0; i < 360; ++i)
		{
			float fAngle = i * (mPi / 180);
			
			int iY = iCY + sin(fAngle) * r;
			int iX = iCX + cos(fAngle) * r;

			if (zer::athm::inRange2D(mWH, mWW, iY, iX))
				map[mWW * iY + iX] = place;
		}
	}
}

void displayConsoleInformation(std::string sCurrentPlace, std::map<std::string, float>& cfg, bool bRunning, int iBrushRadius)
{
	std::transform(sCurrentPlace.begin(), sCurrentPlace.end(), sCurrentPlace.begin(), [](unsigned char c){return std::toupper(c);});

	system("cls");

	std::cout << "# ants simulation #" << std::endl;
	std::cout << "\n[!] simulation state: " << (bRunning ? "RUNNING" : "STOPPED") << std::endl;
	std::cout << "\n[!] note: you CAN DRAW ONLY WHEN the simulation is STOPPED;" << std::endl;
	std::cout << "\n[!] current brush: " << sCurrentPlace << std::endl;
	std::cout << "\n[!] brush radius: " << iBrushRadius << std::endl;
	std::cout << "\n[!] change the radius of the brush: scrolling mouse WHEEL;" << std::endl;
	std::cout << "\n[!] paint with a brush: LEFT mouse button;" << std::endl;
	std::cout << "\n[!] move anthill (works only before the simulation starts): RIGHT mouse button;" << std::endl;
	std::cout << "\n[!] keyboard buttons for control:" << std::endl;
	std::cout << "\t [ ESC ] - exit;" << std::endl;
	std::cout << "\t [ SPACE ] - start/stop simulation;" << std::endl;
	std::cout << "\t [ R ] - restart simulation;" << std::endl;
	std::cout << "\t [ Q ] - change brush to next;" << std::endl;
	std::cout << "\t [ 1 ] - brush \"food\";" << std::endl;
	std::cout << "\t [ 2 ] - brush \"wall\";" << std::endl;
	std::cout << "\t [ 3 ] - brush \"empty\" (eraser);" << std::endl;
	std::cout << "\n[!] note: visit a \".cfg\" file to change simulation configuration;" << std::endl;
	std::cout << "\n[!] current configuration:" << std::endl;
	
	for (std::map<std::string, float>::iterator p = cfg.begin(); p != cfg.end(); p++)
		std::cout << "\t" << p -> first << " = " << p -> second << ";" << std::endl;
}

std::map<std::string, float> readConfig(std::string sConfigPath)
{
	std::map<std::string, float> cfg;

	zer::File file(sConfigPath);
	file.read({zer::file::Modifier::lines});

	for (int i = 0; i < file.linesLen(); ++i)
	{
		std::string sLine = file.lineAt(i);
		if (sLine.find(" = ") != std::string::npos)
		{
			std::vector<std::string> lineParts = zer::athm::split(sLine, " = ");
			cfg[lineParts[0]] = stof(lineParts[1]);
		}
	}

	return cfg;
}

PLACES whatIsHere(std::vector<int>& map, int iY, int iX)
{
	if (!zer::athm::inRange2D(mWH, mWW, iY, iX))
		return WALL_PLACE;
	return static_cast<PLACES>(map[mWW * iY + iX]);
}

EVENT_CODE eventListener(sf::RenderWindow& window)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			return CLOSE_EVENT_CODE;
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Escape)
				return CLOSE_EVENT_CODE;
			else if (event.key.code == sf::Keyboard::Space)
				return START_EVENT_CODE;
			else if (event.key.code == sf::Keyboard::R)
				return RESTART_EVENT_CODE;
			else if (event.key.code == sf::Keyboard::Q)
				return CHANGE_PLACE_TO_NEXT_EVENT_CODE;
			else if (event.key.code == sf::Keyboard::Num1)
				return BRUSH_FOOD_EVENT_CODE;
			else if (event.key.code == sf::Keyboard::Num2)
				return BRUSH_WALL_EVENT_CODE;
			else if (event.key.code == sf::Keyboard::Num3)
				return BRUSH_EMPTY_EVENT_CODE;
		}
		if (event.type == sf::Event::MouseWheelScrolled)
		{
			if (event.mouseWheelScroll.delta > 0)
				return MOUSE_SCROLL_UP_EVENT_CODE;
			else
				return MOUSE_SCROLL_DOWN_EVENT_CODE;
		}
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			return LEFT_MOUSE_PRESS_EVENT_CODE;
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
			return RIGHT_MOUSE_PRESS_EVENT_CODE;
	}
	return NULL_EVENT_CODE;
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(mWW, mWH), msTitle);
	return init(window);
}