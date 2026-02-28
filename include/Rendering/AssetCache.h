#pragma once
#include "JsonParser/Value.h"
#include "Graphics/Graphics.h"

#include <vector>
#include <fstream>
#include <unordered_map>
#include <filesystem>

#include "Rendering/StorageCache.h"
#include "Rendering/VoxelCullingCache.h"

#include "GameData/EngineFilesystem.h"

class AssetCache
{
public:
	using ModelCache	= Id::NamedCache<Shape::Model, Id::Model>;

	using PolygonCache	= Id::Cache<Shape::Polygon, Id::Polygon>;
	using ColoringCache = Id::Cache<Shape::Coloring, Id::Coloring>;

	using VertexCache	= Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>;
	using UvCache		= Id::Cache<glm::vec2, Id::Uv, VecEpsilonEqualComparator<glm::vec2>>;
	using NormalCache	= Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>;

	using TextureCache	= Id::NamedCache<Gfx::Utility::PixelData2D, Id::Texture>;

	struct AssetStatistics {
		size_t textureBytes;
		size_t vertexBytes;
		size_t uvBytes;
		size_t polygonBytes;
		size_t coloringBytes;
		size_t geometryIndexBytes;
		size_t geometryEntryBytes;
		size_t appearanceIndexBytes;
		size_t appearanceEntryBytes;
		size_t modelBytes;

		size_t totalModelBytes;
		size_t totalTextureBytes;
		size_t totalBytes;

		size_t textureCount;
		size_t vertexCount;
		size_t uvCount;
		size_t polygonCount;
		size_t coloringCount;
		size_t geometryIndexCount;
		size_t geometryEntryCount;
		size_t appearanceIndexCount;
		size_t appearanceEntryCount;
		size_t modelCount;
	};

private:
	TextureCache m_textureCache;
	VertexCache m_vertexCache;
	UvCache m_uvCache;
	NormalCache m_normalCache;
	PolygonCache m_polygonCache;
	ColoringCache m_coloringCache;
	Shape::PolygonIndexBuffer m_geometryCache;
	Shape::ColoringIndexBuffer m_appearanceCache;
	ModelCache m_modelCache;

	VoxelCullingCache m_voxelCullingCache;
	StorageCache m_storageCache;

	Shape::GeometryId m_standartBlockGeometryId;

public:
	void init(const EngineFilesystem& engineFiles);

	void clearCache(const Gfx::Wrappers::Device& device);

	void registerAssets(const EngineFilesystem& engineFiles);

	void registerVoxelModel(std::string_view path);

	void registerTexture(std::string_view texturePath, std::string_view textureName);

	void moveAssetsToGpuStorage(const Gfx::Wrappers::Device& device,
		const Gfx::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
		Gfx::Queue transferQueue, Gfx::CommandPoolRef temporaryPool,
		Gfx::BufferRef stagingBuffer, Gfx::MemoryMapping& stagingMapping, size_t stagingBuffeSize,
		const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStateCache);

	void writeToDescriptors(const Gfx::Wrappers::Device& device,
		Gfx::DescriptorSet descriptorStorage, Gfx::SamplerRef sampler,
		uint32_t bindingVertex, uint32_t bindingUv, uint32_t bindingNormal, uint32_t bindingPolygon,
		uint32_t bindingColoring, uint32_t bindingPolygonIndex, uint32_t bindingColoringIndex,
		uint32_t bindingGeometry, uint32_t bindingAppearence, uint32_t bindingModel,
		uint32_t bindingStateToModel, uint32_t bindingImage) const {
		m_storageCache.writeToDescriptors(device,
			descriptorStorage, sampler, bindingVertex, bindingUv, bindingNormal,
			bindingPolygon, bindingColoring, bindingPolygonIndex,
			bindingColoringIndex, bindingGeometry, bindingAppearence, bindingModel,
			bindingStateToModel, bindingImage);
	}

	const auto& getTextureCache() const {
		return m_textureCache;
	}

	const auto& getVertexCache() const {
		return m_vertexCache;
	}

	const auto& getUvCache() const {
		return m_uvCache;
	}

	const auto& getPolygonCache() const {
		return m_polygonCache;
	}

	const auto& getColoringCache() const {
		return m_coloringCache;
	}

	const auto& getGeometryCache() const {
		return m_geometryCache;
	}

	const auto& getAppearanceCache() const {
		return m_appearanceCache;
	}

	const auto& getModelCache() const {
		return m_modelCache;
	}

	const auto& getStorageCache() const {
		return m_storageCache;
	}

	const auto& getVoxelCullingCache() const {
		return m_voxelCullingCache;
	}



	auto& getTextureCache() {
		return m_textureCache;
	}

	auto& getVertexCache() {
		return m_vertexCache;
	}

	auto& getUvCache() {
		return m_uvCache;
	}

	auto& getPolygonCache() {
		return m_polygonCache;
	}

	auto& getColoringCache() {
		return m_coloringCache;
	}

	auto& getGeometryCache() {
		return m_geometryCache;
	}

	auto& getAppearanceCache() {
		return m_appearanceCache;
	}

	auto& getModelCache() {
		return m_modelCache;
	}

	auto& getStorageCache() {
		return m_storageCache;
	}

	auto& getVoxelCullingCache() {
		return m_voxelCullingCache;
	}

	AssetStatistics getStatistics() const {
		AssetStatistics stats = {};

		// Calculate texture data
		stats.textureCount = m_textureCache.size();
		for (size_t i = 0; i < stats.textureCount; ++i) {
			const auto& pixelData = m_textureCache.get(Id::Texture(i));
			stats.textureBytes += pixelData.getPixelData().size();
		}

		// Calculate other cache sizes
		stats.vertexCount = m_vertexCache.size();
		stats.vertexBytes = stats.vertexCount * sizeof(glm::vec4);

		stats.uvCount = m_uvCache.size();
		stats.uvBytes = stats.uvCount * sizeof(glm::vec2);

		stats.polygonCount = m_polygonCache.size();
		stats.polygonBytes = stats.polygonCount * sizeof(Shape::Polygon);

		stats.coloringCount = m_coloringCache.size();
		stats.coloringBytes = stats.coloringCount * sizeof(Shape::Coloring);

		stats.geometryIndexCount = m_geometryCache.indexSize();
		stats.geometryIndexBytes = stats.geometryIndexCount * sizeof(uint32_t);

		stats.geometryEntryCount = m_geometryCache.entrySize();
		stats.geometryEntryBytes = stats.geometryEntryCount
			* sizeof(Shape::PolygonIndexBuffer::Entry);

		stats.appearanceIndexCount = m_appearanceCache.indexSize();
		stats.appearanceIndexBytes = stats.appearanceIndexCount * sizeof(uint32_t);

		stats.appearanceEntryCount = m_appearanceCache.entrySize();
		stats.appearanceEntryBytes = stats.appearanceEntryCount
			* sizeof(Shape::PolygonIndexBuffer::Entry);

		stats.modelCount = m_modelCache.size();
		stats.modelBytes = stats.modelCount * sizeof(Shape::Model);

		// Calculate totals
		stats.totalModelBytes = stats.vertexBytes + stats.uvBytes +
			stats.polygonBytes + stats.coloringBytes + stats.geometryIndexBytes +
			stats.appearanceIndexBytes + stats.modelBytes;

		stats.totalTextureBytes = stats.textureBytes;

		stats.totalBytes = stats.textureBytes + stats.vertexBytes + stats.uvBytes +
			stats.polygonBytes + stats.coloringBytes + stats.geometryIndexBytes +
			stats.appearanceIndexBytes + stats.modelBytes;

		return stats;
	}

	void printStatistics() const {
		AssetStatistics stats = getStatistics();

		auto formatBytes = [](size_t bytes) -> std::string {
			if (bytes >= 1024 * 1024) return std::to_string(bytes / (1024 * 1024)) + " MB";
			if (bytes >= 1024) return std::to_string(bytes / 1024) + " KB";
			return std::to_string(bytes) + " B";
			};

		std::cout << "=== Asset Cache Statistics ===" << std::endl;
		std::cout << "Textures:         " << stats.textureCount << " items, " << formatBytes(stats.textureBytes) << std::endl;
		std::cout << "Vertices:         " << stats.vertexCount << " items, " << formatBytes(stats.vertexBytes) << std::endl;
		std::cout << "UVs:              " << stats.uvCount << " items, " << formatBytes(stats.uvBytes) << std::endl;
		std::cout << "Polygons:         " << stats.polygonCount << " items, " << formatBytes(stats.polygonBytes) << std::endl;
		std::cout << "Colorings:        " << stats.coloringCount << " items, " << formatBytes(stats.coloringBytes) << std::endl;
		std::cout << "Geometry Indices: " << stats.geometryIndexCount << " items, " << formatBytes(stats.geometryIndexBytes) << std::endl;
		std::cout << "Geometry Entries: " << stats.geometryEntryCount << " items, " << formatBytes(stats.geometryEntryBytes) << std::endl;
		std::cout << "Appear. Indices:  " << stats.appearanceIndexCount << " items, " << formatBytes(stats.appearanceIndexBytes) << std::endl;
		std::cout << "Appear. Entries:  " << stats.appearanceEntryCount << " items, " << formatBytes(stats.appearanceEntryBytes) << std::endl;
		std::cout << "Models:           " << stats.modelCount << " items, " << formatBytes(stats.modelBytes) << std::endl;
		std::cout << "======== Total Usage =========" << std::endl;
		std::cout << "Total Models:     " << formatBytes(stats.totalModelBytes) << std::endl;
		std::cout << "Total Textures:   " << formatBytes(stats.totalTextureBytes) << std::endl;
		std::cout << "Total:            " << formatBytes(stats.totalBytes) << std::endl;
	}

private:

	Shape::GeometryType parseShape(const Json::Value& shape,
		std::vector<Id::Polygon>& geometry,
		std::vector<Id::Coloring>& appearence);
};

