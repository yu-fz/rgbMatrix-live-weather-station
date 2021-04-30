#pragma once
#include <string>
#include <set>

//std::set<std::string> particleTypes;

void addParticleType(std::string particle);

void removeParticleType(std::string particle);

bool checkIfParticleExists(std::string particle);
