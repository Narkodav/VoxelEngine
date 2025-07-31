#pragma once
#include <vector>
#include <fstream>
#include <unordered_map>

#include "Common.h"
#include "WorldManagement/VoxelVolume.h"
#include "VoxelMeshing.h"

class ChunkMesh
{
private:
	std::array<ChunkMesh*, enumCast(Directions3D::NUM)> m_neighbours;
	VoxelMeshing voxelMeshing;

public:
	ChunkMesh();
	~ChunkMesh();



};

