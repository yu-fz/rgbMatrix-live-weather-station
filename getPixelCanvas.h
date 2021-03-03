#pragma once

#include "graphics.h"
#include "led-matrix.h"


class canvasWithGetPixel : public rgb_matrix::Canvas
{
private:

	//rgb_matrix::FrameCanvas* const delegatee_;
	Canvas* const delegatee_;
	//limit rain/snow display window to 24x64 pixels 
	int canvasWidth = 64;
	int canvasHeight = 20;
	rgb_matrix::Color* pixelMap;
public:
	canvasWithGetPixel(Canvas* canvas) : delegatee_(canvas)
	{
	}

	int width() const override;
	int height() const override;
	void Clear() override;
	void Fill(uint8_t red, uint8_t green, uint8_t blue) override;
	void initPixelMap();
	void SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue) override;

	rgb_matrix::Color getPixel(int x, int y);
};
