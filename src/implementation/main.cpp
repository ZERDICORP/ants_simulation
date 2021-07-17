#include "config.h"
#include "tools.h"
#include "ant.h"

int loop(sf::RenderWindow& window, std::map<std::string, float>& cfg)
{
	bool bNeedToUpdateConsole = true;
	bool bStarted = false;
	bool bRunning = false;

	int iBrushRadiusMax = 20;
	int iBrushRadiusMin = 1;
	int iBrushRadius = iBrushRadiusMax / 2;
	int iBrushShadowCoeff = 80;
	int currentPlace = FOOD_PLACE;

	float fPheromonesDisappearanceRateInAir = cfg["pheromonesDisappearanceRateInAir"];
	float fAntSensorLength = cfg["antSensorLength"];
	float fAntSensorDistance = cfg["antSensorDistance"];

	std::map<int, std::string> placesNames;
	placesNames[FOOD_PLACE] = "food";
	placesNames[WALL_PLACE] = "wall";
	placesNames[EMPTY_PLACE] = "empty";

	std::map<int, sf::Color> dBrushShadowColors;
	dBrushShadowColors[FOOD_PLACE] = sf::Color(0, 116 - iBrushShadowCoeff, 116 - iBrushShadowCoeff);
	dBrushShadowColors[WALL_PLACE] = sf::Color(202 - iBrushShadowCoeff, 202 - iBrushShadowCoeff, 202 - iBrushShadowCoeff);
	dBrushShadowColors[EMPTY_PLACE] = sf::Color(0, 0, 0);

	std::vector<int> map(mMapLength);
	for (int i = 0; i < mMapLength; ++i)
		map[i] = EMPTY_PLACE;

	std::vector<float> homePhemap(mMapLength, 0);
	std::vector<float> foodPhemap(mMapLength, 0);
	
	std::vector<uint8_t> pixmap(mMapLength * 4);
	for (int i = 0; i < mMapLength; ++i)
		setPixelToPixmap(pixmap, i, sf::Color(0, 0, 0));

	/*
		Main texture for draw pixel map.
	*/
	sf::Texture mapTexture;
	mapTexture.create(mWW, mWH);

	/*
		Anthill position.
	*/
	sf::Vector2f anthillPos(mWW * 0.2, mWH * 0.2);

	/*
		Creation and filling array of Ant(...) objects.
	*/
	std::vector<Ant> ants;
	for (int i = 0; i < ((int)cfg["antsQuantity"] < 360 ? 360 : (int)cfg["antsQuantity"]);
		i += (((int)cfg["antsQuantity"] / 360) == 0 ? 360 / ((int)cfg["antsQuantity"] % 360) : 1))
	{
		float fAngle = zer::athm::toRadians(i);
		ants.push_back(Ant{false, fAngle, anthillPos.y + (float)sin(fAngle), anthillPos.x + (float)cos(fAngle), cfg["pheromoneConcentration"]});
	}

	auto updatePhemap = [&fPheromonesDisappearanceRateInAir](std::vector<float>& phemap, int i){
		phemap[i] *= (1 - fPheromonesDisappearanceRateInAir);
		if (phemap[i] < 1)
			phemap[i] = 0;
	};

	auto senseWrapper = [&map, &fAntSensorLength, &fAntSensorDistance](float fSensorAngleOffset, std::vector<float>& phemap, Ant& ant) -> int {
		return sense(map, ant.bFoundFood, phemap, ant.fY, ant.fX, ant.fAngle, zer::athm::toRadians(fSensorAngleOffset),
			fAntSensorLength, fAntSensorDistance);
	};

	while (window.isOpen())
	{
		if (bRunning)
		{
			/*
				Each ant takes a step, after which it is drawn on the map.
			*/
			for (int i = 0; i < ants.size(); ++i)
			{
				map[mWW * (int)ants[i].fY + (int)ants[i].fX] = EMPTY_PLACE;

				ants[i].fPheromoneConcentration *= (1 - cfg["pheromonesDisappearanceRateInsideAnt"]);

				std::vector<float>& phemap = ants[i].bFoundFood ? homePhemap : foodPhemap;

				int iWeightForward = senseWrapper(0, phemap, ants[i]);
				int iWeightLeft = senseWrapper(cfg["antSensorAngleOffset"], phemap, ants[i]);
				int iWeightRight = senseWrapper(-cfg["antSensorAngleOffset"], phemap, ants[i]);

				float fRandomAmplifier = zer::athm::rand_float();

				if (iWeightForward < 0 || iWeightLeft < 0 || iWeightRight < 0)
				{
					if (iWeightForward < 0)
						ants[i].fAngle += 0;
					else if (iWeightLeft < 0)
						ants[i].fAngle += fRandomAmplifier * cfg["antTurnSpeed"];
					else if (iWeightRight < 0)
						ants[i].fAngle -= fRandomAmplifier * cfg["antTurnSpeed"];
				}
				else
				{
					if (iWeightForward > iWeightLeft && iWeightForward > iWeightRight)
						ants[i].fAngle += 0;
					else if (iWeightForward < iWeightLeft && iWeightForward < iWeightRight)
						ants[i].fAngle += (fRandomAmplifier - 0.5) * 2 * cfg["antTurnSpeed"];
					else if (iWeightLeft > iWeightRight)
						ants[i].fAngle += fRandomAmplifier * cfg["antTurnSpeed"];
					else if (iWeightRight > iWeightLeft)
						ants[i].fAngle -= fRandomAmplifier * cfg["antTurnSpeed"];
					else
						ants[i].fAngle += zer::athm::toRadians(zer::athm::rand_int(-10, 10));
				}

				float fNextY = ants[i].fY + sin(ants[i].fAngle) * cfg["antSpeed"];
				float fNextX = ants[i].fX + cos(ants[i].fAngle) * cfg["antSpeed"];

				switch (whatIsHere(map, fNextY, fNextX))
				{
					case WALL_PLACE:
						ants[i].fAngle += zer::athm::toRadians(zer::athm::rand_choice({-90, 90}, 2));
						break;

					case FOOD_PLACE:
						if (!ants[i].bFoundFood)
						{
							ants[i].fPheromoneConcentration = cfg["pheromoneConcentration"];
							setFillCircleOnMap(map, fNextY, fNextX, EMPTY_PLACE, 1);
							foodPhemap[mWW * (int)fNextY + (int)fNextX] += ants[i].fPheromoneConcentration;
							ants[i].bFoundFood = true;
						}
						ants[i].fAngle += zer::athm::toRadians(180);
						break;


					case ANTHILL_PLACE:
						if (ants[i].bFoundFood)
						{
							ants[i].bFoundFood = false;
							ants[i].fPheromoneConcentration = cfg["pheromoneConcentration"];
							ants[i].fAngle += zer::athm::toRadians(180);
							ants[i].bFoundFood = false;
							break;
						}

					default:
						(ants[i].bFoundFood ? foodPhemap : homePhemap)[mWW * (int)fNextY + (int)fNextX] += ants[i].fPheromoneConcentration;
						ants[i].fY = fNextY;
						ants[i].fX = fNextX;
						break;
				};

				map[mWW * (int)ants[i].fY + (int)ants[i].fX] = ANT_PLACE;
			}
		}

		/*
			Anthill drawing.
		*/
		setFillCircleOnMap(map, anthillPos.y, anthillPos.x, ANTHILL_PLACE, 10);

		/*
			Map & pheromone map drawing on pixel map (as well as pheromone map update).
		*/
		for (int i = 0; i < map.size(); ++i)
		{
			switch (map[i])
			{	
				case ANT_PLACE:
					setPixelToPixmap(pixmap, i, sf::Color(255, 255, 255));
					break;
				
				case WALL_PLACE:
					setPixelToPixmap(pixmap, i, sf::Color(202, 202, 202));
					break;
				
				case FOOD_PLACE:
					setPixelToPixmap(pixmap, i, sf::Color(0, 116, 116));
					break;
				
				case EMPTY_PLACE:
					setPixelToPixmap(pixmap, i, sf::Color((int)homePhemap[i] > 255 ? 255 : (int)homePhemap[i], 
						(int)foodPhemap[i] > 255 ? 255 : (int)foodPhemap[i], 0));
					break;
			};

			if (bRunning)
			{
				/*
					Phemaps updating.
				*/
				updatePhemap(homePhemap, i);
				updatePhemap(foodPhemap, i);
			}
		}

		/*
			Drawing shadow of brush.
		*/
		sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
		if (!bRunning)
			setFillCircleOnPixmap(pixmap, mousePosition.y, mousePosition.x, dBrushShadowColors[currentPlace], iBrushRadius);

		/*
			Anthill drawing.
		*/
		setFillCircleOnPixmap(pixmap, anthillPos.y, anthillPos.x, sf::Color(237, 205, 0), 10);

		/*
			Texture updating from a pixel map.
		*/
		mapTexture.update(&pixmap[0]);

		/*
			Texture rendering and window updating.
		*/
		window.draw(sf::Sprite(mapTexture));
		window.display();

		/*
			Show current state in console.
		*/
		if (bNeedToUpdateConsole)
		{
			displayConsoleInformation(placesNames[currentPlace], cfg, bRunning, iBrushRadius);
			bNeedToUpdateConsole = false;
		}

		/*
			Event handler.
		*/
		switch (eventListener(window))
		{
			case CLOSE_EVENT_CODE:
				window.close();
				break;

			case START_EVENT_CODE:
				bNeedToUpdateConsole = true;
				bRunning = !bRunning;
				if (!bStarted)
					bStarted = true;
				break;

			case RESTART_EVENT_CODE:
				return init(window);

			/*
				Changing brush.
			*/
			case CHANGE_PLACE_TO_NEXT_EVENT_CODE:
				currentPlace++;
				if (currentPlace > placesNames.size() - 1)
					currentPlace = FOOD_PLACE;
				bNeedToUpdateConsole = true;
				break;

			case BRUSH_FOOD_EVENT_CODE:
				currentPlace = FOOD_PLACE;
				bNeedToUpdateConsole = true;
				break;

			case BRUSH_WALL_EVENT_CODE:
				currentPlace = WALL_PLACE;
				bNeedToUpdateConsole = true;
				break;

			case BRUSH_EMPTY_EVENT_CODE:
				currentPlace = EMPTY_PLACE;
				bNeedToUpdateConsole = true;
				break;

			/*
				Changing the radius of the brush.
			*/
			case MOUSE_SCROLL_UP_EVENT_CODE:
				if (iBrushRadius < iBrushRadiusMax)
				{
					iBrushRadius++;
					bNeedToUpdateConsole = true;
				}
				break;

			case MOUSE_SCROLL_DOWN_EVENT_CODE:
				if (iBrushRadius > iBrushRadiusMin)
				{
					iBrushRadius--;
					bNeedToUpdateConsole = true;
				}
				break;

			/*
				Drawing map elements.
			*/
			case LEFT_MOUSE_PRESS_EVENT_CODE:
				if (!bRunning)
					setFillCircleOnMap(map, mousePosition.y, mousePosition.x, currentPlace, iBrushRadius);
				break;
			
			/*
				Anthill position updating.
			*/
			case RIGHT_MOUSE_PRESS_EVENT_CODE:
			{
				if (!bStarted)
				{
					setFillCircleOnMap(map, anthillPos.y, anthillPos.x, EMPTY_PLACE, 10);
					
					anthillPos.y = mousePosition.y;
					anthillPos.x = mousePosition.x;

					for (int i = 0; i < ants.size(); ++i)
					{
						ants[i].fY = anthillPos.y;
						ants[i].fX = anthillPos.x;
					}
				}
				break;
			}
		};
	}

	return 0;
}

int init(sf::RenderWindow& window)
{
	/*
		Config.
	*/
	std::map<std::string, float> cfg = readConfig(msConfigPath);

	return loop(window, cfg);
}