#pragma once
#include <vector>

#include "Shape.h"
#include "GameData/Voxel.h"
#include "WorldManagement/WorldGrid.h"
#include "ShaderLayoutDefinitions.h"
#include "Math/LinearAlgebra.h"

#include "clipper2/clipper.h"

namespace Clip = Clipper2Lib;

class VoxelCullingCache
{
public:
	using BitMask = uint32_t; //shows which polygons to remove
	using CullingId = Id::IndexSequenceCache<BitMask>::EntryId;

private:
	Id::IndexSequenceCache<BitMask> m_cullings;
	std::vector<CullingId> m_cullingIds;
	size_t m_geometryAmount;

	CullingId m_noCullingId;
	size_t m_noCullingIndex;
public:
	void init(const Shape::PolygonIndexBuffer& geometries,
		const Id::Cache<Shape::Polygon, Id::Polygon>& polygons,
		const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertices,
		const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normals);

	std::vector<BitMask> cull(const Shape::PolygonIndexBuffer& geometries,
		const Id::Cache<Shape::Polygon, Id::Polygon>& polygons,
		const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertices,
		const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normals,
		Shape::GeometryId geometryMain, Shape::GeometryId geometryAdj, Shape::Side side);

	const auto& getCullings() const { return m_cullings; };

	void populateBuffer(size_t block, size_t x, size_t y, size_t z,
		const WorldGrid::Chunk& chunk,
		const WorldGrid& grid, std::vector<Indices>& indices,
		const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStates,
		const Id::NamedCache<Shape::Model, Id::Model>& modelCache,
		const Shape::PolygonIndexBuffer::EntryCache& geometryEntries,
		const Shape::ColoringIndexBuffer::EntryCache& appearanceEntries,
		const Shape::PolygonIndexBuffer& geometries, const Shape::ColoringIndexBuffer& appearances) const;

private:

	inline void clipPolygon(size_t i, glm::vec3 adjOffset,
		const Shape::PolygonIndexBuffer::Entry entryMain, const Shape::PolygonIndexBuffer::Entry& entryAdj,
		const Shape::PolygonIndexBuffer& geometries,
		const Id::Cache<Shape::Polygon, Id::Polygon>& polygons,
		const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertices,
		const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normals,
		std::vector<BitMask>& result) const;

	inline std::array<VoxelCullingCache::CullingId, enumCast(Shape::Side::NUM)> getCullingIndices(
		size_t block, size_t x, size_t y, size_t z, Shape::GeometryId geometryMain,
		const WorldGrid::Chunk& chunk, const WorldGrid& grid,
		const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStates,
		const Id::NamedCache<Shape::Model, Id::Model>& modelCache) const;

	template <Shape::Side side>
	inline VoxelCullingCache::CullingId getCullingIndex(
		size_t block, size_t x, size_t y, size_t z, Shape::GeometryId geometryMain,
		const WorldGrid::Chunk& chunk, const WorldGrid& grid,
		const Id::IndexSequenceCache<BitMask>::EntryCache& cullingEntries,
		const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStates,
		const Id::NamedCache<Shape::Model, Id::Model>& modelCache) const
	{
		auto adjState = grid.getAdjacentBlockId<side>(block, x, y, z, chunk);
		if (adjState == Constants::emptyStateId)
			return m_noCullingIndex;
		auto& geom = modelCache[voxelStates[adjState].m_model].geometry;
		return cullingEntries[m_cullingIds[enumCast(side) * m_geometryAmount * m_geometryAmount
			+ m_geometryAmount * geometryMain + geom]].start;
	}

//	const std::vector<SideData>& getVoxelData(Id::VoxelModel main, Id::VoxelModel adj, Shape::Side side) const;
//	const std::vector<SideData>& getVoxelDataAir(Id::VoxelModel main, Shape::Side side) const;
//
//private:
//	static std::vector<SideData> cull(const Id::Cache<Shape::Data, Id::Shape>& parallelograms,
//		const VoxelModel& main, const VoxelModel& adj, Shape::Side side);
//
//	static std::vector<SideData> cullAir(const Id::Cache<Shape::Data, Id::Shape>& parallelograms,
//		const VoxelModel& main, Shape::Side side);
//
//	static bool shouldCull(const Shape::Data& main, const Shape::Data& adj,
//		Coordinate3D adjRelativePos, Shape::Side side);
};

