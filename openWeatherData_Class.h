#pragma once


#include "imageViewerHelperFunctions.h"
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <vector>
#include <unistd.h>

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
	const string_t apiKey = U("2cad0f109bdc9bde64036cb481a0a493"); //apiKey
	const string_t getRequestTempUnits = U("imperial"); //select units (standard/metric/imperial)
	int currentTemperature = 0;
	std::vector<int> arrayOfWeatherIDs = {721};
	std::vector<int>* arrayOfWeatherIDsRef = &arrayOfWeatherIDs;
	std::vector<long long> arrayOfTimes = {100, 200, 300};
	std::vector<long long>* arrayOfTimesRef = &arrayOfTimes;

	std::vector<FileInfo*> file_imgs;
	int windSpeed = 0;
	int feelsLikeTemp = 0;

	time_t counter = time(nullptr);

	time_t timeout = 30;

	string lastImageRenderedName;
	vector<string> imageRenderList = {"./snow-2.png"};

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


	void clearImageRenderList();

	void setLastImageRendered(string fileName);

	string getLastImageRendered() const;

	void setLastFile_Img(vector<FileInfo*> file);

	vector<FileInfo*> getLastFile_Img();
};
