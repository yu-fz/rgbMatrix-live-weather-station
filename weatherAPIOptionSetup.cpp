#include "weatherAPIOptionSetup.h"


void initWeatherAPIOptions(weatherAPIOptions* initWeatherOptions)
{
	initWeatherOptions->apiKey = U("2cad0f109bdc9bde64036cb481a0a493");
	initWeatherOptions->getRequestTempUnits = U("imperial");
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
