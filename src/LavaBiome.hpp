#pragma once

#include "Biome.hpp"

#include <stdio.h>
#include <stdexcept>
#include <string>
#include <iostream>

class LavaBiome : public Biome
{
public:
	LavaBiome()
	{
		const std::string folder = AR_IMAGE_FOLDER;

		const std::string cmap_path = folder + "/cmaps/volcanic.png";
		bool ok = cmap.load(cmap_path);
		if (!ok)
		{
			std::cerr << "Failed to load colormap: " << cmap_path << std::endl;
			throw std::runtime_error("Failed to load colormap");
		}

		rainSize_ = {2.5f, 4.5f};
		rainColor_ = {0.25f, 0.25f, 0.30f};

		mega1Size_ = {6.0f, 8.0f};
		mega1Color_ = {0.35f, 0.30f, 0.28f};

		mega2Size_ = {9.0f, 12.0f};
		mega2Color_ = {1.0f, 0.38f, 0.08f};
	}
};
