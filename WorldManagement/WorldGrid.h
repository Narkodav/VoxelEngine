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
	CoordToChunk m_coordToChunk;
    size_t m_sphereRadius = 0;

public:
	WorldGrid() = default;

	WorldGrid(size_t sphereRadius, glm::ivec3 centerPos);

	void resetSphereRadius(size_t sphereRadius, glm::ivec3 centerPos);

	const auto& getGrid() const { return m_pool.getField<0>(); }
	const auto& getAllocatedChunks() const { return m_allocations; }
	const auto& getCoordToChunk() const { return m_coordToChunk; }
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

	template<Shape::Side side>
	inline Id::VoxelState getAdjacentBlockId(size_t block, size_t x, size_t y, size_t z, const Chunk& chunk) const
	{
		auto& grid = m_pool.getField<0>();
		if constexpr (side == Shape::Side::FRONT)
		{
			if (z == 0)
			{
				auto adj = chunk.neighbourStarts[enumCast(Shape::Side::FRONT)];
				if (adj == noChunkIndex)
					return Constants::emptyStateId;
				return grid[adj + y * Constants::chunkLayerSize + x * Constants::chunkDepth + 15];
			}
			return grid[block - Constants::chunkWidth];
		}
		else if constexpr (side == Shape::Side::BACK)
		{
			if (z == 15)
			{
				auto adj = chunk.neighbourStarts[enumCast(Shape::Side::BACK)];
				if (adj == noChunkIndex)
					return Constants::emptyStateId;
				return grid[adj + y * Constants::chunkLayerSize + x * Constants::chunkDepth];
			}
			return grid[block + Constants::chunkWidth];
		}
		else if constexpr (side == Shape::Side::LEFT)
		{
			if (x == 0)
			{
				auto adj = chunk.neighbourStarts[enumCast(Shape::Side::LEFT)];
				if (adj == noChunkIndex)
					return Constants::emptyStateId;
				return grid[adj + y * Constants::chunkLayerSize + 15 * Constants::chunkDepth + z];
			}
			return grid[block - 1];
		}
		else if constexpr (side == Shape::Side::RIGHT)
		{
			if (x == 15)
			{
				auto adj = chunk.neighbourStarts[enumCast(Shape::Side::RIGHT)];
				if (adj == noChunkIndex)
					return Constants::emptyStateId;
				return grid[adj + y * Constants::chunkLayerSize + z];
			}
			return grid[block + 1];
		}
		else if constexpr (side == Shape::Side::BOTTOM)
		{
			if (y == 0)
			{
				auto adj = chunk.neighbourStarts[enumCast(Shape::Side::BOTTOM)];
				if (adj == noChunkIndex)
					return Constants::emptyStateId;
				return grid[adj + 15 * Constants::chunkLayerSize + x * Constants::chunkDepth + z];
			}
			return grid[block - Constants::chunkLayerSize];
		}
		else if constexpr (side == Shape::Side::TOP)
		{
			if (y == 15)
			{
				auto adj = chunk.neighbourStarts[enumCast(Shape::Side::TOP)];
				if (adj == noChunkIndex)
					return Constants::emptyStateId;
				return grid[adj + x * Constants::chunkDepth + z];
			}
			return grid[block + Constants::chunkLayerSize];
		}
		else
		{
			static_assert(side >= Shape::Side::NUM && "invalid side index");
		}
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

		auto& chunk = m_coordToChunk.at(chunkCoords);
		return m_allocations[chunk].getField<1>().start + coords.x + coords.z * Constants::chunkWidth +
			coords.y * Constants::chunkLayerSize;
	}
};

