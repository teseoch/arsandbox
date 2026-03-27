#pragma once

#include "Biome.hpp"

#include <stdio.h>
#include <stdexcept>
#include <string>
#include <iostream>

class PlainBiome : public Biome
{
public:
	PlainBiome()
	{
		const std::string folder = AR_IMAGE_FOLDER;

		const std::string cmap_path = folder + "/cmaps/topo_classic.png";
		bool ok = cmap.load(cmap_path);
		if (!ok)
		{
			std::cerr << "Failed to load colormap: " << cmap_path << std::endl;
			throw std::runtime_error("Failed to load colormap");
		}

		rainSize_ = {3.0f, 6.0f};
		rainColor_ = {0.0f, 0.314118f, 0.643529f};
		mega1Size_ = {8.0f, 10.0f};
		mega1Color_ = {0.0f, 0.314118f, 0.643529f};
		mega2Size_ = {8.0f, 10.0f};
		mega2Color_ = {1.0f, 0.35f, 0.05f};
	}
};
