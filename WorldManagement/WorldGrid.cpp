#include "WorldGrid.h"

WorldGrid::WorldGrid(size_t sphereRadius, glm::ivec3 centerPos)
{
	resetSphereRadius(sphereRadius, centerPos);
}

void WorldGrid::resetSphereRadius(size_t sphereRadius, glm::ivec3 centerPos)
{
	m_sphereRadius = sphereRadius;
	m_chunks.clear();
	m_grid.clear();
	m_coordToChunk.clear();
	uint32_t gridCursor = 0;

	for (int32_t x = -static_cast<int32_t>(sphereRadius);
		x < static_cast<int32_t>(sphereRadius + 1); ++x)
	{
		for (int32_t z = -static_cast<int32_t>(sphereRadius);
			z < static_cast<int32_t>(sphereRadius + 1); ++z)
		{
			for (int32_t y = -static_cast<int32_t>(sphereRadius);
				y < static_cast<int32_t>(sphereRadius + 1); ++y)
			{
				glm::ivec3 coord;
				uint32_t start;
				uint32_t neighbourStarts[6];

				glm::ivec3 pos = glm::ivec3(x, y, z) + centerPos;
				if (glm::length(glm::vec3(pos)) <= sphereRadius)
				{
					m_chunks.push_back(Chunk{ glm::ivec4(pos, 1), gridCursor, {0} });
					m_coordToChunk.insert({ pos, m_chunks.size() - 1 });
					gridCursor += Constants::chunkSize;
				}
			}
		}
	}

	m_grid.resize(gridCursor);

	for(size_t i = 0; i < m_chunks.size(); ++i)
	{
		Chunk& chunk = m_chunks[i];
		for(size_t j = 0; j < 6; ++j)
		{
			glm::ivec3 neighbourPos = glm::ivec3(chunk.coord) + Constants::directions3D[j];
			auto it = m_coordToChunk.find(neighbourPos);
			if (it != m_coordToChunk.end())
				chunk.neighbourStarts[j] = m_chunks[it->second].start;
			else chunk.neighbourStarts[j] = m_grid.size();
		}
	}
}