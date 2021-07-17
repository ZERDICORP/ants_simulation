#include "config.h"

int sense(std::vector<int>& map, bool bFoundFood, std::vector<float>& phemap, float fY, float fX, float fAngle,
	float fSensorAngleOffset, float fSensorLength, float fSensorDistance);

void displayConsoleInformation(std::string sCurrentPlace, std::map<std::string, float>& cfg, bool bRunning, int iBrushRadius);
void setPixelToPixmap(std::vector<uint8_t>& ui8Pixmap, int iIndex, sf::Color cColor);
void setFillCircleOnPixmap(std::vector<uint8_t>& ui8Pixmap, int iCY, int iCX, sf::Color cColor, int iRadius);
void setFillCircleOnMap(std::vector<int>& map, int iCY, int iCX, int place, int iRadius);

std::map<std::string, float> readConfig(std::string sConfigPath);

PLACES whatIsHere(std::vector<int>& map, int iY, int iX);

EVENT_CODE eventListener(sf::RenderWindow& window);

int init(sf::RenderWindow& window);
int main();