#pragma once

#include "creature.hpp"

#include <cmath>
#include <iostream>

class Eagle : public Creature
{
public:
	float cx = 0.5f, cy = 0.5f;
	float orbit_angle = 0.0f;

	float circle_radius = 0.18f;
	float orbit_speed = 0.6f;
	float chase_speed = 0.2f;

	Eagle();
	void reset_orbit_center();

private:
public:
	void step(const Depth &hf,
			  const Biome &biome,
			  const FlowMap &water, const FlowMap &lava,
			  float dt) override;

	bool find_prey() const override;
};
