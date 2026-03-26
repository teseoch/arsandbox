#pragma once

#include <string>

class Biome
{
public:
	virtual ~Biome() = default;

	virtual const std::string &texture() const = 0;
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
