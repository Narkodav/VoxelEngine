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

#include "Rendering/Shaders/ShaderCache.h"
#include "Rendering/StorageCache.h"
#include "Rendering/ShaderLayoutDefinitions.h"
#include "Rendering/AssetCache.h"
#include "WorldManagement/VoxelVolume.h"

class Renderer
{
public:
	using ChunkDataMapping = std::span<Id::VoxelState>;
	using ChunkCoordMapping = std::span<glm::vec4>;

private:
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
		Gfx::Buffer uniformBuffer;
		Gfx::DescriptorSetHandle perFrameSet;
	};

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
	Gfx::ComputePipeline m_computePipeline;
	Gfx::CommandPool m_graphicsCommandPool;
	Gfx::CommandPool m_temporaryBufferPool;
	Gfx::DescriptorPool m_descriptorPool;
	Gfx::DescriptorPool m_imguiDescriptorPool;

	Gfx::Queue m_graphicsQueue;
	Gfx::Queue m_presentQueue;
	Gfx::Queue m_transferQueue;

	ShaderCache m_shaderCache;

	Gfx::DescriptorSetLayout m_perFrameLayout;
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
	Gfx::Buffer m_chunkDataBuffer;
	Gfx::Buffer m_rangeStartsBuffer;
	Gfx::Buffer m_drawCommandsBuffer;
	Gfx::Buffer m_chunkPositionBuffer;

	Gfx::MappedMemory m_chunkDataMemory;
	Gfx::Memory m_rangeStartsMemory;
	Gfx::Memory m_drawCommandsMemory;
	Gfx::MappedMemory m_chunkPositionMemory;

	Gfx::Buffer m_indexBuffer;
	Gfx::Memory m_indexMemory;

	static inline const std::array<uint32_t, 3> indices = { 0, 1, 2 };

public:
	Renderer() = default;
	~Renderer() { cleanup(); };

	void init(std::string engineName, std::string appName, Window& window);

	void createAndWriteAssets(AssetCache& assetCache,
		const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStateCache);

	void cleanup();

	void drawFrame(const Gfx::CameraPerspective& camera);

	void handleResize(const Gfx::Extent2D& extent);

	void updatePerFrameUniform(const Gfx::CameraPerspective& camera);	

	void setupPerChunkBuffers();

	ChunkDataMapping getChunkDataMapping();
	ChunkCoordMapping getChunkCoordMapping();

private:
	void createLayouts();
	void drawGui(const Graphics::CameraPerspective& camera);
};

