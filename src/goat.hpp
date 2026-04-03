#pragma once

#include "creature.hpp"

#include <cmath>
#include <iostream>

class Goat : public Creature
{
public:
	float h0;
	float dir; // +1 or -1 (cw/ccw)

	float init_dx = 0.0f;
	float init_dy = 0.0f;

	Goat() : Creature(0) {}

private:
	float init_timer = 5.0f; // seconds of initial directed motion

	float stuck_timer = 0.0f;

	float tangential_speed = 0.035f;
	float contour_gain = 1.4f;
	float wash_gain = 0.18f;
	float water_panic_gain = 0.12f;

	float low_altitude_threshold = 0.72f;
	float low_altitude_gain = 0.1f;
	float low_altitude_cooldown = 0.0f;
	float low_altitude_cooldown_time = 1.0f;

	float lava_cooldown = 0.0f;
	float wet_cooldown = 0.0f;

	float ram_speed = 0.11f;
	float ram_hit_radius = 0.045f;
	float ram_cooldown = 0.0f;

public:
	void step(const Depth &hf,
			  const Biome &biome,
			  const FlowMap &water, const FlowMap &lava,
			  float dt) override;

	bool find_ram() const override;
};
