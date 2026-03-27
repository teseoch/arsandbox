#pragma once

#include "depth.hpp"
#include "FlowMap.hpp"

#include <cmath>
#include <iostream>

enum class CreatureState
{
	NORMAL = 0,
	PANIC = 1,
	DEAD = 2
};

class Creature
{
public:
	float u, v;
	int life = 10; // frames

	float angle = 0.0f;
	float flip_x = 0.0f;

	CreatureState state = CreatureState::NORMAL;

public:
	bool alive() const { return life > -50; }

	virtual void step(const Depth &hf, const FlowMap &water, const FlowMap &lava, float dt) = 0;
};
