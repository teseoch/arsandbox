#pragma once

#include "depth.hpp"

#include <cmath>
#include <deque>
#include <functional>
#include <utility>

class FlowMap;

class Drop
{
public:
	float u = 0.0f, v = 0.0f;
	float life = 0.0f; // seconds
	std::deque<Vec2> trail;
	int max_trail = 30;

private:
	float du = 0.0f, dv = 0.0f; // velocity in UV
public:
	void step(const Depth &hf, float dt,
			  float gravity,
			  float damping,
			  float speed_cap,
			  float bounce,
			  const FlowMap &other_flow,
			  float other_repulsion);

	inline bool isAlive() const { return life > 0; }

	inline void reset(float u0, float v0, float life0)
	{
		u = u0;
		v = v0;
		life = life0;
		du = 0.0f;
		dv = 0.0f;
		trail.clear();
		trail.emplace_back(u, v);
	}
};
