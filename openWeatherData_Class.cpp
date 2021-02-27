#include "openWeatherData_Class.h"

long long getEpochTime() //grabs the current UNIX time
{
	const long long timeNow = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	return timeNow;
}

tmillis_t GetTimeInMillis()
{
	struct timeval tp;
	gettimeofday(&tp, nullptr);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}


int requestCurrentWeather::getWeatherData()
{
	try
	{
		// Build request URI and start the request.
		http_client client(U("http://api.openweathermap.org/data/2.5/weather?"));
		uri_builder builder;
		builder.append_query(U("zip"), location);
		builder.append_query(U("appid"), apiKey);
		builder.append_query(U("units"), getRequestTempUnits);
		return client.request(methods::GET, builder.to_string())
		             // Handle response headers arriving.

		             .then([&](http_response response)

		             {
			             if (response.status_code() == status_codes::OK)
			             {
				             //printf("Received response status code:%u\n", response.status_code());
				             openWeatherJSONResponse = response.extract_json().get();
				             json::value weather = openWeatherJSONResponse.at(U("weather"));
				             json::value main = openWeatherJSONResponse.at(U("main"));
				             json::value sys = openWeatherJSONResponse.at(U("sys"));
				             json::value wind = openWeatherJSONResponse.at(U("wind"));
				             arrayOfWeatherIDs.clear();
				             //in the format [currentTime, sunRise, sunSet]
				             for (auto i : weather.as_array())
				             {
					             arrayOfWeatherIDs.push_back(i.at(U("id")).as_integer());
					             std::wcout << i.at(U("id")).as_integer() << "\n";
				             }

				             currentTemperature = main.at(U("temp")).as_integer();
				             feelsLikeTemp = main.at(U("feels_like")).as_integer();
				             windSpeed = wind.at(U("speed")).as_integer();
				             int64_t sunrise = sys.at(U("sunrise")).as_number().to_int64();
				             int64_t sunset = sys.at(U("sunset")).as_number().to_int64();
				             arrayOfTimes.clear();
				             arrayOfTimes.push_back(getEpochTime());
				             arrayOfTimes.push_back(sunrise);
				             arrayOfTimes.push_back(sunset);
				             arrayOfTimesRef = &arrayOfTimes;
				             arrayOfWeatherIDsRef = &arrayOfWeatherIDs;
			             }
		             }).wait();
	}

		// Wait for all the outstanding I/O to complete and handle any exceptions

	catch (const std::exception& e)
	{
		std::wcout << e.what();
		return 1;
	}
}

int requestCurrentWeather::getCurrentTemperature() const
{
	return currentTemperature;
}

vector<int>* requestCurrentWeather::getWeatherIDArray() const
{
	return arrayOfWeatherIDsRef;
}

int requestCurrentWeather::getWindSpeed() const
{
	return windSpeed;
}

int requestCurrentWeather::getFeelsLikeTemp() const
{
	return feelsLikeTemp;
}

vector<long long>* requestCurrentWeather::getTimeArray() const
{
	return arrayOfTimesRef;
}

time_t requestCurrentWeather::getTime() const
{
	return counter;
}

void requestCurrentWeather::imageRenderListPush(string i)
{
	imageRenderList.push_back(i);
}

void requestCurrentWeather::imageRenderListPop()
{
	imageRenderList.erase(imageRenderList.begin());
}

vector<string> requestCurrentWeather::getImageRenderList() const
{
	return imageRenderList;
}

void requestCurrentWeather::clearImageRenderList()
{
	imageRenderList.clear();
}

void requestCurrentWeather::setLastImageRendered(string fileName)
{
	lastImageRenderedName = fileName;
}

string requestCurrentWeather::getLastImageRendered() const
{
	return lastImageRenderedName;
}

void requestCurrentWeather::setLastFile_Img(vector<FileInfo*> file)
{
	file_imgs = file;
}

vector<FileInfo*> requestCurrentWeather::getLastFile_Img()
{
	return file_imgs;
}

void requestCurrentWeather::initOpenWeatherOptions(weatherAPIOptions* initWeatherOptions)
{
	apiKey = initWeatherOptions->apiKey;
	getRequestTempUnits = initWeatherOptions->getRequestTempUnits;
	location = initWeatherOptions->location;
	currentTemperature = initWeatherOptions->currentTemperature;
	arrayOfWeatherIDs = initWeatherOptions->arrayOfWeatherIDs;
	arrayOfTimes = initWeatherOptions->arrayOfTimes;
	file_imgs = initWeatherOptions->file_imgs;
	windSpeed = initWeatherOptions->windSpeed;
	feelsLikeTemp = initWeatherOptions->feelsLikeTemp;
	counter = initWeatherOptions->counter;
	timeout = initWeatherOptions->timeout;
	lastImageRenderedName = initWeatherOptions->lastImageRenderedName;
	imageRenderList = initWeatherOptions->imageRenderList;
}
