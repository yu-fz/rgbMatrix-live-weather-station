#pragma once
#include <vector>
#include <cpprest/details/basic_types.h>

#include "imageViewerHelperFunctions.h"
//#include "openWeatherData_Class.h"

struct weatherAPIOptions
{
	utility::string_t apiKey; // API key generated from openweather.org to make calls 
	utility::string_t getRequestTempUnits; //select units (standard/metric/imperial)
	int currentTemperature;
	std::vector<int> arrayOfWeatherIDs;
	std::vector<long long> arrayOfTimes;

	std::vector<FileInfo*> file_imgs;
	int windSpeed;
	int feelsLikeTemp;

	time_t counter;
	time_t timeout;

	std::string lastImageRenderedName;
	std::vector<std::string> imageRenderList;
};

void initWeatherAPIOptions(weatherAPIOptions* initWeatherOptions);
