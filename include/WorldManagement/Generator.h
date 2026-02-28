#ifndef GENERATOR_H
#define GENERATOR_H

#pragma once
#include "Common.h"

#include "WorldManagement/WorldGrid.h"

#include "Mathematics/PerlinNoise2d.h"
#include "Mathematics/PerlinNoise3d.h"

#include <random>

//generators are custom made for a specific item set, completely new item set requires a new generator
class Generator
{
public:
	enum class BlockTypes :unsigned int
	{
		Air = 0,
		Dirt = 1,
		Num,
	};

private:
	using SeedType = uint64_t;
	Math::PerlinNoise2d m_perlinNoise2d;
	Math::PerlinNoise3d m_perlinNoise3d1;
	Math::PerlinNoise3d m_perlinNoise3d2;

	Id::VoxelState m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Num)];

	static const int32_t m_groundLevel = 200;
	SeedType m_seed;
public:
	Generator();
	Generator(const Generator& other) = default;
	Generator& operator=(const Generator& other) = default;

	void set(SeedType seed);

	bool shouldBeCave(int32_t x, int32_t y, int32_t z);

	void setChunkData(WorldGrid& grid, size_t allocIndex);

	void fillChunk(WorldGrid& grid, size_t allocIndex, BlockTypes type);
};

#endif

