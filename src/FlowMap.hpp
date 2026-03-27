#pragma once

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

#include "drop.hpp"

class FlowMap
{
public:
	int w = 0, h = 0;
	std::vector<float> flow;

	void resize(int w_, int h_)
	{
		w = w_;
		h = h_;
		flow.assign(w * h, 0.0f);
	}

	void clear() { std::fill(flow.begin(), flow.end(), 0.0f); }

	void decay(float factor)
	{
		for (float &v : flow)
			v *= factor;
	}

	void splat(float u, float v, float amount)
	{
		if (w <= 0 || h <= 0)
			return;

		int cx = std::clamp(int(u * float(w - 1)), 0, w - 1);
		int cy = std::clamp(int(v * float(h - 1)), 0, h - 1);

		static const float K[3][3] = {
			{1.0f, 2.0f, 1.0f},
			{2.0f, 4.0f, 2.0f},
			{1.0f, 2.0f, 1.0f},
		};

		for (int j = -1; j <= 1; ++j)
		{
			for (int i = -1; i <= 1; ++i)
			{
				int x = std::clamp(cx + i, 0, w - 1);
				int y = std::clamp(cy + j, 0, h - 1);
				flow[y * w + x] += amount * (K[j + 1][i + 1] / 16.0f);
			}
		}
	}

	void diffuse_once(float center_weight = 0.86f, float neighbor_weight = 0.035f)
	{
		if (w <= 2 || h <= 2)
			return;
		std::vector<float> tmp = flow;
		for (int y = 1; y < h - 1; ++y)
		{
			for (int x = 1; x < w - 1; ++x)
			{
				float c = tmp[y * w + x];
				float n = tmp[(y - 1) * w + x];
				float s = tmp[(y + 1) * w + x];
				float e = tmp[y * w + (x + 1)];
				float wv = tmp[y * w + (x - 1)];
				flow[y * w + x] = center_weight * c + neighbor_weight * (n + s + e + wv);
			}
		}
	}

	void flowDrops(const std::vector<Drop> &drops, float amount = 5.0f, float min_speed = 0.0005f)
	{
		for (const auto &d : drops)
		{
			float sp = 0.0f;
			if (d.trail.size() >= 2)
			{
				const auto &[u0, v0] = d.trail[d.trail.size() - 2];
				const auto &[u1, v1] = d.trail[d.trail.size() - 1];
				float du = u1 - u0;
				float dv = v1 - v0;
				sp = std::sqrt(du * du + dv * dv);
			}

			if (sp > min_speed)
			{
				const auto &[u, v] = d.trail.empty() ? Vec2{d.u, d.v} : d.trail.back();
				splat(u, v, amount);
			}
		}
	}

	float sample_bilinear(float u, float v) const
	{
		if (w <= 0 || h <= 0 || flow.empty())
			return 0.0f;

		u = std::clamp(u, 0.0f, 1.0f);
		v = std::clamp(v, 0.0f, 1.0f);

		float x = u * float(w - 1);
		float y = v * float(h - 1);

		int x0 = int(x);
		int y0 = int(y);
		int x1 = std::min(x0 + 1, w - 1);
		int y1 = std::min(y0 + 1, h - 1);

		float tx = x - float(x0);
		float ty = y - float(y0);

		float f00 = flow[y0 * w + x0];
		float f10 = flow[y0 * w + x1];
		float f01 = flow[y1 * w + x0];
		float f11 = flow[y1 * w + x1];

		float f0 = f00 * (1.0f - tx) + f10 * tx;
		float f1 = f01 * (1.0f - tx) + f11 * tx;
		return f0 * (1.0f - ty) + f1 * ty;
	}

	std::pair<float, float> gradient_uv(float u, float v) const
	{
		if (w <= 1 || h <= 1 || flow.empty())
			return {0.0f, 0.0f};

		const float du = 1.0f / std::max(1, w - 1);
		const float dv = 1.0f / std::max(1, h - 1);

		float fx1 = sample_bilinear(u + du, v);
		float fx0 = sample_bilinear(u - du, v);
		float fy1 = sample_bilinear(u, v + dv);
		float fy0 = sample_bilinear(u, v - dv);

		return {(fx1 - fx0) / (2.0f * du), (fy1 - fy0) / (2.0f * dv)};
	}
};
