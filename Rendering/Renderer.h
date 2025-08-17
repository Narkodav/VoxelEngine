#pragma once
#include "Graphics/DeviceCaching/PhysicalDeviceCache.h"
#include "Graphics/Rendering/Device.h"
#include "Graphics/Rendering/CommandBuffer.h"
#include "Graphics/Rendering/CommandPool.h"
#include "Graphics/Rendering/Semaphore.h"
#include "Graphics/Rendering/Fence.h"
#include "Graphics/MemoryManagement/Buffer.h"
#include "Graphics/Rendering/Queue.h"
#include "Graphics/MemoryManagement/Memory.h"
#include "Graphics/MemoryManagement/MappedMemory.h"
#include "Graphics/Camera.h"
#include "Graphics/Rendering/Surface.h"
#include "Graphics/Rendering/Pipeline.h"
#include "Graphics/Rendering/ComputePipeline.h"
#include "Graphics/MemoryManagement/MemoryPool.h"

#include "Rendering/Shaders/ShaderCache.h"
#include "Rendering/StorageCache.h"
#include "Rendering/ShaderLayoutDefinitions.h"
#include "GameData/ResourceCache.h"
#include "WorldManagement/WorldGrid.h"
#include "Rendering/DebugConsole.h"

#include "Multithreading/ThreadPool.h"

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

	//const uint noError = 0;
	//const uint errorChunkIndexOutOfBounds = 1;
	//const uint errorBlockIndexOutOfBounds = 2;
	//const uint errorStateIdOutOfBounds = 3;
	//const uint errorModelIdOutOfBounds = 4;
	//const uint errorGeometryIdOutOfBounds = 5;
	//const uint errorAppearenceIdOutOfBounds = 6;
	//const uint errorGeometrySizeIsZero = 7;

	enum class ShaderErrors : uint32_t {
		None = 0,
		ChunkIndexOutOfBounds = 1,
		BlockIndexOutOfBounds = 2,
		StateIdOutOfBounds = 3,
		ModelIdOutOfBounds = 4,
		GeometryIdOutOfBounds = 5,
		AppearenceIdOutOfBounds = 6,
		GeometrySizeIsZero = 7,
	};

	static inline const std::array<std::string, 8> shaderErrorMessages = {
		"No error",
		"Chunk index out of bounds",
		"Block index out of bounds",
		"State id out of bounds",
		"Model id out of bounds",
		"Geometry id out of bounds",
		"Appearance id out of bounds",
		"Geometry size is zero"
	};

	struct Error {
		ShaderErrors errorCode;
		uint32_t localInvocationId;
		uint32_t globalInvocationId;
		uint32_t workGroupId;
	};

	struct PushConstants {
		glm::mat4 view;
		glm::mat4 proj;
		uint32_t chunkCount;
	};

	struct RangeStarts
	{
		uint32_t startPolygon;
		uint32_t startColoring;
	};

	struct PerFrameObjects
	{
		Gfx::CommandBufferHandle graphicsCommandBuffer;
		Gfx::Semaphore imageAvailableSemaphore;
		Gfx::Semaphore renderFinishedSemaphore;
		Gfx::Fence inFlightFence;
	};

	uint32_t m_frameCounter = 0;
	uint32_t m_currentFrame = 0;
	float m_deltaTime = 0.0f;

	static inline const uint32_t stagingMemorySize = 1024 * 1024;

	static inline const uint32_t framesInFlight = 1;
	std::array<PerFrameObjects, framesInFlight> m_perFrameInFlightObjects;

	Gfx::DescriptorSetHandle m_storageSet;
	Gfx::DescriptorSetHandle m_perChunkSet;
	Gfx::DescriptorSetHandle m_configSet;

	Gfx::Context m_context;
	Gfx::PhysicalDeviceCache m_deviceCache;
	const Gfx::PhysicalDevice* m_physicalDevice;
	Gfx::Device m_device;
	Gfx::RenderRegion m_canvas;
	Gfx::SwapChainFormat m_format;
	Gfx::SwapChain m_swapChain;
	Gfx::RenderPass m_renderPass;
	Gfx::Pipeline m_pipeline;
	//Gfx::ComputePipeline m_computePipeline;
	Gfx::CommandPool m_graphicsCommandPool;
	Gfx::CommandPool m_temporaryBufferPool;
	Gfx::DescriptorPool m_descriptorPool;
	Gfx::DescriptorPool m_imguiDescriptorPool;

	Gfx::Queue m_graphicsQueue;
	Gfx::Queue m_presentQueue;
	Gfx::Queue m_transferQueue;

	ShaderCache m_shaderCache;

	Gfx::DescriptorSetLayout m_storageLayout;
	Gfx::DescriptorSetLayout m_perChunkLayout;
	Gfx::DescriptorSetLayout m_configLayout;

	Gfx::MappedMemory m_uniformMemory;
	Gfx::MappedMemory m_contrastMemory;

	Gfx::Buffer m_contrastBuffer;

	Gfx::MappedMemory m_stagingMemory;
	Gfx::Buffer m_stagingBuffer;

	Gfx::Sampler m_sampler;
	Gfx::UniformTransforms m_transforms;

	Gfx::Surface m_surface;
	Window::WindowEventSubscription m_frameBufferResizeSubscription;

	// per chunk data
	Gfx::Buffer m_gridBuffer;
	Gfx::Buffer m_chunkBuffer;
	Gfx::Buffer m_drawCommandsBuffer;	

	Gfx::MappedMemory m_gridMemory;
	Gfx::MappedMemory m_chunkMemory;
	Gfx::MappedMemory m_drawCommandsMemory;

	Gfx::MemoryPool<Gfx::MappedMemory> m_indicesPool;

	Gfx::MappedMemory m_configMemory;
	Gfx::Buffer m_configBuffer;

	Gfx::Buffer m_indexBuffer;
	Gfx::Memory m_indexMemory;

	static inline const std::array<uint32_t, 3> indices = { 0, 1, 2 };

	size_t m_chunkCount = 0;

	MT::ThreadPool* m_poolHandle = nullptr;

	std::mutex m_poolLock;
	std::vector<std::unique_ptr<std::mutex>> m_chunkDrawLocks;
	std::vector<std::pair<Gfx::DrawCommand, size_t>> m_drawCommands;
	std::shared_mutex m_drawLock;

	Config* m_configMapping;
	//uint32_t* m_errorCount;
	//Error* m_errorMapping;

	
	std::vector<Gfx::MemoryPool<Gfx::MappedMemory>::Allocation> m_indexAllocations;
	std::vector<Gfx::CommandBufferHandle> m_perChunkBuffers;
	std::vector<std::vector<Indices>> m_stagingBuffers;

	DebugConsole m_debugConsole;

public:
	Renderer() = default;
	~Renderer() { cleanup(); };

	void init(std::string engineName, std::string appName,
		Window& window, MT::ThreadPool& poolHandle);

	void createAndWriteAssets(AssetCache& assetCache,
		const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStateCache);

	void cleanup();

	void drawFrame(const Gfx::CameraPerspective& camera);

	void handleResize(const Gfx::Extent2D& extent);

	void resetChunkBuffers(size_t chunkAmount);

	//GridMapping getGridMapping();
	//ChunkMapping getChunkMapping();

	void updateChunk(const ResourceCache& resources, size_t chunkIndex,
		const WorldGrid::Chunk& chunk, const WorldGrid::Grid& grid, size_t threadId);
	void updateChunkAsync(const ResourceCache& resources, size_t chunkIndex,
		const WorldGrid::Chunk& chunk, const WorldGrid::Grid& grid);
private:
	void createLayouts();
	void createConfigBuffers();
	void drawGui(const Graphics::CameraPerspective& camera);
	void drawMemoryPoolVisualization(size_t chunkIndex);
};

