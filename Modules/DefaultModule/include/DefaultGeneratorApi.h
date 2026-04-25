#pragma once
#define MODULAR_PLUGIN_IMPLEMENTATION
#include "Common.h"

#include "WorldManagement/WorldGrid.h"
#include "GameData/ResourceCache.h"

#include "CommonApi/Mathematics/PerlinNoise2d.h"
#include "CommonApi/Mathematics/PerlinNoise3d.h"

#include "SDK/GeneratorInterface.h"

#include <random>
#include <unordered_set>
#include <string_view>

class Generator : public Modular::ModuleBase<Generator> {
public:

	enum class BlockTypes : uint32_t
	{
		Air = 0,
		Dirt = 1,
		Grass = 2,
		Count,
	};

private:
    GeneratorInterface m_generatorInterface;

	using SeedType = uint64_t;
	Math::PerlinNoise2d m_perlinNoise2d;
	Math::PerlinNoise3d m_perlinNoise3d1;
	Math::PerlinNoise3d m_perlinNoise3d2;

	Id::VoxelState m_relevantBlockIds[static_cast<uint32_t>(BlockTypes::Count)];

	static const int32_t m_groundLevel = 200;
	SeedType m_seed;
public:
	Generator() {
        m_generatorInterface.setChunkData = this->bind<&Generator::setChunkData>();
        m_generatorInterface.init = this->bind<&Generator::init>();
        m_generatorInterface.self = this;
        m_seed = 0;
    }

	Generator(const Generator& other) = delete;
	Generator& operator=(const Generator& other) = delete;
    Generator(Generator&& other) = delete;
	Generator& operator=(Generator&& other) = delete;

	void init(GeneratorSeed seed, ResourceCacheInterface* resources);

	bool shouldBeCave(int32_t x, int32_t y, int32_t z);

	void setChunkData(WorldGridInterface* grid, ChunkIndex chunk);

    GeneratorInterface* getInterface() {
        return &m_generatorInterface;
    }
};

class GeneratorApi : public Modular::ServiceApiBase<GeneratorApi, "DefaultGenerator", GeneratorApiInterface> {
private:
    std::unordered_set<Generator*> m_generators;
public:

    GeneratorApi() {
        this->m_interface.createGenerator = this->bind<&GeneratorApi::createGenerator>();
        this->m_interface.destroyGenerator = this->bind<&GeneratorApi::destroyGenerator>();
        this->m_interface.self = this;
    };

    ~GeneratorApi() {
        if(!m_generators.empty()) throw std::runtime_error("Some generator objects were not destroyed");
    }

    GeneratorInterface* createGenerator() {
        auto* gen = new Generator();
        m_generators.insert(gen);
        return gen->getInterface();
    }

    void destroyGenerator(GeneratorInterface* generator) {
        auto* gen = reinterpret_cast<Generator*>(generator->self);
        m_generators.erase(gen);
        delete gen;
    }
};

class DefaultPluginApi : public Modular::PluginApiBase<"DefaultModule", GeneratorApi> {
public:
    uint64_t version() override { return 1; }
    void onUnload() override {}

protected:
    void onLoadImpl(Modular_OnLoadHostInterface* host) override {}
};