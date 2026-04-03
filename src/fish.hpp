#pragma once

#include "creature.hpp"

#include <cmath>
#include <iostream>

class Fish : public Creature
{
public:
	float dir; // +1 or -1 (kept for compatibility, not used much)

	float init_dx = 0.0f;
	float init_dy = 0.0f;

	Fish();

private:
	float swim_speed = 0.020f;
	float flow_follow_gain = 0.050f;
	float shore_avoid_gain = 0.040f;
	float heading_x = 1.0f;
	float heading_y = 0.0f;
	float previous_angle = 0.0f;
	float heading_timer = 0.0f;
	float heading_interval = 1.2f;
	float wet_cooldown = 0.0f;
	float lava_cooldown = 0.0f;
	float init_timer = 0.8f;
	float stuck_timer = 0.0f;
	float display_angle = 0.0f;
	bool is_lava_fish = false;

public:
	void step(const Depth &hf,
			  const Biome &biome,
			  const FlowMap &water, const FlowMap &lava,
			  float dt) override;
};
