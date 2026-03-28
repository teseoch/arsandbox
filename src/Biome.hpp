#pragma once

#include <string>
#include <array>

#include "CMap.hpp"

class Biome
{
public:
	virtual ~Biome() = default;

	const GLuint &texture() const { return cmap.lutTex; }

	inline const std::array<float, 2> &rainSize() const { return rainSize_; }
	inline const std::array<float, 3> &rainColor() const { return rainColor_; }

	inline const std::array<float, 2> &mega1Size() const { return mega1Size_; }
	inline const std::array<float, 3> &mega1Color() const { return mega1Color_; }

	inline const std::array<float, 2> &mega2Size() const { return mega2Size_; }
	inline const std::array<float, 3> &mega2Color() const { return mega2Color_; }

	float gravity1 = -1;
	float damping1 = -1;
	float speed_cap1 = -1;
	float bounce1 = -1;

	float gravity2 = -1;
	float damping2 = -1;
	float speed_cap2 = -1;
	float bounce2 = -1;

	float flow1Decay = -1;

	float flow1RainAmount = -1;
	float flow1RainMinSpeed = -1;
	float flow1DropAmount = -1;
	float flow1DropMinSpeed = -1;
	float flow1DiffuseCenterWeight = -1;
	float flow1DiffuseNeighWeight = -1;

	float flow2Decay = -1;

	float flow2DropAmount = -1;
	float flow2DropMinSpeed = -1;
	float flow2DiffuseCenterWeight = -1;
	float flow2DiffuseNeighWeight = -1;

protected:
	CMap cmap;

	std::array<float, 2> rainSize_;
	std::array<float, 3> rainColor_;
	std::array<float, 2> mega1Size_;
	std::array<float, 3> mega1Color_;
	std::array<float, 2> mega2Size_;
	std::array<float, 3> mega2Color_;
};

// auto base = sampleCurrentCmap(0.1);
// std::cout << base[0] << " " << base[1] << " " << base[2] << "\n";

// auto sampleCurrentCmap = [&](float t) {
// 		t = std::clamp(t, 0.0f, 1.0f);
// 		if (lutCPU.empty() || gCtl.colormapIndex < 0 || gCtl.colormapIndex >= (int)lutCPU.size())
// 		{
// 			return std::array<float, 3>{0.2f, 0.8f, 1.0f};
// 		}

// 		const auto &img = lutCPU[gCtl.colormapIndex];
// 		const int w = lutCPUWidth[gCtl.colormapIndex];
// 		if (w <= 0 || img.size() < size_t(3 * w))
// 		{
// 			return std::array<float, 3>{0.2f, 0.8f, 1.0f};
// 		}

// 		float x = t * float(w - 1);
// 		int x0 = std::clamp(int(std::floor(x)), 0, w - 1);
// 		int x1 = std::clamp(x0 + 1, 0, w - 1);
// 		float a = x - float(x0);

// 		auto c0r = img[3 * x0 + 0] / 255.0f;
// 		auto c0g = img[3 * x0 + 1] / 255.0f;
// 		auto c0b = img[3 * x0 + 2] / 255.0f;
// 		auto c1r = img[3 * x1 + 0] / 255.0f;
// 		auto c1g = img[3 * x1 + 1] / 255.0f;
// 		auto c1b = img[3 * x1 + 2] / 255.0f;

// 		return std::array<float, 3>{
// 			c0r * (1.0f - a) + c1r * a,
// 			c0g * (1.0f - a) + c1g * a,
// 			c0b * (1.0f - a) + c1b * a,
// 		};
// 	};
