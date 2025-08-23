#include "WorldGrid.h"

WorldGrid::WorldGrid(size_t sphereRadius, glm::ivec3 centerPos)
{
	resetSphereRadius(sphereRadius, centerPos);
}

void WorldGrid::resetSphereRadius(size_t sphereRadius, glm::ivec3 centerPos)
{
	m_sphereRadius = sphereRadius;
	m_pool.clear();
	m_allocations.clear();
	m_coordToChunk.clear();
	uint32_t gridCursor = 0;

	m_pool = GridPool(m_sphereRadius * m_sphereRadius * m_sphereRadius * 8);

	glm::ivec3 pos;

	for (pos.x = -static_cast<int32_t>(sphereRadius);
		pos.x < static_cast<int32_t>(sphereRadius + 1); ++pos.x)
	{
		for (pos.z = -static_cast<int32_t>(sphereRadius);
			pos.z < static_cast<int32_t>(sphereRadius + 1); ++pos.z)
		{
			for (pos.y = -static_cast<int32_t>(sphereRadius);
				pos.y < static_cast<int32_t>(sphereRadius + 1); ++pos.y)
			{
				glm::ivec3 posRel = pos + centerPos;
				if (glm::length(glm::vec3(pos)) <= sphereRadius)
				{
					m_allocations.push_back(m_pool.allocate());
					auto& alloc = m_allocations.back();
					auto grid = alloc.getField<0>();
					auto& chunk = alloc.getField<1>();
					chunk.coord = glm::ivec4(posRel, 1);
					chunk.start = alloc.getEntryOffset<0>();
					m_coordToChunk.insert({ posRel, m_allocations.size() - 1});
				}
			}
		}
	}

	for(size_t i = 0; i < m_allocations.size(); ++i)
	{
		Chunk& chunk = m_allocations[i].getField<1>();
		auto grid = m_allocations[i].getField<0>();
		for(size_t j = 0; j < 6; ++j)
		{
			glm::ivec3 neighbourPos = glm::ivec3(chunk.coord) + Constants::directions3D[j];
			auto it = m_coordToChunk.find(neighbourPos);
			if (it != m_coordToChunk.end())
				chunk.neighbourStarts[j] = m_allocations[it->second].getEntryOffset<0>();
			else chunk.neighbourStarts[j] = noChunkIndex;
		}
	}
}