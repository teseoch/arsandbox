#include "drop.hpp"

#include "FlowMap.hpp"

void Drop::step(const Depth &hf, float dt,
				float gravity,
				float damping,
				float speed_cap,
				float bounce,
				const FlowMap &other_flow,
				float other_repulsion)
{
	if (life <= 0.0f)
		return;

	auto [gx, gy] = hf.gradient_uv(u, v);
	float g = std::sqrt(gx * gx + gy * gy);

	auto [ogx, ogy] = other_flow.gradient_uv(u, v);

	if (g < 0.03f)
	{
		gx = 0.0f;
		gy = 0.0f;
		g = 0.0f;
	}

	du += -gravity * gx * dt;
	dv += -gravity * gy * dt;

	// Repel this liquid from the other flow-map so water tends to go around
	// lava and lava tends to go around water.
	du += -other_repulsion * ogx * dt;
	dv += -other_repulsion * ogy * dt;

	du *= std::pow(damping, dt * 60.0f);
	dv *= std::pow(damping, dt * 60.0f);

	float sp = std::sqrt(du * du + dv * dv);
	if (sp > speed_cap)
	{
		du *= speed_cap / sp;
		dv *= speed_cap / sp;
	}

	u += du * dt;
	v += dv * dt;

	// bounce / clamp at boundaries
	if (u < 0)
	{
		u = 0;
		du *= bounce;
	}
	if (u > 1)
	{
		u = 1;
		du *= bounce;
	}
	if (v < 0)
	{
		v = 0;
		dv *= bounce;
	}
	if (v > 1)
	{
		v = 1;
		dv *= bounce;
	}

	trail.emplace_back(u, v);
	while ((int)trail.size() > max_trail)
		trail.pop_front();

	life -= dt;
	if (life <= 0.0f)
		trail.clear();
}