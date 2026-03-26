#pragma once

#include <vector>

#include "utils.hpp"
#include "image.hpp"

class CMap
{
public:
	bool load(const std::string &filename)
	{
		int w = 0;
		auto data = load_png_as_1d_texture(filename, w);
		if (!data.empty())
		{
			lutCPU = std::move(data);
			lutCPUWidth = w;
			lutTex = makeColormap1D(lutCPU, lutCPUWidth);
			return true;
		}
		return false;
	}

	GLuint lutTex;
	std::vector<uint8_t> lutCPU;
	int lutCPUWidth;
};