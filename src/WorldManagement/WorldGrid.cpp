#include "WorldManagement/WorldGrid.h"

void WorldGrid::generate(const ShapeSettings& settings) {
	switch (settings.shape) {
		case ShapeToGenerate::Sphere:
			generateSphere(*static_cast<const SphereShapeParams*>(settings.params.get()));
			break;
		case ShapeToGenerate::Cylinder:
			generateCylinder(*static_cast<const CylinderShapeParams*>(settings.params.get()));
			break;
		case ShapeToGenerate::Parallelepiped:
			generateParallelepiped(*static_cast<const ParallelepipedShapeParams*>(settings.params.get()));
			break;
		case ShapeToGenerate::Cube:
			generateCube(*static_cast<const CubeShapeParams*>(settings.params.get()));
			break;
		default:
			throw std::runtime_error("Shape not implemented");
	}
}

void WorldGrid::generateSphere(const SphereShapeParams& params)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool((params.radius * 2 + 1) * (params.radius * 2 + 1) * (params.radius * 2 + 1) * 2);

	glm::ivec3 pos;

	for (pos.x = -static_cast<int32_t>(params.radius);
		pos.x < static_cast<int32_t>(params.radius + 1); ++pos.x)
	{
		for (pos.z = -static_cast<int32_t>(params.radius);
			pos.z < static_cast<int32_t>(params.radius + 1); ++pos.z)
		{
			for (pos.y = -static_cast<int32_t>(params.radius);
				pos.y < static_cast<int32_t>(params.radius + 1); ++pos.y)
			{
				glm::ivec3 posRel = pos + params.centerPosition;
				if (glm::length(glm::vec3(pos)) <= params.radius)
					addChunk(posRel);
			}
		}
	}
}

void WorldGrid::generateCylinder(const CylinderShapeParams& params)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool((params.radius * 2 + 1) * (params.radius * 2 + 1) * params.height * 2);

	glm::ivec3 pos;

	for (pos.x = -static_cast<int32_t>(params.radius);
		pos.x < static_cast<int32_t>(params.radius + 1); ++pos.x)
	{
		for (pos.z = -static_cast<int32_t>(params.radius);
			pos.z < static_cast<int32_t>(params.radius + 1); ++pos.z)
		{
			for (pos.y = 0; pos.y < static_cast<int64_t>(params.height); ++pos.y)
			{
				glm::ivec3 posRel = pos + params.bottomCenterPosition;
				if (glm::length(glm::vec2(pos.x, pos.z)) <= params.radius)
					addChunk(posRel);
			}
		}
	}
}

void WorldGrid::generateParallelepiped(const ParallelepipedShapeParams& params)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool(params.width * params.height * params.depth * 2);
	glm::ivec3 pos;

	for (pos.x = params.cornerPosition.x; pos.x < params.cornerPosition.x + static_cast<int32_t>(params.width); ++pos.x)
		for (pos.z = params.cornerPosition.z; pos.z < params.cornerPosition.z + static_cast<int32_t>(params.depth); ++pos.z)
			for (pos.y = params.cornerPosition.y; pos.y < params.cornerPosition.y + static_cast<int32_t>(params.height); ++pos.y)
				addChunk(pos);
}

void WorldGrid::generateCube(const CubeShapeParams& params) {
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool(params.edge * params.edge * params.edge * 2);
	glm::ivec3 pos;

	for (pos.x = params.cornerPosition.x; pos.x < params.cornerPosition.x + static_cast<int32_t>(params.edge); ++pos.x)
		for (pos.z = params.cornerPosition.z; pos.z < params.cornerPosition.z + static_cast<int32_t>(params.edge); ++pos.z)
			for (pos.y = params.cornerPosition.y; pos.y < params.cornerPosition.y + static_cast<int32_t>(params.edge); ++pos.y)
				addChunk(pos);
}

void WorldGrid::sortAllocationsByDistance(glm::ivec3 centerPos)
{	
	std::sort(m_allocations.begin(), m_allocations.end(),
		[&centerPos](const GridPool::Allocation& left, const GridPool::Allocation& right) -> bool {
			float lengthRight = glm::length(glm::vec3(glm::ivec3(left.getField<1>().coord) - centerPos));
			float lengthLeft = glm::length(glm::vec3(glm::ivec3(right.getField<1>().coord) - centerPos));
			return lengthLeft > lengthRight;
		});

	m_coordToAllocation.clear();
	for (size_t i = 0; i < m_allocations.size(); ++i)
		m_coordToAllocation.insert({ glm::ivec3(m_allocations[i].getField<1>().coord), i });
}