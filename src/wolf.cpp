#include "wolf.hpp"

#include "audio.hpp"
#include "Biome.hpp"

#include <algorithm>
#include <cstdlib>

Wolf::Wolf() : Creature(6)
{
	search_radius = 0.15f;
	life = 400;
	u = 0.5f;
	v = 0.5f;
	size = 50;
	panic_flicks = false;
}

void Wolf::step(const Depth &hf,
				const Biome &biome,
				const FlowMap &, const FlowMap &lava,
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

	if (biome.type() == BiomeType::Lava)
	{
		chase_speed_mod *= 1.25f;
	}

	life--;
	state = CreatureState::NORMAL;

	if (target && (!target->alive() || target->decaying()))
	{
		target = nullptr;
	}

	float z = hf.sample_bilinear(u, v);

	float hot = lava.sample_bilinear(u, v);
	hot = std::clamp((hot - 0.04f) / 0.18f, 0.0f, 1.0f);

	if (biome.lava_threshold >= 0.0f)
	{
		float terrain_hot = std::clamp((biome.lava_threshold - z) / 0.08f, 0.0f, 1.0f);
		hot = std::max(hot, terrain_hot);
	}

	const bool burning = hot > 0.35f;
	hunting_cooldown -= dt;

	if (burning)
	{
		state = CreatureState::PANIC;
		panic_flicks = true;
		target = nullptr;
		hunting_cooldown = 0.8f;

		lava_cooldown -= dt;
		if (lava_cooldown <= 0.0f)
		{
			life -= 30;
			lava_cooldown = 0.2f;
		}
	}

	if (target)
	{
		state = CreatureState::PANIC;
		panic_flicks = false;

		// Chase prey directly.
		float du = target->u - u;
		float dv = target->v - v;
		float dist2 = du * du + dv * dv;
		float dist = std::sqrt(dist2);

		float speed = chase_speed_mod * 1.3f;

		if (life > 250) // recently ate
			speed *= 0.6f;

		if (dist > 1e-6f)
		{
			float vx = (speed * 1.5f) * du / dist;
			float vy = (speed * 1.5f) * dv / dist;
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
		if (dist2 < 0.00025f)
		{
			target->life = 0;
			life += 150;
			target = nullptr;
			flip_x = 0.0f;
		}
	}
	else if (burning)
	{
		auto [gx, gy] = hf.gradient_uv(u, v);
		float gnorm = std::sqrt(gx * gx + gy * gy);
		if (gnorm < 1e-6f)
			gnorm = 1e-6f;

		float downhill_x = -gx / gnorm;
		float downhill_y = -gy / gnorm;

		float vx = 0.35f * downhill_x;
		float vy = 0.35f * downhill_y;

		u += vx * dt;
		v += vy * dt;

		angle = std::atan2(vy, vx);
		flip_x = (vx < 0.0f) ? 1.0f : 0.0f;
	}
	else
	{
		auto [gx, gy] = hf.gradient_uv(u, v);
		float gnorm = std::sqrt(gx * gx + gy * gy);
		// Wander slowly when not hunting
		float jitter = 0.5f - (std::rand() / float(RAND_MAX));
		angle += jitter * 0.1f;

		float vx = std::cos(angle) * 0.02f;
		float vy = std::sin(angle) * 0.02f;

		if (gnorm > 0.08f)
		{
			float downhill_x = -gx / gnorm;
			float downhill_y = -gy / gnorm;

			vx += 0.005f * downhill_x;
			vy += 0.005f * downhill_y;
		}
		angle = std::atan2(vy, vx);
		flip_x = (vx < 0.0f) ? 1.0f : 0.0f;

		u += vx * dt;
		v += vy * dt;

		// stay in sandbox
		u = std::clamp(u, 0.0f, 1.0f);
		v = std::clamp(v, 0.0f, 1.0f);

		flip_x = (vx < 0.0f) ? 1.0f : 0.0f;
	}

	if (std::rand() / float(RAND_MAX) < 0.000001f)
		life--;
}

bool Wolf::find_wolf_prey() const
{
	return target == nullptr && life < 300 && hunting_cooldown <= 0.0f;
}
