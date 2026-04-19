#include "frog.hpp"

#include "audio.hpp"
#include "Biome.hpp"

#include <algorithm>
#include <cstdlib>

void Frog::step(const Depth &hf,
				const Biome &biome,
				const FlowMap &water, const FlowMap &lava,
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

	state = CreatureState::NORMAL;

	float z = hf.sample_bilinear(u, v);
	auto [gx, gy] = hf.gradient_uv(u, v);
	float gnorm = std::sqrt(gx * gx + gy * gy);
	if (gnorm < 1e-6f)
		gnorm = 1e-6f;

	float wet = water.sample_bilinear(u, v);
	wet = std::clamp((wet - 0.08f) / 0.30f, 0.0f, 1.0f);

	float hot = lava.sample_bilinear(u, v);
	hot = std::clamp((hot - 0.04f) / 0.18f, 0.0f, 1.0f);

	// Terrain-defined biome hazards.
	if (biome.water_threshold >= 0.0f)
	{
		float terrain_wet = std::clamp((biome.water_threshold - z) / 0.10f, 0.0f, 1.0f);
		wet = std::max(wet, terrain_wet);
	}
	if (biome.lava_threshold >= 0.0f)
	{
		float terrain_hot = std::clamp((biome.lava_threshold - z) / 0.08f, 0.0f, 1.0f);
		hot = std::max(hot, terrain_hot);
	}

	float downhill_x = -gx / gnorm;
	float downhill_y = -gy / gnorm;

	// Frogs like wet places, but survive on land for a while.
	wet_cooldown -= dt;
	if (wet < 0.18f)
	{
		state = CreatureState::PANIC;
		if (wet_cooldown <= 0.0f)
		{
			life--;
			wet_cooldown = 1.1f;
		}
	}

	// Lava is very bad for frogs.
	lava_cooldown -= dt;
	if (hot > 0.25f)
	{
		state = CreatureState::PANIC;
		if (lava_cooldown <= 0.0f)
		{
			life--;
			lava_cooldown = 0.30f;
		}
	}

	// Between jumps, frogs keep a little leftover momentum so they land rather
	// than teleport.
	u += vx * dt;
	v += vy * dt;
	vx *= landing_damp;
	vy *= landing_damp;

	jump_timer -= dt;
	if (jump_timer <= 0.0f)
	{
		float jx = 0.0f;
		float jy = 0.0f;

		if (hot > 0.25f)
		{
			// Escape lava by jumping downhill / away from danger.
			jx = downhill_x;
			jy = downhill_y;
		}
		else if (wet < 0.18f)
		{
			// Out of water, try to reach lower / wetter places.
			jx = downhill_x;
			jy = downhill_y;
		}
		else if (wet > 0.35f)
		{
			// In nice wet areas, hop with a tangent bias so frogs do not look too
			// deterministic.
			float tx = -downhill_y;
			float ty = downhill_x;
			float s = (std::rand() % 2 == 0) ? 1.0f : -1.0f;
			jx = 0.55f * tx * s + 0.45f * downhill_x;
			jy = 0.55f * ty * s + 0.45f * downhill_y;
		}
		else
		{
			// Mildly random hop when not in strong conditions.
			float a = ((float)std::rand() / float(RAND_MAX)) * 6.2831853f;
			jx = std::cos(a);
			jy = std::sin(a);
		}

		float n = std::sqrt(jx * jx + jy * jy);
		if (n > 1e-6f)
		{
			jx /= n;
			jy /= n;
		}

		vx += jump_speed * jx;
		vy += jump_speed * jy;
		angle = std::atan2(jy, jx);
		flip_x = (jx < 0.0f) ? 1.0f : 0.0f;

		jump_timer = jump_cooldown_min + (jump_cooldown_max - jump_cooldown_min) * ((float)std::rand() / float(RAND_MAX));
	}

	u = std::clamp(u, 0.0f, 1.0f);
	v = std::clamp(v, 0.0f, 1.0f);

	applyOldAge(dt, 120.0f);
}
