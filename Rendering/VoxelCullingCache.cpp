#include "VoxelCullingCache.h"

void VoxelCullingCache::init(const Id::NamedCache<Voxel::State, Id::VoxelState>& states,
	const Id::Cache<VoxelModel, Id::VoxelModel>& models,
	const Id::Cache<Shape::Data, Id::Shape>& shapes)
{
	// size is (N^2) * 6, 
	// for 1000 models it would be 6 000 000 entries or approximately 274 Mega Bytes, which is acceptable

	m_cache.reserve(states.size() * states.size() * enumCast(Shape::Side::NUM));

	m_sideSectorSize = voxels.size() * voxels.size();
	m_voxelAmount = voxels.size();

	for (size_t side = 0; side < enumCast(Shape::Side::NUM); ++side)
	{
		for (size_t i = 0; i < voxels.size(); ++i)
		{
			for (size_t j = 0; j < voxels.size(); ++j)
				m_cache.push_back(cull(parallelograms, voxels[i], voxels[j], Shape::Side(side)));
			m_airCullCache.push_back(cullAir(parallelograms, voxels[i], Shape::Side(side)));
		}
	}
}

const VoxelData& VoxelCullingCache::getVoxelData(Id::VoxelModel main, Id::VoxelModel adj, Shape::Side side) const
{
	return m_cache[main * m_voxelAmount + adj + enumCast(side) * m_sideSectorSize];
}

const VoxelData& VoxelCullingCache::getVoxelDataAir(Id::VoxelModel main, Shape::Side side) const
{
	return m_airCullCache[main + enumCast(side) * m_voxelAmount];
}

VoxelData VoxelCullingCache::cullAir(const Id::Cache<Shape::Data, Id::Shape>& parallelograms,
	const VoxelModel& main, Shape::Side side)
{
	const auto& mainShapes = main.getShapes();

	VoxelData data;

	for (size_t i = 0; i < mainShapes.size(); ++i)
	{
		if (parallelograms[mainShapes[i]].rotations != glm::vec4(0, 0, 0, 1))
		{
			data.parallelogramId.push_back(mainShapes[i]);
			data.sideToDraw.push_back(side);
			continue;
		}

		//cull mains own parallelograms
		for (size_t j = 0; j < mainShapes.size(); ++j)
		{
			if (i == j)
				continue;
			if (shouldCull(parallelograms[mainShapes[i]],
				parallelograms[mainShapes[j]], Coordinate3D(0, 0, 0), side))
				break;
		}

		data.parallelogramId.push_back(mainShapes[i]);
		data.sideToDraw.push_back(side);
	}
	return data;
}

VoxelData VoxelCullingCache::cull(const Id::Cache<Shape::Data, Id::Shape>& parallelograms,
	const VoxelModel& main, const VoxelModel& adj, Shape::Side side)
{
	const auto& mainShapes = main.getShapes();
	const auto& adjShapes = adj.getShapes();

	VoxelData data;	

	for (size_t i = 0; i < mainShapes.size(); ++i)
	{
		if (parallelograms[mainShapes[i]].rotations != glm::vec4(0, 0, 0, 1))
		{
			data.parallelogramId.push_back(mainShapes[i]);
			data.sideToDraw.push_back(side);
			continue;
		}
		bool shouldCullFlag = false;

		//cull mains own parallelograms
		for (size_t j = 0; j < mainShapes.size(); ++j)
		{
			if (i == j)
				continue;
			if (shouldCull(parallelograms[mainShapes[i]],
				parallelograms[mainShapes[j]], Coordinate3D(0, 0, 0), side))
			{
				shouldCullFlag = true;
				break;
			}
		}
		if (shouldCullFlag)
			continue;

		//cull mains parallelograms relative to adj
		for (size_t j = 0; j < mainShapes.size(); ++j)
		{
			if (!shouldCull(parallelograms[mainShapes[i]],
				parallelograms[adjShapes[j]], Constants::directions3D[enumCast(side)], side))
			{
				shouldCullFlag = true;
				break;
			}
		}
		if (shouldCullFlag)
			continue;

		data.parallelogramId.push_back(mainShapes[i]);
		data.sideToDraw.push_back(side);
	}
	return data;
}

bool VoxelCullingCache::shouldCull(const Shape::Data& main, const Shape::Data& adj,
	Coordinate3D adjRelativePos, Shape::Side side)
{
	auto faceMainVertices = Shape::frontFacePositions;
	auto faceOtherVertices = Shape::frontFacePositions;
	for (int i = 0; i < enumCast(Shape::FaceCorner::NUM); ++i)
	{
		Shape::transformFaceVertex(faceMainVertices[i], main.dimensions, main.position, side);
		Shape::transformFaceVertex(faceOtherVertices[i], adj.dimensions,
			adj.position + glm::vec4(adjRelativePos, 1), side);
	}

	return Shape::isFaceContained(faceOtherVertices, faceMainVertices);
}