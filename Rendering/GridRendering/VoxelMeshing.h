#pragma once
#include <vector>
#include <fstream>
#include <unordered_map>

#include "Common.h"
#include "Rendering/Shape.h"
#include "WorldManagement/VoxelVolume.h"
#include "Rendering/AssetCache.h"
#include "GameData/ResourceCache.h"
#include "Rendering/Shaders/ShaderCache.h"

class VoxelMeshing
{
public:
	using AdjacentVolumes = std::array<const VoxelVolume*, enumCast(Directions3D::NUM)>;

private:

	Gfx::Buffer m_chunkBuffer;
	Gfx::Memory m_chunkMemory;

public:
	void render(const VoxelVolume& volume, AdjacentVolumes& adjacentVolumes,
		const Coordinate3D& chunkPosition, const ResourceCache& cache);

	void addBlock(const VoxelVolume& volume, AdjacentVolumes& adjacentVolumes,
		const Coordinate3D& chunkPosition, const Coordinate3D& coord,
		const ResourceCache& cache, BufferData& data);

	void updateVertexBuffers(BufferData&& data);

	void clear();

	void draw(const Coordinate3D& chunkCoord) const;
private:

	template <Shape::Side side>
	constexpr void addSide(const VoxelVolume& volume, AdjacentVolumes& adjacentVolumes,
		const Coordinate3D& chunkPosition, const Coordinate3D& coord,
		const ResourceCache& cache, const VoxelCullingCache& voxelCullings, BufferData& data)
	{
		Direction3D adjacentCoord;
		adjacentCoord.x = Constants::directions3D[enumCast(side)].x + coord.x;
		adjacentCoord.y = Constants::directions3D[enumCast(side)].y + coord.y;
		adjacentCoord.z = Constants::directions3D[enumCast(side)].z + coord.z;

		VoxelData voxelData;

		if (isInChunkBounds(adjacentCoord))
		{
			voxelData = voxelCullings.getVoxelData(cache.getVoxel(volume[coord]).getModel(),
				cache.getVoxel(volume[adjacentCoord]).getModel(), side);
		}
		else
		{
			const VoxelVolume* adjacentVolume = adjacentVolumes[enumCast(side)];
			if (adjacentVolume == nullptr)
			{
				voxelData = voxelCullings.getVoxelDataAir(
					cache.getVoxel(volume[coord]).getModel(), side);
			}
			else
			{
				auto [_, AdjChunkLocal] = fromGlobalToChunk(fromChunkToGlobal(chunkPosition, adjacentCoord));
				voxelData = voxelCullings.getVoxelData(cache.getVoxel(volume[coord]).getModel(),
					cache.getVoxel((*adjacentVolume)[AdjChunkLocal]).getModel(), side);
			}
		}
		if (voxelData.parallelogramId.empty())
			return;

		data.parallelogramIDs.insert(data.parallelogramIDs.end(),
			voxelData.parallelogramId.begin(), voxelData.parallelogramId.end());
		data.faces.insert(data.faces.end(),
			voxelData.sideToDraw.begin(), voxelData.sideToDraw.end());
		data.positions.insert(data.positions.end(), voxelData.parallelogramId.size(), coord);
	}
};

