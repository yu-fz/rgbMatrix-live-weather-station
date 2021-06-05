#pragma once
#include "weatherAPIOptionSetup.h"
#include "imageViewerHelperFunctions.h"
#include <sstream>
#include <iostream>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Infos.hpp>
#include <nlohmann/json.hpp>
#include <curlpp/Options.hpp>
#include <vector>
#include <unistd.h>
#include <sys/time.h>

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
	
	// Create http_client to send the request.
	//http_client client; 
	//uri_builder builder;
	//json::value openWeatherJSONResponse;
	string_t apiKey ;  //apiKey
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
	
	bool precipStatus = false;
	
	bool* precipStatusRef = &precipStatus;

public:
	
	//requestCurrentWeather();
	
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
