#pragma once

#include "creature.hpp"

#include <cmath>
#include <iostream>

class Frog : public Creature
{
public:
	Frog() : Creature(5) {}

private:
	float jump_timer = 0.6f;
	float jump_cooldown_min = 0.5f;
	float jump_cooldown_max = 1.4f;
	float jump_speed = 0.18f;
	float wet_cooldown = 0.0f;
	float lava_cooldown = 0.0f;
	float vx = 0.0f;
	float vy = 0.0f;
	float landing_damp = 0.88f;

public:
	void step(const Depth &hf,
			  const Biome &biome,
			  const FlowMap &water, const FlowMap &lava,
			  float dt) override;
};
