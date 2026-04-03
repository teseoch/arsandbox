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

class Biome;

class Creature
{
public:
	float u, v;
	int life = 10; // frames

	float angle = 0.0f;
	float flip_x = 0.0f;

	float search_radius = 0;

	bool panic_flicks = true;
	float size = 30;

	std::shared_ptr<Creature> target;

	CreatureState state = CreatureState::NORMAL;

	Creature(int texture_index_) : texture_index(texture_index_) {}

public:
	bool alive() const { return life > -50; }
	bool decaying() const { return life <= 0; }

	virtual bool find_ram() const { return false; }
	virtual bool find_prey() const { return false; }
	virtual bool find_corpse() const { return false; }

	float textureIndex() const { return texture_index; }

	virtual void step(
		const Depth &hf,
		const Biome &biome,
		const FlowMap &water, const FlowMap &lava,
		float dt) = 0;

private:
	const int texture_index;

protected:
	bool deadSoundPlayed = false;
};
