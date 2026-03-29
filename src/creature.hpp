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

	Creature(int texture_index_) : texture_index(texture_index_) {}

public:
	bool alive() const { return life > -50; }

	float textureIndex() const { return texture_index; }

	virtual void step(const Depth &hf, const FlowMap &water, const FlowMap &lava, float dt) = 0;

private:
	const int texture_index;

protected:
	bool deadSoundPlayed = false;
};
