#include "pig.hpp"

#include "audio.hpp"

void Pig::step(const Depth &hf, const FlowMap &water, const FlowMap &lava, float dt)
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

	// Pigs should still move on flat terrain, so do not early-return when gnorm is tiny.
	if (gnorm < 1e-6f)
		gnorm = 1e-6f;

	state = CreatureState::NORMAL;
	low_altitude_cooldown = std::max(0.0f, low_altitude_cooldown - dt);

	const float old_u = u;
	const float old_v = v;

	float z = hf.sample_bilinear(u, v);
	(void)z;

	float wet = water.sample_bilinear(u, v);
	wet = std::clamp((wet - 0.08f) / 0.30f, 0.0f, 1.0f);

	float hot = lava.sample_bilinear(u, v);
	hot = std::clamp((hot - 0.04f) / 0.18f, 0.0f, 1.0f);

	// Update cruising heading from time to time, but only on relatively flat ground.
	// This avoids jittery random reorientation while the pig is already fighting a slope.
	heading_timer -= dt;
	if (heading_timer <= 0.0f && gnorm < slope_slow_threshold)
	{
		float a = ((float)std::rand() / float(RAND_MAX)) * 6.2831853f;
		heading_x = std::cos(a);
		heading_y = std::sin(a);
		heading_timer = heading_interval + 1.0f * ((float)std::rand() / float(RAND_MAX));
	}

	// Initial push out of the house/spawner.
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

	float move_x = cruise_speed * heading_x;
	float move_y = cruise_speed * heading_y;

	// Mild slope: pigs slow down and drift slightly downhill.
	if (gnorm > slope_slow_threshold)
	{
		float downhill_x = -gx / gnorm;
		float downhill_y = -gy / gnorm;
		float slope = std::clamp((gnorm - slope_slow_threshold) / std::max(1e-4f, roll_threshold - slope_slow_threshold), 0.0f, 1.0f);
		move_x *= (1.0f - 0.45f * slope);
		move_y *= (1.0f - 0.45f * slope);
		move_x += 0.6f * drift_gain * slope * downhill_x;
		move_y += 0.6f * drift_gain * slope * downhill_y;
	}

	// Strong slope: roll downhill. This is the main pig-specific behavior.
	if (gnorm > roll_threshold)
	{
		state = CreatureState::PANIC;
		float downhill_x = -gx / gnorm;
		float downhill_y = -gy / gnorm;
		float roll = std::clamp((gnorm - roll_threshold) / 0.20f, 0.0f, 1.0f);
		move_x = (0.55f * cruise_speed) * heading_x + (0.65f * roll_speed * (0.25f + 0.75f * roll)) * downhill_x;
		move_y = (0.55f * cruise_speed) * heading_y + (0.65f * roll_speed * (0.25f + 0.75f * roll)) * downhill_y;
		angle += 4.0f * roll * dt; // gentler rolling visual cue
	}
	else
	{
		angle = std::atan2(move_y, move_x);
	}

	// Water: pigs are mildly bothered, but much less than goats.
	if (wet > 0.0f)
	{
		state = CreatureState::PANIC;
		low_altitude_cooldown = low_altitude_cooldown_time;
		float downhill_x = -gx / gnorm;
		float downhill_y = -gy / gnorm;
		move_x += 0.02f * wet * downhill_x;
		move_y += 0.02f * wet * downhill_y;
	}

	// Lava: strong panic and damage over time.
	if (hot > 0.0f)
	{
		state = CreatureState::PANIC;
		low_altitude_cooldown = low_altitude_cooldown_time;
		float downhill_x = -gx / gnorm;
		float downhill_y = -gy / gnorm;
		move_x += 0.06f * hot * downhill_x;
		move_y += 0.06f * hot * downhill_y;

		lava_cooldown -= dt;
		if (hot > 0.75f && lava_cooldown <= 0.0f)
		{
			life--;
			lava_cooldown = 0.2f;
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

	// Sprite facing.
	flip_x = (move_x < 0.0f) ? 1.0f : 0.0f;
	if (gnorm <= roll_threshold)
		angle = std::atan2(move_y, move_x);

	// Kill pigs that genuinely get stuck.
	float du = u - old_u;
	float dv = v - old_v;
	float moved2 = du * du + dv * dv;
	if (moved2 < 1e-7f)
		stuck_timer += dt;
	else
		stuck_timer = 0.0f;

	if (stuck_timer > 5.0f)
	{
		life = 0;
		state = CreatureState::DEAD;
		return;
	}

	if (std::rand() / float(RAND_MAX) < 0.00001f)
		life--;

	if (wet > 0.6f)
	{
		drown_timer += dt;

		if (drown_timer > 2.0f) // ~2 seconds underwater
		{
			life--;
			drown_timer = 0.5f; // slow ticking damage
		}
	}
	else
	{
		drown_timer = std::max(0.0f, drown_timer - 2.0f * dt); // recover quickly
	}
}
