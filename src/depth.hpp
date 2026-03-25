#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

#include "types.hpp"

class Depth
{
public:
	int w, h;
	std::vector<float> depth; // millimeters, size w*h
	float depthMinMm, depthMaxMm;

	std::vector<uint8_t> rgb; // rgb
	Quad uv_quad;

	inline std::pair<float, float> warp_uv(float u, float v) const
	{
		u = std::clamp(u, 0.0f, 1.0f);
		v = std::clamp(v, 0.0f, 1.0f);

		const float ax = uv_quad.v[0].x * (1.0f - u) + uv_quad.v[1].x * u;
		const float ay = uv_quad.v[0].y * (1.0f - u) + uv_quad.v[1].y * u;
		const float bx = uv_quad.v[3].x * (1.0f - u) + uv_quad.v[2].x * u;
		const float by = uv_quad.v[3].y * (1.0f - u) + uv_quad.v[2].y * u;

		const float uu = ax * (1.0f - v) + bx * v;
		const float vv = ay * (1.0f - v) + by * v;
		return {uu, vv};
	}

	std::pair<float, float> inverse_warp_uv(float u_img, float v_img) const
	{
		std::pair<float, float> p{u_img, v_img}; // initial guess

		for (int it = 0; it < 20; ++it)
		{
			std::pair<float, float> q = warp_uv(p.first, p.second); // forward map guess -> image uv

			float ex = q.first - u_img;
			float ey = q.second - v_img;

			if (std::abs(ex) + std::abs(ey) < 1e-6f)
				break;

			const float eps = 1e-3f;
			std::pair<float, float> qx = warp_uv(std::clamp(p.first + eps, 0.0f, 1.0f), p.second);
			std::pair<float, float> qy = warp_uv(p.first, std::clamp(p.second + eps, 0.0f, 1.0f));

			float j00 = (qx.first - q.first) / eps;
			float j10 = (qx.second - q.second) / eps;
			float j01 = (qy.first - q.first) / eps;
			float j11 = (qy.second - q.second) / eps;

			float det = j00 * j11 - j01 * j10;
			if (std::abs(det) < 1e-10f)
				break;

			float dx = (-j11 * ex + j01 * ey) / det;
			float dy = (j10 * ex - j00 * ey) / det;

			p.first = std::clamp(p.first + dx, 0.0f, 1.0f);
			p.second = std::clamp(p.second + dy, 0.0f, 1.0f);
		}

		return p;
	}

	inline float height(int xi, int yi) const
	{
		const float d = depth[yi * w + xi];
		const float t =
			std::clamp((d - depthMinMm) / (depthMaxMm - depthMinMm), 0.0f, 1.0f);
		return 1.0f - t;
	}

	inline float sample_bilinear(float u, float v) const
	{
		auto [uu, vv] = warp_uv(u, v);
		float x = uu * (w - 1);
		float y = vv * (h - 1);
		int x0 = (int)std::floor(x), y0 = (int)std::floor(y);
		int x1 = std::min(x0 + 1, w - 1);
		int y1 = std::min(y0 + 1, h - 1);
		float tx = x - x0, ty = y - y0;

		// std::cout << "sample_bilinear at u=" << u << " v=" << v << " => x=" << x
		//           << " y=" << y << " x0=" << x0 << " y0=" << y0 << " x1=" << x1
		//           << " y1=" << y1 << " tx=" << tx << " ty=" << ty << "\n";

		float z00 = height(x0, y0);
		float z10 = height(x1, y0);
		float z01 = height(x0, y1);
		float z11 = height(x1, y1);

		// std::cout << "  corners: z00=" << z00 << " z10=" << z10 << " z01=" << z01
		//           << " z11=" << z11 << "\n";

		float z0 = z00 * (1 - tx) + z10 * tx;
		float z1 = z01 * (1 - tx) + z11 * tx;
		return z0 * (1 - ty) + z1 * ty;
	}

	inline std::pair<float, float> gradient_uv(float u, float v) const
	{
		// finite differences in sandbox UV
		const float du = 1.0f / std::max(1, w - 1);
		const float dv = 1.0f / std::max(1, h - 1);

		float zx1 = sample_bilinear(u + du, v);
		float zx0 = sample_bilinear(u - du, v);
		float zy1 = sample_bilinear(u, v + dv);
		float zy0 = sample_bilinear(u, v - dv);

		float gx = (zx1 - zx0) / (2.0f * du);
		float gy = (zy1 - zy0) / (2.0f * dv);
		return {gx, gy}; // ∂z/∂u, ∂z/∂v in sandbox space
	}

	void save_png(const std::string &filename) const;

	void blur()
	{
		std::vector<float> tmp = depth;

		for (int y = 1; y < h - 1; ++y)
		{
			for (int x = 1; x < w - 1; ++x)
			{

				float sum = 0.0f;

				for (int j = -1; j <= 1; ++j)
					for (int i = -1; i <= 1; ++i)
						sum += tmp[(y + j) * w + (x + i)];

				depth[y * w + x] = sum / 9.0f;
			}
		}
	}
};
