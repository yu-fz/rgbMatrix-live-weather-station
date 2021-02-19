// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2015 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

// To use this image viewer, first get image-magick development files
// $ sudo apt-get install libgraphicsmagick++-dev libwebp-dev
//
// Then compile with
// $ make led-image-viewer

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <random>
#include <map>
#include <string>
//#include <vector>
#include <Magick++.h>

#include <magick/image.h>
#include  "imageViewerHelperFunctions.h"
#include "imageFileDictHeader.h"
#include "graphics.h"


using namespace rgb_matrix;
using rgb_matrix::Canvas;
using rgb_matrix::RGBMatrix;

//web stuff for API requests
using namespace utility; // Common utilities like string conversions
using namespace web; // Common features like URIs.
using namespace http; // Common HTTP functionality
using namespace client; // HTTP client features
using namespace concurrency::streams; // Asynchronous streams

//timer 
using namespace std::chrono; // nanoseconds, system_clock, seconds
using std::chrono::seconds;
using std::chrono::system_clock;


//std c++ namespaces
using std::string;
using std::pair;
using std::vector;
using std::thread;

//method prototypes 
//string selectImagesToDraw(vector<int>&, vector<long long>&, time_t counter, requestCurrentWeather ); // receives weather code and UNIX time 
string getTemperatureToDisplay(int, int);
//interrupt_received = false; //check if Ctrl+C has been pressed


static void InterruptHandler(int signo)
{
	interrupt_received = true;
}

static tmillis_t GetTimeInMillis()
{
	struct timeval tp;
	gettimeofday(&tp, nullptr);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

long long getEpochTime() //grabs the current UNIX time
{
	const long long timeNow = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
	return timeNow;
}

static void StoreInStream(const Magick::Image& img,
                          int delay_time_us,
                          bool do_center,
                          FrameCanvas* scratch,
                          StreamWriter* output)
{
	scratch->Clear();
	const int x_offset = do_center ? (scratch->width() - img.columns()) / 2 : 0;
	const int y_offset = do_center ? (scratch->height() - img.rows()) / 2 : 0;
	for (size_t y = 0; y < img.rows(); ++y)
	{
		for (size_t x = 0; x < img.columns(); ++x)
		{
			const Magick::Color& c = img.pixelColor(x, y);
			if (c.alphaQuantum() < 256)
			{
				scratch->SetPixel(x + x_offset,
				                  y + y_offset,
				                  ScaleQuantumToChar(c.redQuantum()),
				                  ScaleQuantumToChar(c.greenQuantum()),
				                  ScaleQuantumToChar(c.blueQuantum()));
			}
		}
	}
	output->Stream(*scratch, delay_time_us);
}

static void CopyStream(StreamReader* r,
                       StreamWriter* w,
                       FrameCanvas* scratch)
{
	uint32_t delay_us;
	while (r->GetNext(scratch, &delay_us))
	{
		w->Stream(*scratch, delay_us);
	}
}

//

static void add_micros(struct timespec* accumulator, long micros)
{
	const long billion = 1000000000;
	const int64_t nanos = static_cast<int64_t>(micros) * 1000;
	accumulator->tv_sec += nanos / billion;
	accumulator->tv_nsec += nanos % billion;
	while (accumulator->tv_nsec > billion)
	{
		accumulator->tv_nsec -= billion;
		accumulator->tv_sec += 1;
	}
}

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

	int getWeatherData()
	{
		try
		{
			// Build request URI and start the request.
			http_client client(U("http://api.openweathermap.org/data/2.5/weather?"));
			uri_builder builder;
			builder.append_query(U("id"), 5128581);
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
					             //std::wcout << currentTemperature << "\n";
					             int64_t sunrise = sys.at(U("sunrise")).as_number().to_int64();
					             int64_t sunset = sys.at(U("sunset")).as_number().to_int64();
					             // std::wcout << sunrise << "\n";
					             // std::wcout << sunset << "\n";
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

	int getCurrentTemperature() const
	{
		return currentTemperature;
	}

	vector<int>* getWeatherIDArray() const
	{
		return arrayOfWeatherIDsRef;
	}

	int getWindSpeed() const
	{
		return windSpeed;
	}

	int getFeelsLikeTemp() const
	{
		return feelsLikeTemp;
	}

	vector<long long>* getTimeArray() const
	{
		return arrayOfTimesRef;
	}

	time_t getTime() const
	{
		return counter;
	}

	void imageRenderListPush(string i)
	{
		imageRenderList.push_back(i);
	}

	void imageRenderListPop()
	{
		imageRenderList.erase(imageRenderList.begin());
	}

	vector<string> getImageRenderList() const
	{
		return imageRenderList;
	}

	void clearImageRenderList()
	{
		imageRenderList.clear();
	}

	void setLastImageRendered(string fileName)
	{
		lastImageRenderedName = fileName;
	}

	string getLastImageRendered() const
	{
		return lastImageRenderedName;
	}

	void setLastFile_Img(vector<FileInfo*> file)
	{
		file_imgs = file;
	}

	vector<FileInfo*> getLastFile_Img()
	{
		return file_imgs;
	}
};


//Gets the necessary string to display on the bottom of the matrix
string getTemperatureToDisplay(int temp, int windSpeed)
{
	string temperature_str = std::to_string(temp);
	string windSpeed_str = std::to_string(windSpeed);
	string a = "Current Temp: ";
	string b = "Windspeed: ";
	string output = a.append(temperature_str).append("F").append("  ").append(b).append(windSpeed_str).append("mph");

	return output;
}

string selectImagesToDraw(vector<int>& weatherID, vector<long long>& times,
                          requestCurrentWeather weather, std::default_random_engine& seed)
//pass by reference an integer array [ [weatherCodes], [weatherID, currentTime, sunRise, sunSet, currentTemp]]

{
	//this function appends the proper directory to the image file path
	const long long currentTime = times[0];
	const long long sunRise = times[1];
	const long long sunSet = times[2];
	vector<string> filesToDraw = lookUpImageToDraw(weatherID);
	vector<string> imageRenderList = weather.getImageRenderList();
	string imageFilePath;
	string imageFileToBeRendered;

	//std::wcout << "length of filesToDraw " << filesToDraw.size();
	//imageRenderList.clear();
	if (filesToDraw != imageRenderList)
	{
		weather.clearImageRenderList();

		for (auto i : filesToDraw)
		{
			//std::cout << i;
			if ((currentTime > sunRise) && (currentTime <= sunSet))
			{
				imageFilePath = "./day/";
				string fullImageFilePath = imageFilePath.append(i).append(".png");
				if (std::find(imageRenderList.begin(), imageRenderList.end(), fullImageFilePath) ==
					imageRenderList.end())
					//only change  images in imageRenderList if there are new files to add 
				{
					// it is daytime, use the daytime icon folder path
					weather.imageRenderListPush(fullImageFilePath);
				}
			}
			else
			{
				//it is night, use nighttime icon folder path
				imageFilePath = "./night/";
				string fullImageFilePath = imageFilePath.append(i).append(".png");
				if (std::find(imageRenderList.begin(), imageRenderList.end(), fullImageFilePath) ==
					imageRenderList.end())
				{
					weather.imageRenderListPush(fullImageFilePath);
				}
			}
		}
	}

	if (weather.getWindSpeed() > 15)
	{
		weather.imageRenderListPush("./day/windy-1.png");
	}

	if (weather.getCurrentTemperature() < 30)
	{
		weather.imageRenderListPush("./day/lowTemp.png");
	}
	if (weather.getCurrentTemperature() > 88)
	{
		weather.imageRenderListPush("./day/hiTemp.png");
	}

	// Place image files into image render buffer every so often
	imageRenderList = weather.getImageRenderList();
	if (imageRenderList.size() > 1)
	{
		imageFileToBeRendered = imageRenderList.front();
		std::shuffle(std::begin(imageRenderList), std::end(imageRenderList), seed);
		imageFileToBeRendered = imageRenderList.front();
	}
	else
	{
		imageFileToBeRendered = imageRenderList.front();
	}

	return imageFileToBeRendered;
}

vector<FileInfo*> prepImageFileForRendering(string imageFile, FrameCanvas* offScreen, StreamWriter* globalStreamWriter,
                                            requestCurrentWeather currentWeather)
{
	if (currentWeather.getLastImageRendered().c_str() != imageFile.c_str())
	{
		//load it up

		std::map<const void*, struct ImageParams> filename_params;

		// Set defaults.
		ImageParams img_param;

		filename_params[nullptr] = img_param;

		const char* stream_output = nullptr;

		// Prepare matrix

		// These parameters are needed once we do scrolling.
		const bool fill_width = false;
		const bool fill_height = false;
		const bool do_center = true;

		std::vector<FileInfo*> file_imgs;

		const char* filename = imageFile.c_str();
		FileInfo* file_info = nullptr;

		std::string err_msg;
		std::vector<Magick::Image> image_sequence;
		if (LoadImageAndScale(filename,
		                      56,
		                      //Canvas->width(),
		                      56,
		                      //Canvas->height(),
		                      fill_width,
		                      fill_height,
		                      &image_sequence,
		                      &err_msg))
		{
			file_info = new FileInfo();
			file_info->params = filename_params[filename];
			file_info->content_stream = new MemStreamIO();
			file_info->is_multi_frame = false;
			StreamWriter out(file_info->content_stream);

			const Magick::Image& img = image_sequence[0];
			int64_t delay_time_us;

			delay_time_us = file_info->params.wait_ms * 1000; // single image.

			if (delay_time_us <= 0) delay_time_us = 100 * 1000; // 1/10sec

			StoreInStream(img,
			              delay_time_us,
			              do_center,
			              offScreen,
			              globalStreamWriter ? globalStreamWriter : &out);
		}

		if (file_info)
		{
			file_imgs.push_back(file_info);
		}

		currentWeather.setLastFile_Img(file_imgs);
		return file_imgs;
	}

	//else return currentWeather.getLastFile_Img();
	return currentWeather.getLastFile_Img();
}


static void initCanvasOptions(RGBMatrix::Options& canvasOptions)
{
	canvasOptions.hardware_mapping = "adafruit-hat"; //define driver board model
	canvasOptions.rows = 64;
	canvasOptions.cols = 64;
	canvasOptions.chain_length = 1;
	canvasOptions.parallel = 1;
	canvasOptions.row_address_type = 0;
	canvasOptions.multiplexing = 0;
}


static void initRuntimeOptions(RuntimeOptions& runtimeOptions)
{
	runtimeOptions.gpio_slowdown = 0; //no slowdown necessary because pi Zero is already slow enough
}

static bool FullSaturation(const Color& c)
{
	return (c.r == 0 || c.r == 255)
		&& (c.g == 0 || c.g == 255)
		&& (c.b == 0 || c.b == 255);
}


int main(int argc, char* argv[])
{
	Magick::InitializeMagick(*argv);

	RGBMatrix::Options canvasOptions;
	RuntimeOptions runtimeOptions;
	requestCurrentWeather currentWeather;
	currentWeather.getWeatherData();
	initCanvasOptions(canvasOptions);
	initRuntimeOptions(runtimeOptions);

	RGBMatrix* Canvas = RGBMatrix::CreateFromOptions(canvasOptions, runtimeOptions);
	FrameCanvas* offScreenCanvas = Canvas->CreateFrameCanvas();

	if (Canvas == nullptr)
	{
		return 1;
	}

	//text scroller options 
	Color color(255, 16, 46);
	Color bg_color(0, 0, 0);
	Color outline_color(0, 0, 0);
	bool with_outline = false;

	const char* bdf_font_file = "./spleen-5x8.bdf";
	//currentWeather.getWeatherData();
	int currentTemp = 0;
	int currentWindSpeed = 0;
	std::string line = "loading...";
	bool xorigin_configured = false;
	int x_orig = 0;
	int y_orig = 56;
	int letter_spacing = 0;
	float speed = 5.0f;
	int loops = -1;
	int blink_on = 0;
	int blink_off = 0;
	bool do_forever = true;


	if (line.empty())
	{
		fprintf(stderr, "Add the text you want to print on the command-line");
		return 1;
	}

	if (bdf_font_file == nullptr)
	{
		fprintf(stderr, "Need to specify BDF font-file");
		return 1;
	}

	/*
	* Load font. This needs to be a filename with a bdf bitmap font.
	*/
	Font font;
	if (!font.LoadFont(bdf_font_file))
	{
		fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
		return 1;
	}
	Font* outline_font = nullptr;
	if (with_outline)
	{
		outline_font = font.CreateOutlineFont();
	}

	const bool all_extreme_colors = (canvasOptions.brightness == 100)
		&& FullSaturation(color)
		&& FullSaturation(bg_color)
		&& FullSaturation(outline_color);
	if (all_extreme_colors)
		Canvas->SetPWMBits(1);

	printf("CTRL-C for exit.\n");
	const int scroll_direction = (speed >= 0) ? -1 : 1;
	speed = fabs(speed);
	int delay_speed_usec = 1000000;
	if (speed > 0)
	{
		delay_speed_usec = 1000000 / speed / font.CharacterWidth('W');
	}

	if (!xorigin_configured)
	{
		if (speed == 0)
		{
			// There would be no scrolling, so text would never appear. Move to front.
			x_orig = with_outline ? 1 : 0;
		}
		else
		{
			x_orig = scroll_direction < 0 ? Canvas->width() : 0;
		}
	}

	int x = x_orig;
	int y = y_orig;
	int length = 0;

	struct timespec next_frame = {0, 0};

	uint frame_counter = 0;

	bool do_center = true;
	bool do_shuffle = false;

	// We remember ImageParams for each image, which will change whenever
	// there is a flag modifying them. This map keeps track of filenames
	// and their image params (also for unrelated elements of argv[], but doesn't
	// matter).
	// We map the pointer instad of the string of the argv parameter so that
	// we can have two times the same image on the commandline list with different
	// parameters.
	std::map<const void*, struct ImageParams> filename_params;

	// Set defaults.
	ImageParams img_param;

	filename_params[nullptr] = img_param;

	const char* stream_output = nullptr;

	// Prepare matrix

	runtimeOptions.do_gpio_init = (stream_output == nullptr);

	// These parameters are needed once we do scrolling.
	const bool fill_width = false;
	const bool fill_height = false;

	// In case the output to stream is requested, set up the stream object.
	StreamIO* stream_io = nullptr;
	StreamWriter* global_stream_writer = nullptr;
	if (stream_output)
	{
		int fd = open(stream_output, O_CREAT | O_WRONLY, 0644);
		if (fd < 0)
		{
			perror("Couldn't open output stream");
			return 1;
		}
		stream_io = new FileStreamIO(fd);
		global_stream_writer = new StreamWriter(stream_io);
	}

	const tmillis_t start_load = GetTimeInMillis();
	// Preparing all the images beforehand as the Pi might be too slow to
	// be quickly switching between these. So preprocess.
	std::vector<FileInfo*> file_imgs;
	const char* filename = "./snow-2.png";
	FileInfo* file_info = nullptr;
	std::string err_msg;
	std::vector<Magick::Image> image_sequence;

	if (LoadImageAndScale(filename,
	                      Canvas->width(),
	                      Canvas->height(),
	                      fill_width,
	                      fill_height,
	                      &image_sequence,
	                      &err_msg))
	{
		file_info = new FileInfo();
		file_info->params = filename_params[filename];
		file_info->content_stream = new MemStreamIO();
		file_info->is_multi_frame = false;

		StreamWriter out(file_info->content_stream);

		const Magick::Image& img = image_sequence[0];
		int64_t delay_time_us;

		delay_time_us = file_info->params.wait_ms * 1000; // single image.

		if (delay_time_us <= 0) delay_time_us = 100 * 1000; // 1/10sec

		StoreInStream(img,
		              delay_time_us,
		              do_center,
		              offScreenCanvas,
		              global_stream_writer ? global_stream_writer : &out);
	}


	if (file_info)
	{
		file_imgs.push_back(file_info);
	}
	else
	{
		fprintf(stderr,
		        "%s skipped: Unable to open (%s)\n",
		        filename,
		        err_msg.c_str());
	}

	// Some parameter sanity adjustments.
	if (file_imgs.empty())
	{
		// e.g. if all files could not be interpreted as image.
		fprintf(stderr, "No image could be loaded.\n");
		return 1;
	}

	fprintf(stderr,
	        "Loading took %.3fs; now: Display.\n",
	        (GetTimeInMillis() - start_load) / 1000.0);

	currentWeather.setLastFile_Img(file_imgs);
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	time_t timeOut = 120;
	std::random_device randDevice{};
	auto rng = std::default_random_engine{randDevice()};

	string imageFile = selectImagesToDraw(*currentWeather.getWeatherIDArray(),
	                                      *currentWeather.getTimeArray(),
	                                      currentWeather, rng);
	file_imgs = prepImageFileForRendering(imageFile, offScreenCanvas, global_stream_writer, currentWeather);
	currentTemp = currentWeather.getCurrentTemperature();
	currentWindSpeed = currentWeather.getWindSpeed();
	time_t timeNow_image = time(nullptr);
	do
	{
		line = getTemperatureToDisplay(currentTemp, currentWindSpeed);
		++frame_counter;
		offScreenCanvas->Fill(bg_color.r, bg_color.g, bg_color.b);
		const bool draw_on_frame = (blink_on <= 0)
			|| (frame_counter % (blink_on + blink_off) < static_cast<uint>(blink_on));

		DisplayAnimation(file_imgs[0], Canvas, offScreenCanvas);
		if (time(nullptr) - timeNow_image > timeOut)
		{
			timeNow_image = time(nullptr);
			string imageFile = selectImagesToDraw(*currentWeather.getWeatherIDArray(),
			                                      *currentWeather.getTimeArray(),
			                                      currentWeather, rng);
			file_imgs = prepImageFileForRendering(imageFile, offScreenCanvas, global_stream_writer, currentWeather);
			currentWeather.getWeatherData();
			currentTemp = currentWeather.getCurrentTemperature();
			currentWindSpeed = currentWeather.getWindSpeed();
		}

		if (draw_on_frame)
		{
			if (outline_font)
			{
				// The outline font, we need to write with a negative (-2) text-spacing,
				// as we want to have the same letter pitch as the regular text that
				// we then write on top.
				DrawText(offScreenCanvas,
				         *outline_font,
				         x - 1,
				         y + font.baseline(),
				         outline_color,
				         nullptr,
				         line.c_str(),
				         letter_spacing - 2);
			}


			// length = holds how many pixels our text takes up

			length = DrawText(offScreenCanvas,
			                  font,
			                  x,
			                  y + font.baseline(),
			                  color,
			                  nullptr,
			                  line.c_str(),
			                  letter_spacing);
		}


		x += scroll_direction;
		if ((scroll_direction < 0 && x + length < 0) ||
			(scroll_direction > 0 && x > Canvas->width()))
		{
			x = x_orig + ((scroll_direction > 0) ? -length : 0);
			if (loops > 0) --loops;
		}

		// Make sure render-time delays are not influencing scroll-time
		if (speed > 0)
		{
			if (next_frame.tv_sec == 0)
			{
				// First time. Start timer, but don't wait.
				clock_gettime(CLOCK_MONOTONIC, &next_frame);
			}
			else
			{
				add_micros(&next_frame, delay_speed_usec);
				clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_frame, nullptr);
			}
		}
		// Swap the offscreen_canvas with canvas on vsync, avoids flickering
		offScreenCanvas = Canvas->SwapOnVSync(offScreenCanvas);
		if (speed <= 0) pause(); // Nothing to scroll.
	}
	while (do_forever && !interrupt_received);

	if (interrupt_received)
	{
		fprintf(stderr, "Caught signal. Exiting.\n");
	}
	// Animation finished. Shut down the RGB matrix.
	Canvas->Clear();
	delete Canvas;


	return 0;
}
