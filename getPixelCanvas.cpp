#include "getPixelCanvas.h"

int canvasWithGetPixel::width() const
{
	return delegatee_->width();
}

int canvasWithGetPixel::height() const
{
	return delegatee_->height();
}


void canvasWithGetPixel::Clear()
{
	delegatee_->Clear();
}

void canvasWithGetPixel::initPixelMap()
{
	pixelMap = new rgb_matrix::Color [canvasHeight * canvasWidth];
}

void canvasWithGetPixel::Fill(uint8_t red, uint8_t green, uint8_t blue)
{
	delegatee_->Fill(red, green, blue);
}

void canvasWithGetPixel::SetPixel(int x, int y, uint8_t red, uint8_t green, uint8_t blue)
{
	int offSetY = 35;
	rgb_matrix::Color pixelColor(red, green, blue);
	pixelMap[x + (y * canvasWidth)] = pixelColor;

	delegatee_->SetPixel(x, y + offSetY, red, green, blue);
}

rgb_matrix::Color canvasWithGetPixel::getPixel(int x, int y)
{
	rgb_matrix::Color pixelColor = pixelMap[x + (y * canvasWidth)];
	return pixelColor;
}
