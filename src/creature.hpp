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
	float h0;
	float dir; // +1 or -1 (cw/ccw)
	float angle = 0.0f;
	float flip_x = 0.0f;
	float init_dx = 0.0f;
	float init_dy = 0.0f;
	float init_timer = 5.0f; // seconds of initial directed motion

	float stuck_timer = 0.0f;

	int life = 10; // frames

	float tangential_speed = 0.035f;
	float contour_gain = 1.4f;
	float wash_gain = 0.18f;
	float water_panic_gain = 0.12f;

	// float panic_cooldown = 0.0f;
	float lava_cooldown = 0.0f;

	CreatureState state = CreatureState::NORMAL;

	bool alive() const { return life > -50; }

	void step(const Depth &hf, const FlowMap &water, const FlowMap &lava, float dt)
	{
		if (life <= 0)
		{
			life--;
			state = CreatureState::DEAD;
			return;
		}
		auto [gx, gy] = hf.gradient_uv(u, v);
		float gnorm = std::sqrt(gx * gx + gy * gy) + 1e-6f;

		if (gnorm < 1e-4f)
			return;

		state = CreatureState::NORMAL;

		const float old_u = u;
		const float old_v = v;

		// tangent = rotate(grad) by 90 deg
		float tx = dir * (-gy / gnorm);
		float ty = dir * (gx / gnorm);

		// initial push from spawn direction
		if (init_timer > 0.0f)
		{
			tx = tx / 4 + init_dx;
			ty = ty / 4 + init_dy;
			init_timer -= dt;
		}

		float fx = tx;
		float fy = ty;

		float nx = gx / gnorm;
		float ny = gy / gnorm;

		if (fy < 0.0f)
		{
			fx = -fx;
			fy = -fy;
			nx = -nx;
			ny = -ny;
		}
		angle = std::atan2(fy, fx);
		// right vector of the rotated sprite
		float rx = fy;
		float ry = -fx;
		flip_x = (rx * nx + ry * ny < 0.0f) ? 0.0f : 1.0f;

		float z = hf.sample_bilinear(u, v);
		float err = (z - h0); // >0 means above target height

		// correction toward isoline: move along -grad if above, +grad if below
		float cx = -(err) * (gx / (gnorm * gnorm));
		float cy = -(err) * (gy / (gnorm * gnorm));

		u += (tangential_speed * tx + contour_gain * cx) * dt;
		v += (tangential_speed * ty + contour_gain * cy) * dt;

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
			dir = -dir;

		// Water makes goats panic a bit and pushes them downhill.
		float wet = water.sample_bilinear(u, v);
		wet = std::clamp((wet - 0.08f) / 0.30f, 0.0f, 1.0f);
		if (wet > 0.0f)
		{
			state = CreatureState::PANIC;
			// Add a little random-ish panic by biasing direction away from the current
			// contour motion and drift slightly downhill.
			float downhill_x = -gx / gnorm;
			float downhill_y = -gy / gnorm;
			float speed_scale = 1.0f + 0.8f * wet;
			float contour_scale = 1.0f - 0.7f * wet;

			u += (speed_scale * tangential_speed * tx + contour_scale * contour_gain * cx) * dt;
			v += (speed_scale * tangential_speed * ty + contour_scale * contour_gain * cy) * dt;
			// panic_cooldown -= dt;

			// Strong water makes the goat more likely to flip contour direction.
			// if (wet > 0.6f && panic_cooldown <= 0.0f && std::rand() / float(RAND_MAX) < 0.4f)
			// {
			// 	dir = -dir;
			// 	panic_cooldown = 0.5f; // half a second
			// }
		}

		// Lava is a much stronger hazard than water, but keep the response smooth:
		// modify the motion instead of applying a second strong movement step.
		float hot = lava.sample_bilinear(u, v);
		hot = std::clamp((hot - 0.04f) / 0.18f, 0.0f, 1.0f);
		if (hot > 0.0f)
		{
			state = CreatureState::PANIC;

			float downhill_x = -gx / gnorm;
			float downhill_y = -gy / gnorm;
			float speed_scale = 1.0f + 0.35f * hot;
			float contour_scale = 1.0f - 0.45f * hot;

			u += (speed_scale * tangential_speed * tx + contour_scale * contour_gain * cx) * dt;
			v += (speed_scale * tangential_speed * ty + contour_scale * contour_gain * cy) * dt;
			u += (0.02f * hot) * downhill_x * dt;
			v += (0.02f * hot) * downhill_y * dt;

			lava_cooldown -= dt;
			if (hot > 0.75f && lava_cooldown <= 0.0f)
			{
				life--;
				lava_cooldown = 0.15f;
			}
		}

		float du = u - old_u;
		float dv = v - old_v;
		float moved2 = du * du + dv * dv;

		if (moved2 < 1e-8f)
			stuck_timer += dt;
		else
			stuck_timer = 0.0f;

		if (stuck_timer > 1.5f)
		{
			life = 0;
			state = CreatureState::DEAD;
			return;
		}
	}
};
