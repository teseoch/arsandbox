#include "eagle.hpp"

#include "audio.hpp"
#include "Biome.hpp"

#include <algorithm>
#include <cstdlib>

Eagle::Eagle() : Creature(3)
{
	search_radius = 0.3f;
	life = 500;
	u = 0.5f;
	v = 0.5f;
	panic_flicks = false;
	reset_orbit_center();
}

void Eagle::reset_orbit_center()
{
	float dx = u - cx;
	float dy = v - cy;
	float dist = std::sqrt(dx * dx + dy * dy);
	if (dist > 1e-6f)
	{
		cx = u - circle_radius * dx / dist;
		cy = v - circle_radius * dy / dist;
	}
	orbit_angle = std::atan2(v - cy, u - cx);
}

void Eagle::step(const Depth &,
				 const Biome &biome,
				 const FlowMap &, const FlowMap &,
				 float dt)
{
	if (life <= 0)
	{
		if (!deadSoundPlayed)
		{
			Audio::instance().play(Sound::Burn, 0.5f);
			deadSoundPlayed = true;
		}

		life--;
		state = CreatureState::DEAD;
		return;
	}

	float chase_speed_mod = chase_speed;
	float orbit_speed_mod = orbit_speed;

	if (biome.type() == BiomeType::Lava)
	{
		chase_speed_mod *= 1.25f;
		orbit_speed_mod *= 1.25f;
	}

	life--;
	state = CreatureState::NORMAL;

	if (target && (!target->alive() || target->decaying()))
	{
		target = nullptr;
		reset_orbit_center();
	}

	if (target)
	{
		state = CreatureState::PANIC;

		// Chase prey directly.
		float du = target->u - u;
		float dv = target->v - v;
		float dist2 = du * du + dv * dv;
		float dist = std::sqrt(dist2);

		if (dist > 1e-6f)
		{
			float vx = chase_speed_mod * du / dist;
			float vy = chase_speed_mod * dv / dist;
			u += vx * dt;
			v += vy * dt;
			angle = std::atan2(vy, vx);
			flip_x = (vx < 0.0f) ? 1.0f : 0.0f;
		}

		// Clamp to sandbox.
		u = std::clamp(u, 0.0f, 1.0f);
		v = std::clamp(v, 0.0f, 1.0f);

		// Kill prey on contact, then smoothly resume a circular trajectory by
		// rebuilding the orbit center from the new position.
		if (dist2 < 0.0025f)
		{
			target->life = 0;
			life += 200;
			target = nullptr;
			flip_x = 0.0f;
			reset_orbit_center();
		}
	}
	else
	{
		// Drift on a circular trajectory around the current center.
		orbit_angle += orbit_speed_mod * dt;
		u = cx + circle_radius * std::cos(orbit_angle);
		v = cy + circle_radius * std::sin(orbit_angle);

		u = std::clamp(u, 0.0f, 1.0f);
		v = std::clamp(v, 0.0f, 1.0f);

		float tx = -std::sin(orbit_angle);
		float ty = std::cos(orbit_angle);
		angle = std::atan2(ty, tx);
		// flip_x = (tx < 0.0f) ? 1.0f : 0.0f;
	}

	if (std::rand() / float(RAND_MAX) < 0.000001f)
		life--;
}

bool Eagle::find_prey() const
{
	return target == nullptr && life < 300;
}
