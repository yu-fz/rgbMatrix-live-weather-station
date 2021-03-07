#include <iostream>

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
	range = intensity;
	for (int x = 21; x <= 41; x++)
	{
		//Canvas.getPixelMap()[x] = black;
		//do RNG here
		randNum = distr(gen);
		randInt = randNum;
		if (randNum <= 10)
		{
			Canvas.getPixelMap()[x] = particleColor;
		}
	}
}

void pixelParticle::updateParticles(canvasWithGetPixel Canvas)
{
	time_t seed = time(nullptr); // obtain a random number from hardware
	std::mt19937 gen(seed); // seed the generator
	std::uniform_int_distribution<> distr(1, 250); // define the range
	int randNum;
	//loop through all pixels back to front, update pixel location if it is an active particle
	for (int y = Canvas.getHeight() - 1; y >= 0; y--)
	{
		for (int x = Canvas.getWidth() - 1; x >= 0; x--)
		{
			if ((y < Canvas.getHeight() - 1) && (checkIfParticleColorEquiv(Canvas.getPixel(x, y), particleColor)))
				//handles falling particles
			{
				//check down, downLeft and downRight
				if (checkDown(x, y, Canvas) == false)
				{
					//move down

					for (int j = y; j >= 0; j--)
					{
						Canvas.getPixelMap()[x + (j * Canvas.getWidth())] = black;
					}

					Canvas.getPixelMap()[x + ((y + 1) * Canvas.getWidth())] = particleColor;
				}
				else if ((x < Canvas.getWidth()) && checkDownRight(x, y, Canvas) == false)
				{
					for (int j = y; j >= 0; j--)
					{
						Canvas.getPixelMap()[x + (j * Canvas.getWidth())] = black;
					}

					Canvas.getPixelMap()[(x + 1) + ((y + 1) * Canvas.getWidth())] = particleColor;
				}
				else if ((x > 0) && checkDownLeft(x, y, Canvas) == false)
				{
					for (int j = y; j >= 0; j--)
					{
						Canvas.getPixelMap()[x + (j * Canvas.getWidth())] = black;
					}

					Canvas.getPixelMap()[(x - 1) + ((y + 1) * Canvas.getWidth())] = particleColor;
					//fprintf(stderr, "nuttt");
				}


				else if ((x < Canvas.getWidth()) && (checkRight(x, y, Canvas) == false) && (particleStringID == "rain")
					&& (randInt < range / 2))
				{
					for (int j = y; j >= 0; j--)
					{
						Canvas.getPixelMap()[x + (j * Canvas.getWidth())] = black;
					}

					for (int k = x; k < Canvas.getWidth(); k++)
					{
						if (checkIfPixelIsEmpty(Canvas.getPixel(k, y + 1)))
						{
							Canvas.getPixelMap()[k + ((y + 1) * Canvas.getWidth())] = particleColor;
							break;
						}
					}
				}

				else if ((x > 0) && checkLeft(x, y, Canvas) == false && (particleStringID == "rain"))
				{
					for (int j = y; j >= 0; j--)
					{
						Canvas.getPixelMap()[x + (j * Canvas.getWidth())] = black;
					}

					for (int k = x; k >= 0; k--)
					{
						if (checkIfPixelIsEmpty(Canvas.getPixel(k, y + 1)))
						{
							Canvas.getPixelMap()[k + ((y + 1) * Canvas.getWidth())] = particleColor;
							break;
						}
					}
				}
			}

			if ((y == Canvas.getHeight() - 1) && (checkIfParticleColorEquiv(Canvas.getPixel(x, y), particleColor))
				&& (
					checkUp(x, y, Canvas) == false)) //handles evaporating particles 
			{
				randNum = distr(gen);
				if (randNum <= 15)
				{
					Canvas.getPixelMap()[x + (y * Canvas.getWidth())] = black;
				}
			}
			else if ((checkIfParticleColorEquiv(Canvas.getPixel(x, y), particleColor))
				&& (
					checkUp(x, y, Canvas) == false) && (checkDown(x, y, Canvas) == true))
			{
				randNum = distr(gen);
				if (randNum <= 15)
				{
					Canvas.getPixelMap()[x + (y * Canvas.getWidth())] = black;
				}
			}
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
	rgb_matrix::Color pixelColor = Canvas.getPixel(x, y + 1);
	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkIfParticleColorEquiv(rgb_matrix::Color x, rgb_matrix::Color y)
{
	if ((x.r == y.r) && (x.g == y.g) && (x.b == y.b))
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkDownRight(int x, int y, canvasWithGetPixel Canvas)
{
	rgb_matrix::Color pixelColor = Canvas.getPixel(x + 1, y + 1);

	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkDownLeft(int x, int y, canvasWithGetPixel Canvas)
{
	rgb_matrix::Color pixelColor = Canvas.getPixel(x - 1, y + 1);

	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkLeft(int x, int y, canvasWithGetPixel Canvas)
{
	rgb_matrix::Color pixelColor = Canvas.getPixel(x - 1, y);

	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkRight(int x, int y, canvasWithGetPixel Canvas)
{
	rgb_matrix::Color pixelColor = Canvas.getPixel(x + 1, y);

	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}

bool pixelParticle::checkUp(int x, int y, canvasWithGetPixel Canvas)
{
	rgb_matrix::Color pixelColor = Canvas.getPixel(x, y - 1);

	if (pixelColor.r != 0 || pixelColor.g != 0 || pixelColor.b != 0)
	{
		return true;
	}

	return false;
}


void pixelParticle::evaporateParticles(canvasWithGetPixel Canvas)
{
	//loop through the canvas back to front, use RNG to decide whether or not to delete active particle
	//particle can only be evaporated if the particle is on the ground or above another particle and there is no particle above it.

	time_t seed = time(nullptr); // obtain a random number from hardware
	std::mt19937 gen(seed); // seed the generator
	std::uniform_int_distribution<> distr(1, 250); // define the range
	int randNum;
	//range = intensity;
	for (int y = Canvas.getHeight() - 1; y >= 0; y--)
	{
		for (int x = 0; x < Canvas.getWidth(); x++)
		{
			if ((y == Canvas.getHeight() - 1) && (checkIfParticleColorEquiv(Canvas.getPixel(x, y), particleColor))
				&& (
					checkUp(x, y, Canvas) == false)) //handles evaporating particles 
			{
				randNum = distr(gen);
				if (randNum <= 50)
				{
					Canvas.getPixelMap()[x + (y * Canvas.getWidth())] = black;
				}
			}
			else if ((checkIfParticleColorEquiv(Canvas.getPixel(x, y), particleColor))
				&& (
					checkUp(x, y, Canvas) == false) && (checkDown(x, y, Canvas) == true))
			{
				randNum = distr(gen);
				if (randNum <= 50)
				{
					Canvas.getPixelMap()[x + (y * Canvas.getWidth())] = black;
				}
			}
		}
	}
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
