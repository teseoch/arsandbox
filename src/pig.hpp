#pragma once

#include "creature.hpp"

#include <cmath>
#include <iostream>

class Pig : public Creature
{
public:
	float dir; // +1 or -1 (kept for compatibility, not used much)

	float init_dx = 0.0f;
	float init_dy = 0.0f;

	Pig() : Creature(1) {}

private:
	float init_timer = 0.8f; // initial push out of the house
	float stuck_timer = 0.0f;

	float drown_timer = 0.0f;

	// Pig-specific movement: likes plains, dislikes slopes.
	float cruise_speed = 0.018f;
	float drift_gain = 0.020f;
	float slope_slow_threshold = 0.08f;
	float roll_threshold = 0.22f;
	float roll_speed = 0.11f;
	float heading_x = 1.0f;
	float heading_y = 0.0f;
	float heading_timer = 0.0f;
	float heading_interval = 1.2f;

	float low_altitude_cooldown = 0.0f;
	float low_altitude_cooldown_time = 1.0f;
	float lava_cooldown = 0.0f;

public:
	void step(const Depth &hf, const FlowMap &water, const FlowMap &lava, float dt) override;
};
