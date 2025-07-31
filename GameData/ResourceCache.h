#pragma once
#include "JsonParser/Parser.h"
#include "JsonParser/Document.h"

#include <vector>

#include "Rendering/AssetCache.h"
#include "Voxel.h"

class ResourceCache
{
private:
	using VoxelCache = Id::NamedCache<Voxel, Id::Voxel>;
	using VoxelStateCache = Id::NamedCache<Voxel::State, Id::VoxelState>;
	VoxelCache m_voxels;
	VoxelStateCache m_voxelStates;
	AssetCache m_assetCache;

public:

	void registerResources(const std::string& voxelsPath);
	void registerVoxel(const std::string& path);

	auto& getAssetCache() { return m_assetCache; };
	const auto& getAssetCache() const { return m_assetCache; };

	auto& getVoxelCache() { return m_voxels; };
	const auto& getVoxelCache() const { return m_voxels; };

	auto& getVoxelStateCache() { return m_voxelStates; };
	const auto& getVoxelStateCache() const { return m_voxelStates; };
};

