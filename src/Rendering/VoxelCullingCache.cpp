#include "Rendering/VoxelCullingCache.h"

void VoxelCullingCache::init(const Shape::PolygonIndexBuffer& geometries,
	const Id::Cache<Shape::Polygon, Id::Polygon>& polygons,
	const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertices,
	const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normals,
	Shape::GeometryId standardBlockGeometryId)
{
	m_standardBlockGeometryId = standardBlockGeometryId;
	// size is (N^2) * 6, 
	// for 1000 models it would be 6 000 000 entries or approximately 68 Mega Bytes, which is acceptable
	//empty geometry is not needed because the cache shows which faces to remove if geometry is not empty
	
	m_geometryAmount = geometries.entrySize();
	m_geometryAmountSquared = m_geometryAmount * m_geometryAmount;
	
	for (size_t i = 0; i < enumCast(Shape::Side::Count); ++i)
		m_sideOffsets[i] = i * m_geometryAmountSquared;
		
	m_cullings.reserveEntryData(m_geometryAmountSquared * enumCast(Shape::Side::Count));
	//one entry would have at least one index
	m_cullings.reserveIndexData(m_geometryAmountSquared * enumCast(Shape::Side::Count));
	m_cullingIds.reserve(m_geometryAmountSquared * enumCast(Shape::Side::Count));
	
	size_t maxEntrySize = 0;
	
	for (size_t side = 0; side < enumCast(Shape::Side::Count); ++side)
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
	//auto maskSize = (entry.size - 1) / (sizeof(BitMask) * 8) + 1;
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
	if (i % ((sizeof(BitMask) * 8)) == 0)
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
		result.back() |= (1 << (i % ((sizeof(BitMask) * 8))));
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
	
	BitMask currentMask;
	//size_t entrySize = 0;
	
	size_t count = geometryEntry.size / (sizeof(BitMask) * 8);
	size_t rest = geometryEntry.size - count * (sizeof(BitMask) * 8);
	
	auto cullingIndices = getCullingIndices(block, x, y, z, modelMain.geometry, chunk, grid, voxelStates, modelCache);
	
	for (size_t i = 0; i < count; ++i)
	{
		currentMask = 0;
		for (size_t j = 0; j < enumCast(Shape::Side::Count); ++j)
			currentMask |= m_cullings[cullingIndices[j] + i];
		for (size_t j = 0; j < (sizeof(BitMask) * 8); ++j)
		{
			if ((currentMask & 1) == 0)
			{
				Indices index;
				index.polygon = geometries[geometryEntry.start + i * (sizeof(BitMask) * 8) + j];
				index.coloring = appearances[appearanceEntry.start + i * (sizeof(BitMask) * 8) + j];
				index.block = block;
				indices.push_back(index);
			}
			currentMask >>= 1;
		}
	}

	currentMask = 0;
	for (size_t j = 0; j < enumCast(Shape::Side::Count); ++j)
		currentMask |= m_cullings[cullingIndices[j] + count];
	for (size_t j = 0; j < rest; ++j)
	{
		if ((currentMask & 1) == 0)
		{
			Indices index;
			index.polygon = geometries[geometryEntry.start + count * (sizeof(BitMask) * 8) + j];
			index.coloring = appearances[appearanceEntry.start + count * (sizeof(BitMask) * 8) + j];
			index.block = block;
			indices.push_back(index);
		}
		currentMask >>= 1;
	}
}

std::array<VoxelCullingCache::CullingId, enumCast(Shape::Side::Count)> VoxelCullingCache::getCullingIndices(
	size_t block, size_t x, size_t y, size_t z, Shape::GeometryId geometryMain,
	const WorldGrid::Chunk& chunk, const WorldGrid& grid,
	const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStates,
	const Id::NamedCache<Shape::Model, Id::Model>& modelCache) const
{
	std::array<CullingId, enumCast(Shape::Side::Count)> cullingIndices;
	auto& cullingEntries = m_cullings.entryCache();
	cullingIndices[enumCast(Shape::Side::Bottom)] = getCullingIndex<Shape::Side::Bottom>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::Top)] = getCullingIndex<Shape::Side::Top>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::Left)] = getCullingIndex<Shape::Side::Left>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::Right)] = getCullingIndex<Shape::Side::Right>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::Back)] = getCullingIndex<Shape::Side::Back>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	cullingIndices[enumCast(Shape::Side::Front)] = getCullingIndex<Shape::Side::Front>(
		block, x, y, z, geometryMain, chunk, grid, cullingEntries, voxelStates, modelCache);
	return cullingIndices;
}