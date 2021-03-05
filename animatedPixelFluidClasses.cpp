#include "animatedPixelFluids.h"

void pixelParticle::spawnParticle(int intensity, canvasWithGetPixel Canvas)
{
	//use RNG to determine which pixel in the "spawn layer" (y = 0) gets to spawn a particle every tick 
	// confine spawn area to a 20 pixel wide strip in the center ...31+/- 10px maybe
	// ~x = [21, 41]

	time_t seed = time(nullptr); // obtain a random number from hardware
	std::mt19937 gen(seed); // seed the generator
	std::uniform_int_distribution<> distr(1, intensity); // define the range
	int randNum;
	for (int x = 21; x <= 41; x++)
	{
		//do RNG here
		randNum = distr(gen);
		if (randNum <= 10)
		{
			Canvas.getPixelMap()[x] = particleColor;
		}
	}
}

void pixelParticle::updateParticles(canvasWithGetPixel Canvas)
{
	//loop through all pixels back to front, update pixel location if it is an active particle
	for (int y = Canvas.getHeight() - 1; y == 0; y--)
	{
		for (int x = Canvas.getWidth() - 1; x == 0; x--)
		{
		}
	}
}

bool pixelParticle::checkIfPixelIsEmpty(rgb_matrix::Color Color)
{
	if (Color.b == 0 && Color.g == 0 && Color.r == 0)
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkDown(int x, int y, canvasWithGetPixel Canvas)
{
	rgb_matrix::Color pixelColor = Canvas.getPixelMap()[(x + (y + 1) * Canvas.getWidth())];
	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkDownRight(int x, int y, canvasWithGetPixel Canvas)
{
	rgb_matrix::Color pixelColor = Canvas.getPixelMap()[((x + 1) + (y + 1) * Canvas.getWidth())];

	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkDownLeft(int x, int y, canvasWithGetPixel Canvas)
{
	rgb_matrix::Color pixelColor = Canvas.getPixelMap()[((x - 1) + (y + 1) * Canvas.getWidth())];

	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}


void pixelParticle::drawParticles(canvasWithGetPixel Canvas)
{
	// call setPixel in a double for loop

	for (int y = 0; y < Canvas.getHeight(); y++)
	{
		for (int x = 0; x < Canvas.getWidth(); x++)
		{
			rgb_matrix::Color pixelColor = Canvas.getPixelMap()[x + (y * Canvas.getWidth())];
			if (checkIfPixelIsEmpty(pixelColor) == false)
			{
				Canvas.SetPixel(x, y, pixelColor.r, pixelColor.g, pixelColor.b);
			}
		}
	}
}

void pixelParticle::setParticleColor(rgb_matrix::Color Color)
{
	particleColor = Color;
}
