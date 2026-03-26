#pragma once

#include "depth.hpp"

#include <cmath>
#include <iostream>

class Creature
{
public:
	float u, v;
	float h0;
	float dir; // +1 or -1 (cw/ccw)
	float angle = 0.0f;
	float flip_x = 0.0f;

	const float tangential_speed = 0.035f;
	const float contour_gain = 1.4f;

	void step(const Depth &hf, float dt)
	{
		auto [gx, gy] = hf.gradient_uv(u, v);
		float gnorm = std::sqrt(gx * gx + gy * gy) + 1e-6f;

		if (gnorm < 1e-4f)
			return;

		// tangent = rotate(grad) by 90 deg
		float tx = dir * (-gy / gnorm);
		float ty = dir * (gx / gnorm);

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
	}
};
