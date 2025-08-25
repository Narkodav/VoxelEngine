#pragma once
#include "Common.h"
#include "Rendering/Shape.h"
#include "Utility/StructOfArraysPool.h"

#include <vector>
#include <unordered_map>

//generates a spherical chunk world
class WorldGrid
{
public:
	using Grid = std::vector<Id::VoxelState>;

	struct alignas(16) Chunk {	
		glm::ivec4 coord;
		glm::ivec4 blockCornerCoord;
		uint32_t start;
		uint32_t neighbourStarts[6];
		uint32_t padding;
	};

	using CoordToChunk = std::unordered_map<glm::ivec3, size_t>;


	using GridPoolDescriptor = StructOfArraysPoolType<Id::VoxelState, Constants::chunkSize>;
	using ChunksPoolDescriptor = StructOfArraysPoolType<Chunk, 1>;
	using GridPool = StructOfArraysPool<GridPoolDescriptor, ChunksPoolDescriptor>;

	static inline const uint32_t noChunkIndex = std::numeric_limits<uint32_t>::max();
private:
	GridPool m_pool;
	std::vector<GridPool::Allocation> m_allocations;
	CoordToChunk m_coordToAllocation;

public:
	WorldGrid() = default;

	WorldGrid(size_t sphereRadius, glm::ivec3 centerPos);
	WorldGrid(size_t radius, size_t height, glm::ivec3 centerPos);

	void generateSphere(size_t radius, glm::ivec3 centerPos);
	void generateCylinder(size_t radius, size_t height, glm::ivec3 centerPos);

	const auto& getGrid() const { return m_pool.getField<0>(); }
	const auto& getAllocatedChunks() const { return m_allocations; }
	const auto& getCoordToChunk() const { return m_coordToAllocation; }
	const auto& getPool() const { return m_pool; }


	inline const Id::VoxelState& getBlock(glm::ivec3 coords) const
	{
		return m_pool.getField<0>()[coordsToIndex(coords)];
	}

	inline Id::VoxelState& getBlock(glm::ivec3 coords)
	{
		return m_pool.getField<0>()[coordsToIndex(coords)];
	}

	inline const Id::VoxelState& getBlock(size_t index) const
	{
		return m_pool.getField<0>()[index];
	}

	inline Id::VoxelState& getBlock(size_t index)
	{
		return m_pool.getField<0>()[index];
	}

	void addChunk(glm::ivec3 chunkCoords)
	{
		if (m_coordToAllocation.find(chunkCoords) != m_coordToAllocation.end())
			return;
		m_allocations.push_back(m_pool.allocate());
		auto& alloc = m_allocations.back();
		auto& chunk = alloc.getField<1>();
		chunk.coord = glm::ivec4(chunkCoords, 1);
		chunk.blockCornerCoord = chunk.coord * glm::ivec4(Constants::chunkDimensions, 1);
		chunk.start = alloc.getEntryOffset<0>();
		m_coordToAllocation.insert({ chunkCoords, m_allocations.size() - 1 });
		for (size_t j = 0; j < 6; ++j)
		{
			glm::ivec3 neighbourPos = glm::ivec3(chunk.coord) + Constants::directions3D[j];
			auto it = m_coordToAllocation.find(neighbourPos);
			if (it != m_coordToAllocation.end())
			{
				chunk.neighbourStarts[j] = m_allocations[it->second].getEntryOffset<0>();
				auto& neighbourChunk = m_allocations[it->second].getField<1>();
				neighbourChunk.neighbourStarts[enumCast(reverseDir3D(j))] = chunk.start;
			}
			else chunk.neighbourStarts[j] = noChunkIndex;
		}
	}

	void removeChunk(glm::ivec3 chunkCoords)
	{
		auto it = m_coordToAllocation.find(chunkCoords);
		if (it == m_coordToAllocation.end())
			return;
		auto& alloc = m_allocations[it->second];
		m_pool.free(alloc);
		m_coordToAllocation.find(m_allocations.back().getField<1>().coord)->second = it->second;
		m_allocations[it->second] = m_allocations.back();
		m_allocations.pop_back();
	}

private:
	size_t coordsToIndex(glm::ivec3 coords) const
	{
		glm::ivec3 chunkCoords = { coords.x / Constants::chunkWidth,
		coords.y / Constants::chunkHeight,
		coords.z / Constants::chunkDepth };
		glm::ivec3 localCoords = { coords.x - chunkCoords.x * Constants::chunkWidth,
		coords.y - chunkCoords.y * Constants::chunkHeight,
		coords.z - chunkCoords.z * Constants::chunkDepth };

		auto& chunk = m_coordToAllocation.at(chunkCoords);
		return m_allocations[chunk].getField<1>().start + coords.x + coords.z * Constants::chunkWidth +
			coords.y * Constants::chunkLayerSize;
	}
};

