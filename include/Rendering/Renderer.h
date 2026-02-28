#pragma once
#include "Graphics/Graphics.h"

#include "Rendering/ShaderCache.h"
#include "Rendering/StorageCache.h"
#include "Rendering/ShaderLayoutDefinitions.h"

#include "GameData/ResourceCache.h"
#include "GameData/EngineFilesystem.h"

#include "WorldManagement/WorldGrid.h"
#include "Rendering/DebugConsole.h"
#include "Rendering/Compass.h"
#include "Utility/StructOfArraysPool.h"

#include "MultiThreading/ThreadPool.h"

#include "Common.h"

//layout(set = 2, binding = 0) uniform Config {
//    float contrast;         // 0.0 = grayscale, 1.0 = normal, >1.0 = increased contrast
//    uint bufferSize;        //in indices, not bytes, current index buffer size
//    uint bufferThreshold;   //in indices, not bytes, current index buffer threshold
//} config;
//
//layout(set = 2, binding = 1, std430) writeonly buffer Usage {
//    uint bufferUsage;       // Current usage written by compute shader
//    uint needsExpansion;    // 1 if threshold exceeded
//    uint overflowed;        // 1 if buffer size exceeded
//} usage;

class Renderer
{
public:
	using GridMapping = std::span<Id::VoxelState>;
	using ChunkMapping = std::span<WorldGrid::Chunk>;

private:
	struct Config {
		float contrast;
	};

	struct PushConstants {
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 viewProj;
	};

	struct RangeStarts
	{
		uint32_t startPolygon;
		uint32_t startColoring;
	};

	struct PerFrameObjects
	{
		Gfx::CommandBuffer graphicsCommandBuffer;
		Gfx::Semaphore imageAvailableSemaphore;
		Gfx::Semaphore renderFinishedSemaphore;
		Gfx::Fence inFlightFence;
	};

	struct PoolDrawCommand {
		Gfx::DrawCommand drawCommand;
		uint32_t bufferId;
	};

	PushConstants m_pushConstants;

	size_t m_frameCounter = 0;
	size_t m_currentFrame = 0;
	float m_deltaTime = 0.0f;

	static inline const size_t s_stagingMemorySize = 1024 * 1024;

	static inline const size_t s_framesInFlight = 1;
	std::array<PerFrameObjects, s_framesInFlight> m_perFrameInFlightObjects;

	Gfx::Wrappers::Instance m_instance;
	Gfx::PhysicalDevice m_physicalDevice;
	Gfx::Wrappers::Device m_device;
	Gfx::RenderRegion m_canvas;

	Gfx::PhysicalDeviceMemoryProperties m_deviceMemoryProps;

	Gfx::Utility::RenderPassData m_renderPassData;
	Gfx::Utility::SwapChainData m_swapChainData;
	Gfx::Utility::GraphicsPipelineData m_graphicsPipelineData;
	//Gfx::ComputePipeline m_computePipeline;

	Gfx::CommandPool m_graphicsCommandPool;
	Gfx::CommandPool m_temporaryBufferPool;
	Gfx::DescriptorPool m_descriptorPool;
	Gfx::DescriptorPool m_imguiDescriptorPool;

	Gfx::Queue m_graphicsQueue;
	Gfx::Queue m_presentQueue;
	Gfx::Queue m_transferQueue;

	ShaderCache m_shaderCache;

	enum class DescriptorSetIndex {
		Storage = 0,
		PerChunk,
		Pool,
		Config,
		Count
	};

	std::array<Gfx::DescriptorSetLayout, static_cast<size_t>(DescriptorSetIndex::Count)> m_descriptorSetLayouts;
	std::array<Gfx::DescriptorSetLayoutCreateInfo, static_cast<size_t>(DescriptorSetIndex::Count)> m_descriptorLayoutCreateInfos;
	std::vector<Gfx::DescriptorSet> m_descriptorSets;

	Gfx::Memory m_stagingMemory;
	Gfx::Buffer m_stagingBuffer;
	Gfx::MemoryMapping m_stagingMapping;
	size_t m_stagingMemorySize = 0;

	Gfx::Sampler m_sampler;
	Gfx::Surface m_surface;

	// per chunk data
	Gfx::Buffer m_gridBuffer;
	Gfx::Buffer m_chunkBuffer;
	Gfx::Buffer m_drawCommandsBuffer;

	Gfx::Memory m_gridMemory;
	Gfx::MemoryMapping m_gridMapping;

	Gfx::Memory m_chunkMemory;

	Gfx::Memory m_drawCommandsMemory;
	Gfx::MemoryMapping m_drawCommandsMapping;

	Gfx::MemoryManagement::MemoryPool m_indicesPool;

	Gfx::Memory m_configMemory;
	Gfx::MemoryMapping m_configMapping;

	Gfx::Buffer m_configBuffer;

	size_t m_chunkCount = 0;

	MT::ThreadPool* m_poolHandle = nullptr;

	std::mutex m_poolLock;
	std::vector<std::unique_ptr<std::mutex>> m_chunkDrawLocks;
	std::vector<bool> m_meshedChunks;
	/*std::vector<PoolDrawCommand> m_drawCommands;*/
	std::shared_mutex m_drawLock;

	//uint32_t* m_errorCount;
	//Error* m_errorMapping;

	
	std::vector<Gfx::MemoryManagement::MemoryPool::Allocation> m_indexAllocations;
	std::vector<std::vector<Indices>> m_stagingBuffers;

	std::mutex m_drawCommandLock;
	std::mutex m_stagingBufferLock;
	std::unordered_map<size_t, size_t> m_drawIndexToPoolIndex;
	std::vector<size_t> m_chunkDrawIndices;
	size_t m_drawCommandAmount;

	DebugConsole m_debugConsole;

public:
	Renderer() = default;
	~Renderer() = default;

	void init(std::string engineName, std::string appName,
		Platform::Window& window, MT::ThreadPool& poolHandle,
		const EngineFilesystem& engineFiles);

	void createAndWriteAssets(AssetCache& assetCache,
		const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStateCache);

	void cleanup(StorageCache& cache);

	void drawFrame(const Gfx::Utility::CameraPerspective& camera);

	void handleResize(const Gfx::Extent2D& extent);

	void resetChunkBuffers(const WorldGrid& grid);

	//GridMapping getGridMapping();
	//ChunkMapping getChunkMapping();

	void updateChunk(const ResourceCache& resources, size_t chunkIndex, const WorldGrid& grid, size_t threadId);
	void updateChunkAsync(const ResourceCache& resources, size_t chunkIndex, const WorldGrid& grid);

	void unmeshChunk(size_t chunkPoolIndex);

	void dumpHandles();
private:
	void createLayouts();
	void configureMemory();
	void drawGui(const Gfx::Utility::CameraPerspective& camera);
	void drawMemoryPoolVisualization(size_t chunkIndex);

	void onPoolBufferAlloc(Gfx::MemoryRef memory, Gfx::BufferRef buffer, size_t bufferIndex);
};

