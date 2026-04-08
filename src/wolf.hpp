#pragma once

#include "creature.hpp"

#include <cmath>
#include <iostream>

class Wolf : public Creature
{
public:
	float chase_speed = 0.1f;

	Wolf();

private:
	float lava_cooldown = 0.0f;
	float hunting_cooldown = 0.0f;

public:
	void step(const Depth &hf,
			  const Biome &biome,
			  const FlowMap &water, const FlowMap &lava,
			  float dt) override;

	bool find_wolf_prey() const override;
};
