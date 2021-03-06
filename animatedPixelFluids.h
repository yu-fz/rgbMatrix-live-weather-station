#pragma once

#include <vector>
#include <graphics.h>
#include "getPixelCanvas.h"
#include <random>

// pixelParticle rain, rain.spawnParticle( int intensity, rbj_matrix::Canvas getPixelcanvas)
class pixelParticle
{
private:


	float initialVelocity;
	std::string particleStringID;
	float deltaT = 0.1;
	rgb_matrix::Color particleColor;

	int randInt;
	int range;

	rgb_matrix::Color black = rgb_matrix::Color(0, 0, 0);

public:

	pixelParticle(int x, std::string y, rgb_matrix::Color z)
	{
		initialVelocity = x;
		particleStringID = y;
		particleColor = z;
	}

	void spawnParticle(int intensity, canvasWithGetPixel Canvas);
	bool checkIfPixelIsEmpty(rgb_matrix::Color Color);
	bool checkDown(int x, int y, canvasWithGetPixel Canvas);
	bool checkDownRight(int x, int y, canvasWithGetPixel Canvas);
	bool checkDownLeft(int x, int y, canvasWithGetPixel Canvas);
	bool checkRight(int x, int y, canvasWithGetPixel Canvas);
	bool checkLeft(int x, int y, canvasWithGetPixel Canvas);
	bool checkIfParticleColorEquiv(rgb_matrix::Color x, rgb_matrix::Color y);
	void drawParticles(canvasWithGetPixel Canvas);
	void updateParticles(canvasWithGetPixel Canvas);

	void setParticleColor(rgb_matrix::Color Color);
};
