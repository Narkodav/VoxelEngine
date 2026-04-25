#pragma once
#include "JsonParser/Value.h"

#include <vector>

#include "Rendering/AssetCache.h"
#include "GameData/Voxel.h"
#include "GameData/EngineFilesystem.h"

#include "SDK/ResourceCacheInterface.h"

class ResourceCache : public Modular::ModuleBase<ResourceCache>
{
private:
	using VoxelCache = Id::NamedCache<Voxel, Id::Voxel>;
	using VoxelStateCache = Id::NamedCache<Voxel::State, Id::VoxelState>;
	VoxelCache m_voxels;
	VoxelStateCache m_voxelStates;
	AssetCache m_assetCache;

	ResourceCacheInterface m_interface;

public:

	ResourceCache() {
		m_interface.getVoxelId = this->bind<&ResourceCache::getVoxelId>();
		m_interface.getVoxelStateId = this->bind<&ResourceCache::getVoxelStateId>();
		m_interface.self = this;
	}

	VoxelId getVoxelId(const char* name) { return m_voxels.getId(name); }
	VoxelStateId getVoxelStateId(const char* name) { return m_voxelStates.getId(name); }

	void registerResources(const EngineFilesystem& engineFiles);
	void registerVoxel(std::string_view path);

	auto& getAssetCache() { return m_assetCache; };
	const auto& getAssetCache() const { return m_assetCache; };

	auto& getVoxelCache() { return m_voxels; };
	const auto& getVoxelCache() const { return m_voxels; };

	auto& getVoxelStateCache() { return m_voxelStates; };
	const auto& getVoxelStateCache() const { return m_voxelStates; };

	ResourceCacheInterface* getInterface() { return &m_interface; }
};

