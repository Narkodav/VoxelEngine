#include "WorldManagement/WorldGrid.h"

WorldGrid::WorldGrid(size_t radius, glm::ivec3 centerPos)
{
	generateSphere(radius, centerPos);
}

WorldGrid::WorldGrid(size_t radius, size_t height, glm::ivec3 centerPos)
{
	generateCylinder(radius, height, centerPos);
}

void WorldGrid::generateSphere(size_t radius, glm::ivec3 centerPos)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool(radius * radius * radius * 8);

	glm::ivec3 pos;

	for (pos.x = -static_cast<int32_t>(radius);
		pos.x < static_cast<int32_t>(radius + 1); ++pos.x)
	{
		for (pos.z = -static_cast<int32_t>(radius);
			pos.z < static_cast<int32_t>(radius + 1); ++pos.z)
		{
			for (pos.y = -static_cast<int32_t>(radius);
				pos.y < static_cast<int32_t>(radius + 1); ++pos.y)
			{
				glm::ivec3 posRel = pos + centerPos;
				if (glm::length(glm::vec3(pos)) <= radius)
					addChunk(posRel);
			}
		}
	}
}

void WorldGrid::generateCylinder(size_t radius, size_t height, glm::ivec3 centerPos)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool(radius * radius * height * 4);

	glm::ivec3 pos;

	for (pos.x = -static_cast<int32_t>(radius);
		pos.x < static_cast<int32_t>(radius + 1); ++pos.x)
	{
		for (pos.z = -static_cast<int32_t>(radius);
			pos.z < static_cast<int32_t>(radius + 1); ++pos.z)
		{
			for (pos.y = 0; pos.y < static_cast<int64_t>(height); ++pos.y)
			{
				glm::ivec3 posRel = pos + centerPos;
				if (glm::length(glm::vec2(pos.x, pos.z)) <= radius)
					addChunk(posRel);
			}
		}
	}
}

void WorldGrid::generateParallelogram(size_t width, size_t height, size_t depth, glm::ivec3 cornerPos)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool(width * height * depth);
	glm::ivec3 pos;

	for (pos.x = cornerPos.x; pos.x < cornerPos.x + static_cast<int32_t>(width); ++pos.x)
		for (pos.z = cornerPos.z; pos.z < cornerPos.z + static_cast<int32_t>(depth); ++pos.z)
			for (pos.y = cornerPos.y; pos.y < cornerPos.y + static_cast<int32_t>(height); ++pos.y)
				addChunk(pos);
}

void WorldGrid::generateCube(size_t edge, glm::ivec3 cornerPos) {
	generateParallelogram(edge, edge, edge, cornerPos);
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