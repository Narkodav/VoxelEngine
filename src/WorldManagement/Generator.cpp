#include "WorldManagement/Generator.h"

Generator::Generator()
{
	m_seed = 0;
}

void Generator::set(SeedType seed)
{ 
	m_seed = seed; 
	m_perlinNoise2d.setSeed(m_seed);
	m_perlinNoise3d1.setSeed(m_seed);
	m_perlinNoise3d2.setSeed(m_seed ^ std::numeric_limits<SeedType>::max());
	m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Air)] = 0;
	m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Dirt)] = 1;
}

bool Generator::shouldBeCave(int32_t x, int32_t y, int32_t z)
{
	float caveDensity = (m_perlinNoise3d1.getFbm(x, y, z, 3, 0.04f) + 1) / 2;
	caveDensity *= caveDensity;
	// Create the worm effect by checking if noise is within a specific range

	// Check if the noise value is within our "worm" range
	if (caveDensity >= 0.4f) return 0;

	caveDensity += pow((m_perlinNoise3d2.getFbm(x, y, z, 3, 0.04f) + 1) / 2, 2);
	if (caveDensity >= 0.4f) return 0;

	return 1;
}

void Generator::setChunkData(WorldGrid& grid, size_t allocIndex)
{
	auto& alloc = grid.getAllocatedChunks()[allocIndex];
	auto& chunk = alloc.getField<1>();
	auto blocks = alloc.getField<0>();

	glm::ivec3 coords000 = chunk.coordCorner;
	size_t height = 0;
	float amplitudeHight = 16.f;
	//float density;

	for (size_t x = 0; x < Constants::chunkWidth; x++)
		for (size_t z = 0; z < Constants::chunkDepth; z++)
		{
			height = m_groundLevel + amplitudeHight * m_perlinNoise2d.getFbm(x + coords000.x, 
				z + coords000.z, 3, 0.02f);
			size_t yEnd;

			if (coords000.y + Constants::chunkHeight < height)
				yEnd = Constants::chunkHeight;
			else if (coords000.y < static_cast<int64_t>(height))
				yEnd = height - coords000.y;
			else yEnd = 0;

			for (size_t y = 0; y < yEnd; y++)
			{
				if (shouldBeCave(x + coords000.x, y + coords000.y, z + coords000.z))
					blocks[y * Constants::chunkLayerSize + z * Constants::chunkDepth + x] =
					m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Air)];
				else blocks[y * Constants::chunkLayerSize + z * Constants::chunkDepth + x] =
					m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Dirt)];
			}
			for (size_t y = yEnd; y < Constants::chunkHeight; y++)
				blocks[y * Constants::chunkLayerSize + z * Constants::chunkDepth + x] =
				m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Air)];
		}
}

void Generator::fillChunk(WorldGrid& grid, size_t allocIndex, BlockTypes type) {
	auto& alloc = grid.getAllocatedChunks()[allocIndex];
	auto blocks = alloc.getField<0>();

	auto blockId = m_relevantBlockIds[static_cast<uint32_t>(type)];
	std::fill(blocks.begin(), blocks.end(), blockId);
}