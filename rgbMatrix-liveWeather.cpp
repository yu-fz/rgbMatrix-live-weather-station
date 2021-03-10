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
	make animated rain and snow 
	make weather icons look prettier 
	make it easier to change text scrolling color
	abstract away more settings into a configuration file 
*/


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


#include <magick/image.h>
#include "imageFileDictHeader.h"
#include "graphics.h"
#include "openWeatherData_Class.h"
#include "weatherAPIOptionSetup.h"
#include "imageViewerHelperFunctions.h"
#include "getPixelCanvas.h"
#include "animatedPixelFluids.h"

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

//Gets the necessary string to display on the bottom of the matrix
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

vector<FileInfo*> prepImageFileForRendering(string imageFile, FrameCanvas* offScreen,
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
		                      offScreen->width(),
		                      48,
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
			              &out);
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

void setBrightness(RGBMatrix* Canvas, requestCurrentWeather currentWeather)
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


int main(int argc, char* argv[])
{
	Magick::InitializeMagick(*argv);

	RGBMatrix::Options canvasOptions;
	RuntimeOptions runtimeOptions;
	requestCurrentWeather currentWeather;

	weatherAPIOptions* initWeatherOptions = new weatherAPIOptions();
	initWeatherAPIOptions(initWeatherOptions);
	currentWeather.initOpenWeatherOptions(initWeatherOptions);
	currentWeather.getWeatherData();
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
	//currentWeather.getWeatherData();
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

	time_t timeOut = 120;

	time_t seed = time(nullptr); //seed RNG with current unix time to shuffle the icon vector if len > 0
	std::mt19937 randDevice{seed};
	auto rng = std::default_random_engine{randDevice()};

	string imageFile = selectImagesToDraw(*currentWeather.getWeatherIDArray(),
	                                      *currentWeather.getTimeArray(),
	                                      currentWeather, rng);
	vector<FileInfo*> file_imgs = prepImageFileForRendering(imageFile, offScreenCanvas, currentWeather);
	currentTemp = currentWeather.getCurrentTemperature();
	currentWindSpeed = currentWeather.getWindSpeed();
	currentFeelsLikeTemp = currentWeather.getFeelsLikeTemp();
	time_t timeNow_image = time(nullptr);
	auto rainColor = Color(0, 119, 190);
	auto snowColor = Color(255, 255, 255);
	auto iceColor = Color(63, 208, 212);
	pixelParticle rainParticle(55, "rain", rainColor);
	pixelParticle snowParticle(5, "snow", snowColor);
	pixelParticle iceParticle(0, "ice", iceColor);

	do
	{
		rainParticle.spawnParticle(50, getPixelCanvas);
		snowParticle.spawnParticle(150, getPixelCanvas);
		rainParticle.updateParticles(getPixelCanvas);
		snowParticle.updateParticles(getPixelCanvas);
		//iceParticle.updateParticles(getPixelCanvas);
		iceParticle.freezeWaterParticles(getPixelCanvas);
		pixelParticle::drawParticles(getPixelCanvas);
		line = getTemperatureToDisplay(currentTemp, currentWindSpeed, currentFeelsLikeTemp);
		++frame_counter;
		offScreenCanvas->Fill(bg_color.r, bg_color.g, bg_color.b);
		const bool draw_on_frame = (blink_on <= 0)
			|| (frame_counter % (blink_on + blink_off) < static_cast<uint>(blink_on));

		//DisplayAnimation(file_imgs[0], Canvas, offScreenCanvas);
		if (time(nullptr) - timeNow_image > timeOut)
		{
			timeNow_image = time(nullptr);
			setBrightness(Canvas, currentWeather);
			string imageFile = selectImagesToDraw(*currentWeather.getWeatherIDArray(),
			                                      *currentWeather.getTimeArray(),
			                                      currentWeather, rng);
			file_imgs = prepImageFileForRendering(imageFile, offScreenCanvas, currentWeather);
			currentWeather.getWeatherData();
			currentTemp = currentWeather.getCurrentTemperature();
			currentWindSpeed = currentWeather.getWindSpeed();
			currentFeelsLikeTemp = currentWeather.getFeelsLikeTemp();
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
	delete Canvas;
	delete initWeatherOptions;
	delete getPixelCanvas.getPixelMap();
	return 0;
}
