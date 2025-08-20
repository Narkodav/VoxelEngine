#include "VoxelCullingCache.h"

void VoxelCullingCache::init(const Shape::PolygonIndexBuffer& geometries,
	const Id::Cache<Shape::Polygon, Id::Polygon>& polygons,
	const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertices,
	const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normals)
{
	// size is (N^2) * 6, 
	// for 1000 models it would be 6 000 000 entries or approximately 68 Mega Bytes, which is acceptable
	//empty geometry is not needed because the cache shows which faces to remove if geometry is not empty

	m_geometryAmount = geometries.entrySize();

	m_cullings.reserveEntryData(m_geometryAmount * m_geometryAmount * enumCast(Shape::Side::NUM));
	//one entry would have at least one index
	m_cullings.reserveIndexData(m_geometryAmount * m_geometryAmount * enumCast(Shape::Side::NUM));
	m_cullingIds.reserve(m_geometryAmount * m_geometryAmount * enumCast(Shape::Side::NUM));

	size_t maxEntrySize = 0;

	for (size_t side = 0; side < enumCast(Shape::Side::NUM); ++side)
	{
		for (size_t i = 0; i < m_geometryAmount; ++i)
		{
			for (size_t j = 0; j < m_geometryAmount; ++j)
			{				
				auto mask = cull(geometries, polygons, vertices, normals,
					static_cast<Shape::GeometryId>(i),
					static_cast<Shape::GeometryId>(j),
					static_cast<Shape::Side>(side));
				if (mask.size() > maxEntrySize)
					maxEntrySize = mask.size();
				m_cullingIds.push_back(m_cullings.add(mask));
			}
		}
	}
	m_noCullingId = m_cullings.add(std::vector<BitMask>(maxEntrySize, 0));
	m_noCullingIndex = m_cullings.entryCache()[m_noCullingId].start;
}

std::vector<VoxelCullingCache::BitMask> VoxelCullingCache::cull(const Shape::PolygonIndexBuffer& geometries,
	const Id::Cache<Shape::Polygon, Id::Polygon>& polygons,
	const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertices,
	const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normals,
	Shape::GeometryId geometryMain, Shape::GeometryId geometryAdj, Shape::Side side)
{
	//auto entry = geometries.entryCache()[geometryMain];
	//auto maskSize = (entry.size - 1) / std::numeric_limits<BitMask>::digits + 1;
	//return std::vector<BitMask>(maskSize, 0);
	std::vector<VoxelCullingCache::BitMask> result;

	auto& geometryEntries = geometries.entryCache();
	auto& entryMain = geometryEntries[geometryMain];
	auto& entryAdj = geometryEntries[geometryAdj];

	for (size_t i = 0; i < entryMain.size; ++i)
	{
		clipPolygon(i, Constants::directionsFloat3D[enumCast(side)], entryMain, entryAdj,
			geometries, polygons, vertices, normals, result);
	}
	return result;
}

inline void VoxelCullingCache::clipPolygon(size_t i, glm::vec3 adjOffset,
	const Shape::PolygonIndexBuffer::Entry entryMain, const Shape::PolygonIndexBuffer::Entry& entryAdj,
	const Shape::PolygonIndexBuffer& geometries,
	const Id::Cache<Shape::Polygon, Id::Polygon>& polygons,
	const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertices,
	const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normals,
	std::vector<BitMask>& result) const
{
	Clip::ClipperD clipper;
	Clip::PathsD adjUnion;
	Clip::PathD mainPoly;
	if (i % std::numeric_limits<BitMask>::digits == 0)
		result.push_back(0);
	mainPoly.clear();
	adjUnion.clear();

	auto polygonId = geometries[entryMain.start + i];
	auto polygon = polygons[polygonId];
	auto normalMain = glm::vec3(normals[polygon.normal]);

	LinearAlgebra::CoordinateSystem cs;
	cs.alignWithZAxis(normalMain);
	size_t vert = 0;
	{
		auto vertex = vertices[polygon.position[vert]];
		cs.setOffset(glm::vec3(vertex));
		auto projection = cs.worldToLocal(glm::vec3(vertex));
		mainPoly.push_back(Clip::PointD(projection.x, projection.y));
	}
	++vert;
	for (; vert < 3; ++vert)
	{
		auto vertex = vertices[polygon.position[vert]];
		auto projection = cs.worldToLocal(glm::vec3(vertex));
		mainPoly.push_back(Clip::PointD(projection.x, projection.y));		
	}

	for (size_t j = 0; j < entryAdj.size; ++j)
	{
		polygonId = geometries[entryAdj.start + j];
		polygon = polygons[polygonId];
		auto normalAdj = glm::vec3(normals[polygon.normal]);

		if (glm::dot(normalMain, normalAdj) > -0.999f)
			continue;

		Clip::PathD adjPoly;
		size_t vert = 0;
		{
			auto vertex = glm::vec3(vertices[polygon.position[vert]]) + adjOffset;
			auto projection = cs.worldToLocal(vertex);
			if (abs(projection.z) > std::numeric_limits<float>::epsilon())
				continue;
			adjPoly.push_back(Clip::PointD(projection.x, projection.y));
		}
		++vert;
		for (; vert < 3; ++vert)
		{
			auto vertex = glm::vec3(vertices[polygon.position[vert]]) + adjOffset;
			auto projection = cs.worldToLocal(vertex);
			adjPoly.push_back(Clip::PointD(projection.x, projection.y));
		}
		clipper.AddSubject(Clip::PathsD{ adjPoly });
	}

	bool success = clipper.Execute(Clip::ClipType::Union,
		Clip::FillRule::EvenOdd,
		adjUnion);
	if (!success) throw std::runtime_error("Clipper union operation failed");

	clipper.AddSubject(Clip::PathsD{ mainPoly }); // Add all subject polygons
	clipper.AddClip(adjUnion);

	Clip::PathsD differenceResult;
	clipper.Execute(Clip::ClipType::Difference, Clip::FillRule::EvenOdd, differenceResult);

	if (differenceResult.empty())
		result.back() |= (1 << (i % std::numeric_limits<BitMask>::digits));
}

void VoxelCullingCache::populateBuffer(size_t block, size_t x, size_t y, size_t z,
	const WorldGrid::Chunk& chunk, 
	const WorldGrid& grid, std::vector<Indices>& indices,
	const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStates,
	const Id::NamedCache<Shape::Model, Id::Model>& modelCache,
	const Shape::PolygonIndexBuffer::EntryCache& geometryEntries,
	const Shape::ColoringIndexBuffer::EntryCache& appearanceEntries,
	const Shape::PolygonIndexBuffer& geometries, const Shape::ColoringIndexBuffer& appearances) const
{
	auto state = grid.getBlock(block);
	if (state == Constants::emptyStateId)
		return;
	const auto& modelMain = modelCache[voxelStates[state].m_model];
	const auto& geometryEntry = geometryEntries[modelMain.geometry];
	const auto& appearanceEntry = appearanceEntries[modelMain.appearence];	

	//for (size_t i = 0; i < geometryEntry.size; ++i)
	//{
	//	Indices index;
	//	index.polygon = geometries[geometryEntry.start + i];
	//	index.coloring = appearances[appearanceEntry.start + i];
	//	index.block = block;
	//	indices.push_back(index);
	//}

	BitMask currentMask;
	size_t entrySize = 0;	

	size_t count = geometryEntry.size / std::numeric_limits<BitMask>::digits;
	size_t rest = geometryEntry.size - count * std::numeric_limits<BitMask>::digits;

	auto cullingIndices = getCullingIndices(block, x, y, z, modelMain.geometry, chunk, grid, voxelStates, modelCache);

	for (size_t i = 0; i < count; ++i)
	{
		currentMask = 0;
		for (size_t j = 0; j < enumCast(Shape::Side::NUM); ++j)
			currentMask |= m_cullings[cullingIndices[j] + i];
		for (size_t j = 0; j < std::numeric_limits<BitMask>::digits; ++j)
		{
			if ((currentMask & 1) == 0)
			{
				Indices index;
				index.polygon = geometries[geometryEntry.start + i * std::numeric_limits<BitMask>::digits + j];
				index.coloring = appearances[appearanceEntry.start + i * std::numeric_limits<BitMask>::digits + j];
				index.block = block;
				indices.push_back(index);
			}
			currentMask >>= 1;
		}
	}

	currentMask = 0;
	for (size_t j = 0; j < enumCast(Shape::Side::NUM); ++j)
		currentMask |= m_cullings[cullingIndices[j] + count];
	for (size_t j = 0; j < rest; ++j)
	{
		if ((currentMask & 1) == 0)
		{
			Indices index;
			index.polygon = geometries[geometryEntry.start + count * std::numeric_limits<BitMask>::digits + j];
			index.coloring = appearances[appearanceEntry.start + count * std::numeric_limits<BitMask>::digits + j];
			index.block = block;
			indices.push_back(index);
		}
		currentMask >>= 1;
	}
}

std::array<VoxelCullingCache::CullingId, enumCast(Shape::Side::NUM)> VoxelCullingCache::getCullingIndices(
	size_t block, size_t x, size_t y, size_t z, Shape::GeometryId geometryMain,
	const WorldGrid::Chunk& chunk, const WorldGrid& grid,
	const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStates,
	const Id::NamedCache<Shape::Model, Id::Model>& modelCache) const
{
	std::array<CullingId, enumCast(Shape::Side::NUM)> cullingIndices;
	auto& cullingEntries = m_cullings.entryCache();
	cullingIndices[enumCast(Shape::Side::BOTTOM)] = getCullingIndex<Shape::Side::BOTTOM>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::TOP)] = getCullingIndex<Shape::Side::TOP>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::LEFT)] = getCullingIndex<Shape::Side::LEFT>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::RIGHT)] = getCullingIndex<Shape::Side::RIGHT>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::BACK)] = getCullingIndex<Shape::Side::BACK>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::FRONT)] = getCullingIndex<Shape::Side::FRONT>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	return cullingIndices;
}