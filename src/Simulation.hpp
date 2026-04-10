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
	Simulation();

	void rain(const float t, const float u, const float v);
	void randomRain();

	void lavaRain(const float t, const float u, const float v);
	void randomLavaRain();

	void mega1(float t = -1, float x = -1.0f, float y = -1.0f);
	void mega2(float t = -1, float x = -1.0f, float y = -1.0f);

	void spawnGoat(const Depth &depth, float t = -1, float x = -1.0f, float y = -1.0f, float dirx = 0.0f, float diry = 0.0f);
	void spawnPig(const Depth &depth, float t = -1, float x = -1.0f, float y = -1.0f, float dirx = 0.0f, float diry = 0.0f);
	void spawnFish(const Depth &depth, float t = -1, float x = -1.0f, float y = -1.0f, float dirx = 0.0f, float diry = 0.0f);

	void spawnEagle(const Depth &depth, float t = -1, float x = -1.0f, float y = -1.0f, float dirx = 0.0f, float diry = 0.0f);
	void spawnVolture(const Depth &depth, float t = -1, float x = -1.0f, float y = -1.0f, float dirx = 0.0f, float diry = 0.0f);
	void spawnWolf(const Depth &depth, float t = -1, float x = -1.0f, float y = -1.0f, float dirx = 0.0f, float diry = 0.0f);
	void spawnFrog(const Depth &depth, float t = -1, float x = -1.0f, float y = -1.0f, float dirx = 0.0f, float diry = 0.0f);

	void step(const Depth &depth, float dt);

	void init(const int w, const int h);

	void clear();

	void nextBiome();
	void prevBiome();

	void goToBiome(int index);

	inline const std::vector<std::shared_ptr<Creature>> &getCreatures() const { return creatures; }

	inline const std::vector<Drop> &getRain() const { return rain_; }
	inline const std::vector<Drop> &getLavaRain() const { return lava_rain_; }

	inline const std::vector<Drop> &getMega1() const { return drops1; }
	inline const std::vector<Drop> &getMega2() const { return drops2; }

	inline const std::array<float, 2> &rainSize() const { return currentBiome()->rainSize(); }
	inline const std::array<float, 3> &rainColor() const { return currentBiome()->rainColor(); }

	inline const std::array<float, 2> &lavaRainSize() const { return currentBiome()->lavaRainSize(); }
	inline const std::array<float, 3> &lavaRainColor() const { return currentBiome()->lavaRainColor(); }

	inline const std::array<float, 2> &mega1Size() const { return currentBiome()->mega1Size(); }
	inline const std::array<float, 3> &mega1Color() const { return currentBiome()->mega1Color(); }

	inline const std::array<float, 2> &mega2Size() const { return currentBiome()->mega2Size(); }
	inline const std::array<float, 3> &mega2Color() const { return currentBiome()->mega2Color(); }

	inline const FlowMap &getFlowMap1() const { return flowMap1; }
	inline const FlowMap &getFlowMap2() const { return flowMap2; }

	inline const GLuint &texture() const { return currentBiome()->texture(); }

	std::shared_ptr<Biome> currentBiome() const { return biomes[biomeIndex]; }

	int biomeIndex = 0;

private:
	std::shared_ptr<Creature> findNearestCreature(
		const std::shared_ptr<Creature> &self,
		const std::vector<std::shared_ptr<Creature>> &candidates) const;

	FlowMap flowMap1;
	FlowMap flowMap2;

	std::vector<Drop> rain_;
	std::vector<Drop> lava_rain_;
	std::vector<Drop> drops1;
	std::vector<Drop> drops2;

	std::vector<std::shared_ptr<Creature>> creatures;
	std::vector<std::shared_ptr<Creature>> rammables;
	std::vector<std::shared_ptr<Creature>> preys;
	std::vector<std::shared_ptr<Creature>> wolf_preys;
	std::vector<std::shared_ptr<Creature>> corpses;

	std::vector<std::shared_ptr<Biome>> biomes;

	float lastMega1Time = 0.0f;
	float lastMega2Time = 0.0f;

	float lastGoatTime = 0.0f;
	float lastPigTime = 0.0f;
	float lastFishTime = 0.0f;
	float lastEagleTime = 0.0f;
	float lastVoltureTime = 0.0f;
	float lastWolfTime = 0.0f;
	float lastFrogTime = 0.0f;

	float jitter = 0.01f;

	constexpr static const float ru = 0.5f;
	constexpr static const float rv = 1.0f;
};
