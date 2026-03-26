#pragma once

#include "Biome.hpp"
#include "FlowMap.hpp"
#include "drop.hpp"
#include "creature.hpp"

#include <memory>
#include <vector>

class Depth;

class Simulation
{
public:
	void rain(const float t, const float u, const float v);
	void randomRain();

	void mega1(float t = -1, float x = -1.0f, float y = -1.0f);
	void mega2(float t = -1, float x = -1.0f, float y = -1.0f);

	void spawnGoat(const Depth &depth, float x = -1.0f, float y = -1.0f, float h0 = -1.0f, float dirx = 0.0f, float diry = 0.0f);
	void spawnPig(const Depth &depth, float x = -1.0f, float y = -1.0f, float h0 = -1.0f, float dirx = 0.0f, float diry = 0.0f);
	void spawnFish(const Depth &depth, float x = -1.0f, float y = -1.0f, float h0 = -1.0f, float dirx = 0.0f, float diry = 0.0f);

	void step(const Depth &depth, float dt);

	void init(const int w, const int h);

	void clear();

	inline const std::vector<Creature> &getCreatures() const { return creatures; }

	inline const std::vector<Drop> &getRain() const { return rain_; }
	inline const std::vector<Drop> &getMega1() const { return drops1; }
	inline const std::vector<Drop> &getMega2() const { return drops2; }

	inline const std::array<float, 2> &rainSize() const
	{
		static std::array<float, 2> size{3.0f, 6.0f};
		return size;
	}
	inline const std::array<float, 3> &rainColor() const
	{
		static std::array<float, 3> color{0.0f, 0.314118f, 0.643529f};
		return color;
	}

	inline const std::array<float, 2> &mega1Size() const
	{
		static std::array<float, 2> size{8.0f, 10.0f};
		return size;
	}
	inline const std::array<float, 3> &mega1Color() const
	{
		static std::array<float, 3> color{0.0f, 0.314118f, 0.643529f};
		return color;
	}

	inline const std::array<float, 2> &mega2Size() const
	{
		static std::array<float, 2> size{8.0f, 10.0f};
		return size;
	}
	inline const std::array<float, 3> &mega2Color() const
	{
		static std::array<float, 3> color{0.0f, 0.314118f, 0.643529f};
		return color;
	}

	inline const FlowMap &getFlowMap1() const { return flowMap1; }
	inline const FlowMap &getFlowMap2() const { return flowMap2; }

private:
	FlowMap flowMap1;
	FlowMap flowMap2;

	std::vector<Drop> rain_;
	std::vector<Drop> drops1;
	std::vector<Drop> drops2;

	std::vector<Creature> creatures;
	std::shared_ptr<Biome> currentBiome;

	double lastMega1Time = 0.0;
	double lastMega2Time = 0.0;

	constexpr static const float ru = 0.5f;
	constexpr static const float rv = 1.0f;
};
