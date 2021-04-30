#pragma once

#include "weatherAPIOptionSetup.h"
#include "imageViewerHelperFunctions.h"
#include <cpprest/http_client.h>
#include <vector>
#include <unistd.h>
#include <sys/time.h>

//web stuff for API requests
using namespace utility; // Common utilities like string conversions
using namespace web; // Common features like URIs.
using namespace http; // Common HTTP functionality
using namespace client; // HTTP client features
using namespace concurrency::streams; // Asynchronous streams

//std cpp stuff
using std::vector;
using std::string;

//timing stuff
using namespace std::chrono; // nanoseconds, system_clock, seconds
using std::chrono::seconds;
using std::chrono::system_clock;
long long getEpochTime(); //grabs the current UNIX time

tmillis_t GetTimeInMillis();

class requestCurrentWeather
{
private:
	json::value openWeatherJSONResponse;
	// Create http_client to send the request.
	string_t apiKey; //apiKey
	string_t getRequestTempUnits;
	string_t location;
	int currentTemperature;
	std::vector<int> arrayOfWeatherIDs;
	std::vector<int>* arrayOfWeatherIDsRef = &arrayOfWeatherIDs;
	std::vector<long long> arrayOfTimes;
	std::vector<long long>* arrayOfTimesRef = &arrayOfTimes;

	std::vector<FileInfo*> file_imgs;
	
	int windSpeed;
	
	int feelsLikeTemp;

	time_t counter;
	
	time_t timeout;

	string lastImageRenderedName;
	
	vector<string> imageRenderList;
	
	bool precipStatus;
	
	bool* precipStatusRef = &precipStatus;

public:

	int getWeatherData();


	// Wait for all the outstanding I/O to complete and handle any exceptions


	int getCurrentTemperature() const;


	vector<int>* getWeatherIDArray() const;


	int getWindSpeed() const;

	int getFeelsLikeTemp() const;


	vector<long long>* getTimeArray() const;


	time_t getTime() const;


	void imageRenderListPush(string i);


	void imageRenderListPop();

	vector<string> getImageRenderList() const;
	
	int getPrecipitationType(int weatherCode);
	
	int getPrecipitationIntensity(int weatherCode);
	
	int getWeatherArrayFirstElement();

	void clearImageRenderList();

	void setLastImageRendered(string fileName);

	string getLastImageRendered() const;

	void setLastFile_Img(vector<FileInfo*> file);

	vector<FileInfo*> getLastFile_Img();

	void initOpenWeatherOptions(weatherAPIOptions* initWeatherOptions);
	
	bool getPrecipImageStatus();
	void setPrecipImageStatus(bool status);
};
