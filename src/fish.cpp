#include "fish.hpp"

#include "audio.hpp"
#include "Biome.hpp"

#include <algorithm>
#include <cstdlib>

void Fish::step(const Depth &hf,
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

	auto [gx, gy] = hf.gradient_uv(u, v);
	float gnorm = std::sqrt(gx * gx + gy * gy);
	if (gnorm < 1e-6f)
		gnorm = 1e-6f;

	state = CreatureState::NORMAL;
	const float old_u = u;
	const float old_v = v;

	float z = hf.sample_bilinear(u, v);
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

	// Update heading sometimes so fish do not all swim in straight lines forever.
	heading_timer -= dt;
	if (heading_timer <= 0.0f)
	{
		float a = ((float)std::rand() / float(RAND_MAX)) * 6.2831853f;
		heading_x = std::cos(a);
		heading_y = std::sin(a);
		heading_timer = heading_interval + 0.8f * ((float)std::rand() / float(RAND_MAX));
	}

	// Initial push away from the spawner.
	if (init_timer > 0.0f)
	{
		heading_x = 0.25f * heading_x + 0.75f * init_dx;
		heading_y = 0.25f * heading_y + 0.75f * init_dy;
		float n = std::sqrt(heading_x * heading_x + heading_y * heading_y);
		if (n > 1e-6f)
		{
			heading_x /= n;
			heading_y /= n;
		}
		init_timer -= dt;
	}

	float move_x = swim_speed * heading_x;
	float move_y = swim_speed * heading_y;

	// Follow water downhill / along channels a bit, but only when clearly in water.
	float downhill_x = -gx / gnorm;
	float downhill_y = -gy / gnorm;
	if (wet > 0.30f)
	{
		move_x += flow_follow_gain * wet * downhill_x;
		move_y += flow_follow_gain * wet * downhill_y;
	}

	// Out of water, fish panic and try to get back to lower/wetter regions.
	wet_cooldown -= dt;
	if (wet < 0.25f)
	{
		state = CreatureState::PANIC;
		move_x += shore_avoid_gain * downhill_x;
		move_y += shore_avoid_gain * downhill_y;

		if (wet_cooldown <= 0.0f)
		{
			life--;
			wet_cooldown = 0.8f;
		}
	}

	// Lava is very bad for fish.
	lava_cooldown -= dt;
	if (hot > 0.30f)
	{
		state = CreatureState::PANIC;
		move_x += 0.08f * hot * downhill_x;
		move_y += 0.08f * hot * downhill_y;

		if (lava_cooldown <= 0.0f)
		{
			life--;
			lava_cooldown = 0.25f;
		}
	}

	u += move_x * dt;
	v += move_y * dt;

	bool hit_wall = false;
	if (u < 0.0f)
	{
		u = 0.0f;
		hit_wall = true;
	}
	else if (u > 1.0f)
	{
		u = 1.0f;
		hit_wall = true;
	}

	if (v < 0.0f)
	{
		v = 0.0f;
		hit_wall = true;
	}
	else if (v > 1.0f)
	{
		v = 1.0f;
		hit_wall = true;
	}

	if (hit_wall)
	{
		heading_x = -heading_x;
		heading_y = -heading_y;
		dir = -dir;
	}

	flip_x = (move_x < 0.0f) ? 1.0f : 0.0f;
	float target_angle = std::atan2(move_y, move_x);
	display_angle = 0.85f * display_angle + 0.15f * target_angle;
	angle = display_angle;

	float du = u - old_u;
	float dv = v - old_v;
	float moved2 = du * du + dv * dv;
	if (moved2 < 1e-7f)
		stuck_timer += dt;
	else
		stuck_timer = 0.0f;

	if (stuck_timer > 6.0f)
	{
		life = 0;
		state = CreatureState::DEAD;
		return;
	}
}
