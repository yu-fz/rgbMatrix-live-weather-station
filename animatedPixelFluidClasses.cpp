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
	//rgb_matrix::Color rainColor = rgb_matrix::Color(0, 0, 255); //blue ish color for rain 
	for (int x = 21; x <= 41; x++)
	{
		//do RNG here
		randNum = distr(gen);
		if (randNum <= 10)
		{
			Canvas.SetPixel(x, 0, particleColor.r, particleColor.g, particleColor.b);
		}
	}

	//Canvas.SetPixel(...)
}

void pixelParticle::setParticleColor(rgb_matrix::Color Color)
{
	particleColor = Color;
}
