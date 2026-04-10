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
		: Biome(BiomeType::Lava)
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
		rainColor_ = {0.32f, 0.28f, 0.26f};

		lavaRainSize_ = {2.5f, 4.5f};
		lavaRainColor_ = {0.8f, 0.1f, 0.0f};

		mega1Size_ = {6.0f, 8.0f};
		mega1Color_ = {0.45f, 0.32f, 0.22f};

		mega2Size_ = {9.0f, 12.0f};
		mega2Color_ = {1.0f, 0.30f, 0.08f};

		gravity1 = 0.12f;
		damping1 = 0.990f;
		speed_cap1 = 0.035f;
		bounce1 = -0.15f;

		gravity2 = 0.07f;
		damping2 = 0.982f;
		speed_cap2 = 0.018f;
		bounce2 = -0.05f;

		flow1Decay = 0.975f;

		flow1RainAmount = 2.5f;
		flow1RainMinSpeed = 0.0006f;
		flow1DropAmount = 3.0f;
		flow1DropMinSpeed = 0.0006f;
		flow1DiffuseCenterWeight = 0.85f;
		flow1DiffuseNeighWeight = 0.035f;

		flow2Decay = 0.992f;

		flow2RainAmount = 10.0f;
		flow2RainMinSpeed = 0.00015f;

		flow2DropAmount = 8.0f;
		flow2DropMinSpeed = 0.00015f;
		flow2DiffuseCenterWeight = 0.97f;
		flow2DiffuseNeighWeight = 0.008f;

		// Flow colors (used in shader)
		mega1FlowColor = {0.45f, 0.22f, 0.10f};
		mega2FlowColor = {1.0f, 0.40f, 0.10f};

		lava_threshold = 0.6;
	}

	std::tuple<float, float, float> trail_color(const std::array<float, 3> &col) const override
	{
		// Warm the particle trail slightly without pushing it toward blue/cyan.
		float rr = 0.80f * col[0] + 0.20f * 1.00f;
		float rg = 0.80f * col[1] + 0.20f * 0.72f;
		float rb = 0.80f * col[2] + 0.20f * 0.42f;

		return {rr, rg, rb};
	}

	std::tuple<float, float, float> head_color(const std::array<float, 3> &col) const override
	{
		// Brighter hot core for the particle head.
		float rr = 0.65f * col[0] + 0.35f * 1.00f;
		float rg = 0.65f * col[1] + 0.35f * 0.72f;
		float rb = 0.65f * col[2] + 0.35f * 0.30f;

		return {rr, rg, rb};
	}
};
