#pragma once

#include "led-matrix.h"
#include "content-streamer.h"
#include <Magick++.h>
#include <sys/time.h>
#include <random>

typedef int64_t tmillis_t;
extern tmillis_t distant_future;
extern volatile bool interrupt_received;

//RGB Matrix File Stream stuff
using rgb_matrix::RGBMatrix;
using rgb_matrix::StreamReader;
using rgb_matrix::StreamWriter;
using rgb_matrix::FrameCanvas;
using rgb_matrix::StreamIO;
using rgb_matrix::FileStreamIO;

// Load still image or animation.
// Scale, so that it fits in "width" and "height" and store in "result".
bool LoadImageAndScale(const char* filename,
                       int target_width,
                       int target_height,
                       bool fill_width,
                       bool fill_height,
                       std::vector<Magick::Image>* result,
                       std::string* err_msg);


struct ImageParams
{
	ImageParams()
		: anim_duration_ms(distant_future)
		  , wait_ms(1)
		  , anim_delay_ms(-1)
		  , loops(-1)
		  , vsync_multiple(1)
	{
	}

	tmillis_t anim_duration_ms; // If this is an animation, duration to show.
	tmillis_t wait_ms; // Regular image: duration to show.
	tmillis_t anim_delay_ms; // Animation delay override.
	int loops;
	int vsync_multiple;
};


struct FileInfo
{
	ImageParams params; // Each file might have specific timing settings
	bool is_multi_frame;
	StreamIO* content_stream;
};

void DisplayAnimation(const FileInfo* file,
                      RGBMatrix* matrix,
                      FrameCanvas* offscreen_canvas);

static void SleepMillis(tmillis_t milli_seconds);
