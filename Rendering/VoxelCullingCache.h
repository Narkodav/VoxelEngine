#pragma once
#include <vector>

#include "Shape.h"
#include "VoxelModel.h"
#include "GameData/Voxel.h"

struct DrawBitmask
{
	uint32_t bitmask = 0; //shows which of the polygons to draw
};

class VoxelCullingCache
{
private:
	std::vector<DrawBitmask> m_cache;

public:
	void init(const Id::NamedCache<Voxel::State, Id::VoxelState>& states,
		const Id::Cache<VoxelModel, Id::VoxelModel>& models,
		const Id::Cache<Shape::Data, Id::Shape>& shapes);

	const std::vector<SideData>& getVoxelData(Id::VoxelModel main, Id::VoxelModel adj, Shape::Side side) const;
	const std::vector<SideData>& getVoxelDataAir(Id::VoxelModel main, Shape::Side side) const;

private:
	static std::vector<SideData> cull(const Id::Cache<Shape::Data, Id::Shape>& parallelograms,
		const VoxelModel& main, const VoxelModel& adj, Shape::Side side);

	static std::vector<SideData> cullAir(const Id::Cache<Shape::Data, Id::Shape>& parallelograms,
		const VoxelModel& main, Shape::Side side);

	static bool shouldCull(const Shape::Data& main, const Shape::Data& adj,
		Coordinate3D adjRelativePos, Shape::Side side);
};

