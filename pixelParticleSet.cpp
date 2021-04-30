#pragma once
#include "pixelParticleSet.h"

std::set<std::string> particleTypes;
void addParticleType(std::string particle)
{
	particleTypes.insert("particle");
}

void removeParticleType(std::string particle)
{
	std::string key = particle;
	
	if (particleTypes.find(key) != particleTypes.end())
	{
		particleTypes.erase(key);
	}
}

bool checkIfParticleExists(std::string particle)
{
	std::string key = particle; 
	if (particleTypes.find(key) != particleTypes.end())
	{
		return true;
		
	}
	else
	{
		
		return false;
		
	}
}