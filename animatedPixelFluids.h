#pragma once

#include <vector>
#include <graphics.h>
#include "getPixelCanvas.h"
#include <random>

// pixelParticle rain, rain.spawnParticle( int intensity, rbj_matrix::Canvas getPixelcanvas)
class pixelParticle
{
	
	
private:

	int initialVelocity;
	std::string particleStringID;
	float deltaT = 0.05;
	rgb_matrix::Color particleColor;
	rgb_matrix::Color rainColor;
	time_t timeNow = time(nullptr);
	int randInt;
	int range;
	float distance;
	int pixelDistance = 1;
	rgb_matrix::Color black = rgb_matrix::Color(0, 0, 0);

public:

	pixelParticle(int x, std::string y, rgb_matrix::Color z)
	{
		initialVelocity = x;
		particleStringID = y;
		particleColor = z;
		if (particleStringID == "rain")
		{
			rainColor = z;
		}
	}

	void spawnParticle(int intensity, canvasWithGetPixel Canvas);
	void setParticleVelocity(int velocity);
	static bool checkIfPixelIsEmpty(rgb_matrix::Color Color);
	bool checkDown(int x, int y, canvasWithGetPixel Canvas);
	bool checkDownRight(int x, int y, canvasWithGetPixel Canvas);
	bool checkDownLeft(int x, int y, canvasWithGetPixel Canvas);
	bool checkRight(int x, int y, canvasWithGetPixel Canvas);
	bool checkLeft(int x, int y, canvasWithGetPixel Canvas);
	bool checkUp(int x, int y, canvasWithGetPixel Canvas);
	bool checkIfParticleColorEquiv(rgb_matrix::Color x, rgb_matrix::Color y);
	void freezeWaterParticles(canvasWithGetPixel Canvas);
	static void drawParticles(canvasWithGetPixel Canvas);
	void updateParticles(canvasWithGetPixel Canvas);
	void setParticleColor(rgb_matrix::Color Color);
	int calculateFallDistance();
};
