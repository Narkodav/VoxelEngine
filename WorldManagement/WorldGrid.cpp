#include "WorldGrid.h"

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
	uint32_t gridCursor = 0;

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
	uint32_t gridCursor = 0;

	m_pool = GridPool(radius * radius * height * 4);

	glm::ivec3 pos;

	for (pos.x = -static_cast<int32_t>(radius);
		pos.x < static_cast<int32_t>(radius + 1); ++pos.x)
	{
		for (pos.z = -static_cast<int32_t>(radius);
			pos.z < static_cast<int32_t>(radius + 1); ++pos.z)
		{
			for (pos.y = 0; pos.y < height; ++pos.y)
			{
				glm::ivec3 posRel = pos + centerPos;
				if (glm::length(glm::vec2(pos.x, pos.z)) <= radius)
					addChunk(posRel);
			}
		}
	}
}