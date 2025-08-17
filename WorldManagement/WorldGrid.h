#pragma once
#include "Common.h"

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

	using Chunks = std::vector<Chunk>;
	using CoordToChunk = std::unordered_map<glm::ivec3, size_t>;

	struct ChunkView {
		std::span<Id::VoxelState> chunk;
		std::span<Id::VoxelState> neighbours[6];

		static ChunkView fromChunk(Grid& grid, const Chunk& chunk)
		{
			ChunkView view;
			view.chunk = std::span<Id::VoxelState>(
				grid.data() + chunk.start, Constants::chunkSize);
			for (size_t i = 0; i < 6; i++)
				view.neighbours[i] = std::span<Id::VoxelState>(
					grid.data() + chunk.neighbourStarts[i], Constants::chunkSize);
			return view;
		}
	};

	struct ConstChunkView {
		std::span<const Id::VoxelState> chunk;
		std::span<const Id::VoxelState> neighbours[6];

		static ConstChunkView fromChunk(const Grid& grid, const Chunk& chunk)
		{
			ConstChunkView view;
			view.chunk = std::span<const Id::VoxelState>(
				grid.data() + chunk.start, Constants::chunkSize);
			for (size_t i = 0; i < 6; i++)
				view.neighbours[i] = std::span<const Id::VoxelState>(
					grid.data() + chunk.neighbourStarts[i], Constants::chunkSize);
			return view;
		}
	};

private:
	Grid m_grid;
	Chunks m_chunks;
	CoordToChunk m_coordToChunk;
	size_t m_sphereRadius = 0;

public:
	WorldGrid() = default;

	WorldGrid(size_t sphereRadius, glm::ivec3 centerPos);

	void resetSphereRadius(size_t sphereRadius, glm::ivec3 centerPos);

	const Grid& getGrid() const { return m_grid; }
	const Chunks& getChunks() const { return m_chunks; }
	const CoordToChunk& getCoordToChunk() const { return m_coordToChunk; }
	
	ConstChunkView getChunk(glm::ivec3 chunkCoords) const
	{
		const Chunk& chunk = m_chunks[m_coordToChunk.at(chunkCoords)];
		return ConstChunkView::fromChunk(m_grid, chunk);
	}

	ChunkView getChunk(glm::ivec3 chunkCoords)
	{
		const Chunk& chunk = m_chunks[m_coordToChunk.at(chunkCoords)];
		return ChunkView::fromChunk(m_grid, chunk);
	}

	ConstChunkView getChunk(size_t index) const
	{
		const Chunk& chunk = m_chunks[m_grid.data() +
			index * Constants::chunkSize, Constants::chunkSize];
		return ConstChunkView::fromChunk(m_grid, chunk);
	}

	ChunkView getChunk(size_t index)
	{
		const Chunk& chunk = m_chunks[m_grid.data() +
			index * Constants::chunkSize, Constants::chunkSize];
		return ChunkView::fromChunk(m_grid, chunk);
	}

	const Id::VoxelState& getBlock(glm::ivec3 coords) const
	{
		return m_grid[coordsToIndex(coords)];
	}

	Id::VoxelState& getBlock(glm::ivec3 coords)
	{
		return m_grid[coordsToIndex(coords)];
	}

	const Id::VoxelState& getBlock(size_t index) const
	{
		return m_grid[index];
	}

	Id::VoxelState& getBlock(size_t index)
	{
		return m_grid[index];
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

		return m_chunks[chunk].start + coords.x + coords.z * Constants::chunkWidth +
			coords.y * Constants::chunkLayerSize;
	}
};

