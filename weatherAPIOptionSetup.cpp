#include "weatherAPIOptionSetup.h"


void initWeatherAPIOptions(weatherAPIOptions* initWeatherOptions)
{
	std::ifstream keyFile;
	string_t apiKey;
	string_t units;
	string_t location;

	keyFile.open("apiConfig.txt");
	if (keyFile.is_open())
	{
		std::getline(keyFile, apiKey);
		std::getline(keyFile, units);
		std::getline(keyFile, location);
	}
	else
	{
		std::cout << "api key file does not exist";
		exit(1);
	}
	initWeatherOptions->apiKey = apiKey;
	initWeatherOptions->getRequestTempUnits = units;
	initWeatherOptions->currentTemperature = 0;
	initWeatherOptions->arrayOfWeatherIDs = {};
	initWeatherOptions->arrayOfTimes = {};
	initWeatherOptions->file_imgs = {};
	initWeatherOptions->windSpeed = 0;
	initWeatherOptions->feelsLikeTemp = 0;
	initWeatherOptions->counter = time(nullptr);
	initWeatherOptions->timeout = 120;
	initWeatherOptions->imageRenderList = {};
}
