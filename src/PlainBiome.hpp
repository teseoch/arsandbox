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

		gravity1 = 0.20f;
		damping1 = 0.992f;
		speed_cap1 = 0.06f;
		bounce1 = -0.5f;

		gravity2 = 0.10f;
		damping2 = 0.97f;
		speed_cap2 = 0.025f;
		bounce2 = -0.1f;

		flow1Decay = 0.975f;

		flow1RainAmount = 4.5f;
		flow1RainMinSpeed = 0.0005f;
		flow1DropAmount = 5.5f;
		flow1DropMinSpeed = 0.0005f;
		flow1DiffuseCenterWeight = 0.78f;
		flow1DiffuseNeighWeight = 0.055f;

		flow2Decay = 0.975f;

		flow2DropAmount = 10.0f;
		flow2DropMinSpeed = 0.0002f;
		flow2DiffuseCenterWeight = 0.96f;
		flow2DiffuseNeighWeight = 0.01f;

		mega1FlowColor = {0.0f, 0.314118f, 0.643529f};
		mega2FlowColor = {1.0f, 0.35f, 0.05f};

		water_threshold = 0.2f;
	}

	std::tuple<float, float, float> trail_color(const std::array<float, 3> &col) const override
	{
		float rr = 0.65f * col[0] + 0.35f * 0.75f;
		float rg = 0.65f * col[1] + 0.35f * 0.95f;
		float rb = 0.65f * col[2] + 0.35f * 1.00f;

		return {rr, rg, rb};
	}

	std::tuple<float, float, float> head_color(const std::array<float, 3> &col) const override
	{
		float rr = 0.50f * col[0] + 0.50f * 0.75f;
		float rg = 0.50f * col[1] + 0.50f * 0.95f;
		float rb = 0.50f * col[2] + 0.50f * 1.00f;

		return {rr, rg, rb};
	}
};
