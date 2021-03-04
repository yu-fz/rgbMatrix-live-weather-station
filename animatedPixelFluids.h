#pragma once

#include <vector>
#include <graphics.h>
#include "getPixelCanvas.h"
#include <random>

// pixelParticle rain, rain.spawnParticle( int intensity, rbj_matrix::Canvas getPixelcanvas)
class pixelParticle
{
private:


	//float initialVelocity;

	//float deltaT;

	//std::vector<float> position;

	rgb_matrix::Color particleColor;
	std::vector<int> spawnPoints;

public:

	void spawnParticle(int intensity, canvasWithGetPixel Canvas);

	void setParticleColor(rgb_matrix::Color Color);
};
