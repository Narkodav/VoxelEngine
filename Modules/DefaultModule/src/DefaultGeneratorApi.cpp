#include "DefaultGeneratorApi.h"

static DefaultPluginApi s_defaultPluginApi;

extern "C"
{
    MODULAR_PLUGIN_API Modular_PluginInterface* Modular_createPlugin() {
        //std::cout << "Default plugin created" << std::endl;
        return s_defaultPluginApi.interface();
    }

    MODULAR_PLUGIN_API void Modular_destroyPlugin(Modular_PluginInterface*) {
        //std::cout << "Default plugin destroyed" << std::endl;
    }
}

void Generator::init(GeneratorSeed seed, ResourceCacheInterface* resources) {
	m_seed = seed; 
	m_perlinNoise2d.setSeed(m_seed);
	m_perlinNoise3d1.setSeed(m_seed);
	m_perlinNoise3d2.setSeed(m_seed ^ std::numeric_limits<SeedType>::max());
    
	m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Air)] = resources->getVoxelStateId(resources->self, "empty");
	m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Dirt)] = resources->getVoxelStateId(resources->self, "dirt_block");
	m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Grass)] = resources->getVoxelStateId(resources->self, "grass_block");
}

bool Generator::shouldBeCave(int32_t x, int32_t y, int32_t z) {
	float caveDensity = (m_perlinNoise3d1.getFbm(x, y, z, 3, 0.04f) + 1) / 2;
	caveDensity *= caveDensity;
	// Create the worm effect by checking if noise is within a specific range

	// Check if the noise value is within our "worm" range
	if (caveDensity >= 0.4f) return 0;

	caveDensity += pow((m_perlinNoise3d2.getFbm(x, y, z, 3, 0.04f) + 1) / 2, 2);
	if (caveDensity >= 0.4f) return 0;

	return 1;
}

void Generator::setChunkData(WorldGridInterface* grid, ChunkIndex chunk) {
    
	auto blocks = grid->getChunkBlocks(grid->self, chunk);
	glm::ivec3 coords000 = grid->getChunkCornerCoord(grid->self, chunk);

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
					m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Grass)];
			}
			for (size_t y = yEnd; y < Constants::chunkHeight; y++)
				blocks[y * Constants::chunkLayerSize + z * Constants::chunkDepth + x] =
				m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Air)];
		}
}