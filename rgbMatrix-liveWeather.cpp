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

/* TODO 
	add stonk tracker
	make animated rain and snow ✅
	make weather icons look prettier 
	make it easier to change text scrolling color
	abstract away more settings into a configuration file 
*/

//#define _CRTDBG_MAP_ALLOC


#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <thread>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <random>
#include <map>
#include <string>
#include <Magick++.h>
#define CURL_STATICLIB
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <stdlib.h>

#include <magick/image.h>
#include "imageFileDictHeader.h"
#include "graphics.h"
#include "openWeatherData_Class.h"
#include "weatherAPIOptionSetup.h"
#include "imageViewerHelperFunctions.h"
#include "getPixelCanvas.h"
#include "animatedPixelFluids.h"
#include "pixelParticleSet.h"


using namespace rgb_matrix;
using rgb_matrix::Canvas;
using rgb_matrix::RGBMatrix;


static void InterruptHandler(int signo)
{
	interrupt_received = true;
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

//Gets the necessary string  to display on the bottom of the matrix
string getTemperatureToDisplay(int temp, int windSpeed, int feelsLikeTemp)
{
	string temperature_str = std::to_string(temp);
	string windSpeed_str = std::to_string(windSpeed);
	string feelsLikeTemp_str = std::to_string(feelsLikeTemp);
	string a = "Current Temp: ";
	string b = "Windspeed: ";
	string c = "Feels Like: ";
	string output = a.append(temperature_str).append("F").append("  ").append(b).append(windSpeed_str).append("mph").
	                  append("  ").append(c).append(feelsLikeTemp_str).append("F");

	return output;
}

string selectImagesToDraw(vector<int>& weatherID, vector<long long>& times,
                          requestCurrentWeather &weather, std::default_random_engine& seed)
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
	string errorIconFile = "./error_icon.png";

	if (!imageRenderList.empty() && (std::find(imageRenderList.begin(), imageRenderList.end(), errorIconFile) !=
		imageRenderList.end()))
	{
		imageFileToBeRendered = errorIconFile;
		return imageFileToBeRendered;
	}

	if (!filesToDraw.empty() && (filesToDraw != imageRenderList))
	{
		weather.clearImageRenderList();
		for (auto i : filesToDraw)
		{
			//string fullImageFilePath goes here
			//use ternary operators (if else on 1 line)
			if ((currentTime > sunRise) && (currentTime <= sunSet))
			{
				imageFilePath = "./day/";
				string fullImageFilePath = imageFilePath.append(i).append(".png");
				//if (std::find(imageRenderList.begin(), imageRenderList.end(), fullImageFilePath) ==
				//	imageRenderList.end())
					//only change  images in imageRenderList if there are new files to add 
				//{
					// it is daytime, use the daytime icon folder path
					weather.imageRenderListPush(fullImageFilePath);

				//}
			}
			else
			{
				//it is night, use nighttime icon folder path
				imageFilePath = "./night/";
				string fullImageFilePath = imageFilePath.append(i).append(".png");

				//std::cout << weather.getImageRenderList()[0] << std::endl;	
				
				//if (std::find(imageRenderList.begin(), imageRenderList.end(), fullImageFilePath) ==
				//	imageRenderList.end())
				//{

					//std::cout << fullImageFilePath << std::endl;
					weather.imageRenderListPush(fullImageFilePath);
				//}
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


	imageRenderList = weather.getImageRenderList();

	
	if (imageRenderList.size() > 1)
	{
		imageFileToBeRendered = imageRenderList.front();
		std::shuffle(std::begin(imageRenderList), std::end(imageRenderList), seed);
		imageFileToBeRendered = imageRenderList.front();
	}
	
	
	else if (imageRenderList.size() == 1)
	{
		//string firstImageInList = imageRenderList.front();
		//std::cout << dickButt << std::endl;
		imageFileToBeRendered = imageRenderList.front();
	}
	
	if (imageFileToBeRendered.find("precipitation") != std::string::npos)
	{
		//std::cout << "precip" << std::endl;
		weather.setPrecipImageStatus(true);
		
	}
	else
	{
		weather.setPrecipImageStatus(false);
	}
	
	return imageFileToBeRendered;
}


vector<FileInfo*> prepImageFileForRendering(string imageFile, FrameCanvas &offScreen,
                                            requestCurrentWeather &currentWeather, FileInfo *imageFileInfo)
{
	if (currentWeather.getLastImageRendered().c_str() != imageFile.c_str())
	{
		//load it up

		std::map<const void*, struct ImageParams> filename_params;

		// Set defaults.
		ImageParams img_param;

		filename_params[nullptr] = img_param;

		// Prepare matrix

		// These parameters are needed once we do scrolling.
		const bool fill_width = false;
		const bool fill_height = false;
		const bool do_center = true;

		std::vector<FileInfo*> file_imgs;

		const char* filename = imageFile.c_str();
		
		//FileInfo* file_info = nullptr;
		
		std::string err_msg;
		
		std::vector<Magick::Image> image_sequence;
		if (LoadImageAndScale(filename,
		                      offScreen.width(),
		                      48,
		                      //Canvas->height(),
		                      fill_width,
		                      fill_height,
		                      &image_sequence,
		                      &err_msg))
		{
	
			//file_info = new FileInfo();
			
			imageFileInfo->params = filename_params[filename];
			
			//imageFileInfo->content_stream = new MemStreamIO();
			
			imageFileInfo->is_multi_frame = false;
			
			StreamWriter out(imageFileInfo->content_stream);
			
			const Magick::Image& img = image_sequence[0];
			int64_t delay_time_us;

			delay_time_us = imageFileInfo->params.wait_ms * 1000; // single image.

			if (delay_time_us <= 0) delay_time_us = 100 * 1000; // 1/10sec
			
			StoreInStream(img,
			              delay_time_us,
			              do_center,
			              &offScreen,
			              &out);
		}
		

		file_imgs.push_back(imageFileInfo);
		currentWeather.setLastFile_Img(file_imgs);
		//delete file_info->content_stream;
		//delete file_info;
		///delete[] filename;
		//file_imgs.clear();
		//filename_params.clear();
		return currentWeather.getLastFile_Img();
	}
	//delete file_info;
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
	canvasOptions.brightness = 100;
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



void setBrightness(RGBMatrix* Canvas, requestCurrentWeather &currentWeather)
{
	auto sunRise = static_cast<time_t>(currentWeather.getTimeArray()->at(1));
	time_t timeNow = time(nullptr);
	struct tm* currentTime = localtime(&timeNow);
	int hour = currentTime->tm_hour;
	uint8_t lowBrightness = 10;
	uint8_t highBrightness = 100;
	if ((hour >= 0) && (timeNow <= sunRise))
	{
		Canvas->SetBrightness(lowBrightness);
		//switches display to low brightness in hours between midnight and sunrise 
	}
	else
	{
		Canvas->SetBrightness(highBrightness);
	}
}

void drawPrecipitation(canvasWithGetPixel &getPixelCanvas, pixelParticle &particle, int precipitationType, int precipitationIntensity)
{ 
	//implement with enums 
	//find other way to store severity besides the file directory string 
	//reimplement as class to capture states 
	
		//rain or snow
		if(precipitationType == 1) {
			
			particle.setParticleVelocity(8);    //speed limit is 1 pixel per tick 
		}else if(precipitationType == 2) {
			
			particle.setParticleVelocity(40);
		}
		particle.spawnParticle(precipitationIntensity, &getPixelCanvas);     //higher the number, means less of a chance for particles to spawn
		particle.updateParticles(&getPixelCanvas);
		pixelParticle::drawParticles(&getPixelCanvas);	
		
}


void drawPrecipitation(canvasWithGetPixel &getPixelCanvas, pixelParticle &rainParticle, pixelParticle &snowParticle, pixelParticle &iceParticle, int precipitationIntensity)
{ 
	//implement with enums 
	//find other way to store severity besides the file directory string 
	//reimplement as class to capture states 


			//sleet
			
			snowParticle.setParticleVelocity(10);
			rainParticle.setParticleVelocity(40);
			
			snowParticle.spawnParticle(70, &getPixelCanvas);
			snowParticle.updateParticles(&getPixelCanvas);
			rainParticle.spawnParticle(250, &getPixelCanvas);
			rainParticle.updateParticles(&getPixelCanvas);
			iceParticle.freezeWaterParticles(&getPixelCanvas);
			pixelParticle::drawParticles(&getPixelCanvas);


}

/*
void particleSpawner(requestCurrentWeather &currentWeather, canvasWithGetPixel &getPixelCanvas)
{
	
	auto rainColor = Color(0, 119, 190);
	auto snowColor = Color(255, 255, 255);
	auto iceColor = Color(63, 208, 212);
	
	pixelParticle* rainParticle = new pixelParticle(0, "rain", rainColor);
	pixelParticle* snowParticle = new pixelParticle(0, "snow", snowColor);
	pixelParticle* iceParticle = new pixelParticle(0, "ice", snowColor);
	

}*/

void drawPrecipitationHandler(requestCurrentWeather &currentWeather, canvasWithGetPixel &getPixelCanvas, pixelParticle &rainParticle, pixelParticle& snowParticle, pixelParticle& iceParticle)
{
	int precipitationID;
	int precipitationType;
	int precipitationIntensity;
	
	if (currentWeather.getPrecipImageStatus() == true)
	{
		
		

		for (auto i : *currentWeather.getWeatherIDArray())
		{
			if (i >= 300 && i <= 622)
			{
				precipitationID = i; //precip ID used to determine intensity of particle effects

			}
		}
		
		//create rain/snow/ice pixelparticle instances here
		//use a set to check whether or not the instance already exists
		//if false, delete the instance and remove from set  
		
		//precipitationIntensity = currentWeather.getPrecipitationIntensity(precipitationID);
		precipitationIntensity = 100;
		precipitationType = currentWeather.getPrecipitationType(precipitationID);
		std::cout << precipitationType << std::endl;
		switch (precipitationType)
		{

		case 1 :
			//rain
			//pixelParticle* rainParticle;
			//destroy all other particle types if they exist
			//std::cout << "rain" << std::endl;
			/*
			if(checkIfParticleExists("snow")) {
				
				//stop drawing and updating snow particles 
				//delete &snowParticle;
				snowParticle.clearParticles(&getPixelCanvas);
				removeParticleType("snow");
			}
			if (checkIfParticleExists("ice")) {
				//stop drawing and updating snow particles
				
				
				//delete &iceParticle;
				iceParticle.clearParticles(&getPixelCanvas);
				removeParticleType("ice");

			}
			if(checkIfParticleExists("rain") == false)
			{
				//destroy all other particle types if they exist

				//pixelParticle *rainParticle = new pixelParticle(55, "rain", rainColor);
			
				
				addParticleType("rain");
				drawPrecipitation(getPixelCanvas, rainParticle, precipitationType, precipitationIntensity);
			}
			else
			{
			*/
				drawPrecipitation(getPixelCanvas, rainParticle, precipitationType, precipitationIntensity);

			//}
		
			break;
			
		case 2:
			
			//snow
			//pixelParticle* snowParticle;
			/*
			if(checkIfParticleExists("rain")) {
				
				rainParticle.clearParticles(&getPixelCanvas);
				removeParticleType("rain");

			}
			if (checkIfParticleExists("ice")) {
				rainParticle.clearParticles(&getPixelCanvas);
				removeParticleType("ice");

			}
			if(checkIfParticleExists("snow") == false)
			{
				
				//pixelParticle* snowParticle = new pixelParticle(10, "snow", snowColor);
				addParticleType("snow");
				drawPrecipitation(getPixelCanvas, snowParticle, precipitationType, precipitationIntensity);
			}
			else
			{*/
				drawPrecipitation(getPixelCanvas, snowParticle, precipitationType, precipitationIntensity);

			//}
		
			break;
		case 3:
			//sleet 
			//pixelParticle* rainParticle;
			//pixelParticle* snowParticle;
			//pixelParticle* iceParticle;
			/*
			if (checkIfParticleExists("ice") == false)
			{
				drawPrecipitation(getPixelCanvas, rainParticle, snowParticle, iceParticle, precipitationIntensity);
				addParticleType("ice");
			}
			else
			{
				drawPrecipitation(getPixelCanvas, rainParticle, snowParticle, iceParticle, precipitationIntensity);
			}*/
			
			drawPrecipitation(getPixelCanvas, rainParticle, snowParticle, iceParticle, precipitationIntensity);
		
			break;
	
			}
		}
	/*
	else
	{
		
		if (checkIfParticleExists("rain")) {
			
			//delete &rainParticle;
			//rainParticle.clearParticles(&getPixelCanvas);
			removeParticleType("rain");
			
		}
		if (checkIfParticleExists("snow")) {
			
			snowParticle.clearParticles(&getPixelCanvas);
			removeParticleType("snow");
			
		}
		if (checkIfParticleExists("ice")) {
			
			iceParticle.clearParticles(&getPixelCanvas);
			removeParticleType("ice");
			
		}
	}*/
	

}


int main(int argc, char* argv[])
{
		
	Magick::InitializeMagick(*argv);
	cURLpp::initialize();
	RGBMatrix::Options canvasOptions;
	RuntimeOptions runtimeOptions;
	
	requestCurrentWeather* currentWeather = new requestCurrentWeather();

	weatherAPIOptions* initWeatherOptions = new weatherAPIOptions();
	
	FileInfo* fileInfo = new FileInfo();
	
	initWeatherAPIOptions(initWeatherOptions);
	currentWeather->initOpenWeatherOptions(initWeatherOptions);
	currentWeather->getWeatherData();
	initCanvasOptions(canvasOptions);
	initRuntimeOptions(runtimeOptions);
	RGBMatrix* Canvas = RGBMatrix::CreateFromOptions(canvasOptions, runtimeOptions);
	FrameCanvas* offScreenCanvas = Canvas->CreateFrameCanvas();
	canvasWithGetPixel getPixelCanvas(Canvas);
	getPixelCanvas.initPixelMap();
	if (Canvas == nullptr)
	{
		return 1;
	}
	

	//text scroller options 
	Color color(3, 4, 94);
	Color bg_color(0, 0, 0);
	Color outline_color(0, 0, 0);
	bool with_outline = false;

	const char* bdf_font_file = "./spleen-5x8.bdf";
	int currentTemp = 0;
	int currentFeelsLikeTemp = 0;
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


	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	time_t timeOut = 15;

	time_t seed = time(nullptr); //seed RNG with current unix time to shuffle the icon vector if len > 0
	std::mt19937 randDevice{seed};
	auto rng = std::default_random_engine{randDevice()};

	string imageFile = selectImagesToDraw(*(currentWeather->getWeatherIDArray()),
	                                      *(currentWeather->getTimeArray()),
	                                      *currentWeather, rng);
	//std::cout << imageFile << std::endl;
	vector<FileInfo*> file_imgs = prepImageFileForRendering(imageFile, *offScreenCanvas, *currentWeather, fileInfo);
	currentTemp = currentWeather->getCurrentTemperature();
	currentWindSpeed = currentWeather->getWindSpeed();
	currentFeelsLikeTemp = currentWeather->getFeelsLikeTemp();
	time_t timeNow_image = time(nullptr);
	
	auto rainColor = Color(0, 119, 190);
	auto snowColor = Color(255, 255, 255);
	auto iceColor = Color(63, 208, 212);
	
	pixelParticle* rainParticle = new pixelParticle(0, "rain", rainColor);
	pixelParticle* snowParticle = new pixelParticle(0, "snow", snowColor);
	pixelParticle* iceParticle = new pixelParticle(0, "ice", snowColor);
	
	do
	{
		line = getTemperatureToDisplay(currentTemp, currentWindSpeed, currentFeelsLikeTemp);
		++frame_counter;
		offScreenCanvas->Fill(bg_color.r, bg_color.g, bg_color.b);
		const bool draw_on_frame = (blink_on <= 0)
			|| (frame_counter % (blink_on + blink_off) < static_cast<uint>(blink_on));

		DisplayAnimation(file_imgs[0], Canvas, offScreenCanvas);
		drawPrecipitationHandler(*currentWeather, getPixelCanvas, *rainParticle, *snowParticle, *iceParticle); 

		if (time(nullptr) - timeNow_image > timeOut)
		{
	
			delete fileInfo->content_stream;
			fileInfo->content_stream = new MemStreamIO();
			timeNow_image = time(nullptr);
			setBrightness(Canvas, *currentWeather);
			string imageFile = selectImagesToDraw(*(currentWeather->getWeatherIDArray()),
				*(currentWeather->getTimeArray()),
				*currentWeather,
				rng);
			file_imgs = prepImageFileForRendering(imageFile, *offScreenCanvas, *currentWeather, fileInfo); //this line leaks memory
			currentWeather->getWeatherData(); //leaks
			currentTemp = currentWeather->getCurrentTemperature();
			currentWindSpeed = currentWeather->getWindSpeed();
			currentFeelsLikeTemp = currentWeather->getFeelsLikeTemp();
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
		usleep(16666);
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
	cURLpp::terminate();
	delete fileInfo;
	delete fileInfo->content_stream;
	delete Canvas;
	delete currentWeather;
	delete initWeatherOptions;
	delete rainParticle;
	delete snowParticle;
	delete iceParticle;
	getPixelCanvas.~canvasWithGetPixel();
	return 0;
}
