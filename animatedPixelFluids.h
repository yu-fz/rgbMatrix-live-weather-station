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
	//std::string particleStringID;
	float deltaT = 0.1;
	rgb_matrix::Color particleColor;

public:

	pixelParticle(int x, rgb_matrix::Color z)
	{
		initialVelocity = x;
		//particleStringID = y;
		particleColor = z;
	}

	void spawnParticle(int intensity, canvasWithGetPixel Canvas);
	bool checkIfPixelIsEmpty(rgb_matrix::Color Color);
	void drawParticles(canvasWithGetPixel Canvas);
	void updateParticles(canvasWithGetPixel Canvas);

	void setParticleColor(rgb_matrix::Color Color);
};
