#include "Renderer.h"

void Renderer::init(std::string engineName, std::string appName,
    Window& window, MT::ThreadPool& poolHandle)
{
    m_poolHandle = &poolHandle;

    m_context = Gfx::Context(engineName, appName,
        Gfx::Version(1, 0, 0), Gfx::Version(1, 0, 0),
        Gfx::DebugMessageSeverity::Bits::Warning
        | Gfx::DebugMessageSeverity::Bits::Error);

    m_deviceCache = Gfx::PhysicalDeviceCache(m_context);

    m_surface = window.createSurface(m_context);

    m_frameBufferResizeSubscription = window.registerCallback<WindowEvents::FrameBufferResized>(
        [this](int width, int height) {
            handleResize(Gfx::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
        });

    Gfx::DeviceRequirements requirements;
    requirements.extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        /*VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME*/ };
    requirements.properties = { 
        {Gfx::DeviceProperty::PhysicalDeviceType, Gfx::PhysicalDeviceType::DiscreteGpu} };
    requirements.features = {
        {Gfx::DeviceFeature::GeometryShader, true},
        {Gfx::DeviceFeature::SamplerAnisotropy, true},
        {Gfx::DeviceFeature::DescriptorBindingPartiallyBound, true},
        {Gfx::DeviceFeature::RuntimeDescriptorArray, true},
        {Gfx::DeviceFeature::ShaderSampledImageArrayNonUniformIndexing, true},
        {Gfx::DeviceFeature::DescriptorBindingSampledImageUpdateAfterBind, true},
        {Gfx::DeviceFeature::DescriptorBindingVariableDescriptorCount, true},
        {Gfx::DeviceFeature::MeshShaderEXT, true},
        {Gfx::DeviceFeature::TaskShaderEXT, true},
        {Gfx::DeviceFeature::ShaderDrawParameters, true},
        {Gfx::DeviceFeature::MultiDrawIndirect, true}, };

    requirements.queueProperties.push_back(Gfx::RequiredQueueProperties());
    requirements.queueProperties.back().queueProperties
        .insert({ Gfx::QueueProperty::QueueFlags, Gfx::QueueFlags::Flags(
            Gfx::QueueFlags::Bits::Graphics | Gfx::QueueFlags::Bits::Transfer) });
    requirements.queueProperties.back().shouldSupportPresent = false;

    requirements.queueProperties.push_back(Gfx::RequiredQueueProperties());
    requirements.queueProperties.back().shouldSupportPresent = true;

    auto result = m_deviceCache.getFittingDevice(m_context, m_surface, requirements);
    if (!result.isSuitable())
        std::cout << "No suitable device found" << std::endl;
    else std::cout << "Suitable device found" << std::endl;
    m_physicalDevice = result.device;

    //search for discrete graphics, present and transfer families
    uint32_t graphicsIndex, presentIndex;

    graphicsIndex = result.queueFamilyIndices[0].front();
    size_t i;

    for (i = 0; i < result.queueFamilyIndices[1].size(); i++)
        if (graphicsIndex != result.queueFamilyIndices[1][i])
        {
            presentIndex = result.queueFamilyIndices[1][i];
            break;
        }
    if (i == result.queueFamilyIndices[1].size()) {
        presentIndex = result.queueFamilyIndices[1].front();
    }

    m_device = Gfx::Device(m_context, *result.device, requirements,
        std::vector<uint32_t>{ graphicsIndex, presentIndex },
        std::vector<uint32_t>{ 2, 1 },
        std::vector<std::vector<float>>({ {1.0f,1.0f},{1.0f} }));

    m_graphicsQueue = Gfx::Queue(m_context, m_device, graphicsIndex, 0);
    m_presentQueue = Gfx::Queue(m_context, m_device, presentIndex, 0);
    m_transferQueue = Gfx::Queue(m_context, m_device, graphicsIndex, 1);

    //canvas just stores dimensions, so it doesn't need to have a dedicated delete function
    //but it still has to be updated at window resize
    m_canvas = Gfx::RenderRegion::createFullWindow(window.getWindowExtent());
    m_format = Gfx::SwapChainFormat::create(m_context, m_device,
        m_surface, window.getFrameBufferExtent());

    m_renderPass = Gfx::RenderPass(m_context, m_device, m_format);

    m_swapChain = Gfx::SwapChain(m_context, m_device, m_surface,
        m_renderPass, m_format, presentIndex, graphicsIndex);

    m_shaderCache = ShaderCache(m_context, m_device);

    createLayouts();

    m_pipeline = Gfx::Pipeline(m_context, m_device, m_renderPass,
        { m_shaderCache.getShader(ShaderCache::ShaderPurpose::VoxelVert),
        m_shaderCache.getShader(ShaderCache::ShaderPurpose::VoxelFrag) }, m_canvas, m_format,
        Gfx::VertexDefinitions<VertexDefinitionPositionId, VertexDefinitionIndices>(),
        { &m_storageLayout, &m_perChunkLayout, &m_configLayout },
        { Gfx::PushConstantRange{ Gfx::ShaderStage::Bits::Vertex, 0, sizeof(PushConstants)} },
        Gfx::Pipeline::CullMode::None);

    //m_computePipeline = Gfx::ComputePipeline(m_context, m_device,
    //    m_shaderCache.getShader(ShaderCache::ShaderPurpose::VoxelCompute),
    //    { &m_storageLayout, &m_perChunkLayout, &m_configLayout },
    //    { Gfx::PushConstantRange{ Gfx::ShaderStage::Bits::Compute, 0, sizeof(PushConstants)} });

    m_graphicsCommandPool = Gfx::CommandPool(m_context, m_device, graphicsIndex);

    m_temporaryBufferPool = Gfx::CommandPool(m_context, m_device, graphicsIndex);

    m_descriptorPool = Gfx::DescriptorPool(m_context, m_device, {
            Gfx::DescriptorPool::Size(framesInFlight + 2, Gfx::DescriptorType::UniformBuffer),
            Gfx::DescriptorPool::Size(10 + 3, Gfx::DescriptorType::StorageBuffer),
            Gfx::DescriptorPool::Size(1024, Gfx::DescriptorType::CombinedImageSampler)
        }, framesInFlight + 3,
        Gfx::DescriptorPoolCreateFlags::Bits::UpdateAfterBind);

    std::vector<Gfx::DescriptorSetHandle> sets;
    std::vector<Gfx::DescriptorSetHandle> setsVariableSized;
    {
        std::vector<const Gfx::DescriptorSetLayout*> layouts;
        std::vector<const Gfx::DescriptorSetLayout*> layoutsVariableSized;
        std::vector<uint32_t> descriptorCounts;

        //for (size_t i = 0; i < framesInFlight; ++i)
        //    layouts.push_back(&m_perFrameLayout);
        layouts.push_back(&m_perChunkLayout);
        layouts.push_back(&m_configLayout);
        layoutsVariableSized.push_back(&m_storageLayout);
        descriptorCounts.push_back(1024);

        sets = m_descriptorPool.allocateSets(m_context, m_device, layouts);        
        setsVariableSized = m_descriptorPool.allocateSets(m_context, m_device,
            layoutsVariableSized, descriptorCounts);
    }

    //for (int i = 0; i < framesInFlight; ++i)
    //{
    //    m_perFrameInFlightObjects[i].uniformBuffer = Gfx::Buffer(m_context, m_device,
    //        Gfx::Buffer::Descriptor(sizeof(Gfx::UniformTransforms),
    //            Gfx::BufferUsage::Bits::Uniform, false));
    //}

    //m_uniformMemory = Gfx::MappedMemory(m_context, m_device,
    //    m_perFrameInFlightObjects[0].uniformBuffer.getMemoryRequirements(),
    //    Gfx::MemoryProperty::Bits::HostVisibleCoherent,
    //    sizeof(Gfx::UniformTransforms) * framesInFlight);

    m_perChunkSet = sets[0];
    m_configSet = sets[1];

    for (int i = 0; i < framesInFlight; ++i)
    {
        m_perFrameInFlightObjects[i].graphicsCommandBuffer = m_graphicsCommandPool.allocateBuffer(m_context, m_device);
        m_perFrameInFlightObjects[i].imageAvailableSemaphore = Gfx::Semaphore(m_context, m_device);
        m_perFrameInFlightObjects[i].renderFinishedSemaphore = Gfx::Semaphore(m_context, m_device);
        m_perFrameInFlightObjects[i].inFlightFence = Gfx::Fence(m_context, m_device, true);

        //m_perFrameInFlightObjects[i].perFrameSet = sets[i];
        //m_uniformMemory.bindBuffer(m_context, m_device,
        //    m_perFrameInFlightObjects[i].uniformBuffer, sizeof(Gfx::UniformTransforms) * i);
        //m_perFrameInFlightObjects[i].perFrameSet->write(m_context, m_device,
        //    m_perFrameInFlightObjects[i].uniformBuffer, 0,
        //    0, sizeof(Gfx::UniformTransforms));
    }

    m_sampler = Gfx::Sampler(m_context, m_device, Gfx::Sampler::Descriptor(m_context, m_device));
    m_storageSet = setsVariableSized.front();

    m_stagingBuffer = Gfx::Buffer(m_context, m_device, stagingMemorySize,
        Gfx::BufferUsage::Bits::TransferSrc, false);

    m_stagingMemory = Gfx::MappedMemory(m_context, m_device, m_stagingBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent, stagingMemorySize);
    m_stagingMemory.bindBuffer(m_context, m_device, m_stagingBuffer);

    m_contrastBuffer = Gfx::Buffer(m_context, m_device, sizeof(float) * 16,
        Gfx::BufferUsage::Bits::Uniform, false);
    m_contrastMemory = Gfx::MappedMemory(m_context, m_device, m_contrastBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent, sizeof(float) * 16);
    m_contrastMemory.bindBuffer(m_context, m_device, m_contrastBuffer);

    {
        auto mapping = m_contrastMemory.getMapping<float>(16);
        mapping[0] = 1.0f;
    }

    m_indexBuffer = Gfx::Buffer(m_context, m_device,
        3 * sizeof(uint32_t),
        Gfx::BufferUsage::Bits::Vertex | Gfx::BufferUsage::Bits::TransferDst, false);
    m_indexMemory = Gfx::Memory(m_context, m_device,
        m_indexBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal,
        m_indexBuffer.getMemoryRequirements().size);

    m_indexMemory.bindBuffer(m_context, m_device, m_indexBuffer);
    
    m_temporaryBufferPool.makeOneTimeDataTransfer<uint32_t>(
        m_context, m_device, m_transferQueue,
        m_stagingMemory, m_stagingBuffer,
        m_indexBuffer, indices);

    createConfigBuffers();

    m_imguiDescriptorPool = Gfx::DescriptorPool(m_context, m_device, {
    Gfx::DescriptorPool::Size(100, Gfx::DescriptorType::CombinedImageSampler),
    Gfx::DescriptorPool::Size(100, Gfx::DescriptorType::UniformBuffer),
    Gfx::DescriptorPool::Size(100, Gfx::DescriptorType::StorageBuffer)
        }, 100, Gfx::DescriptorPoolCreateFlags::Bits::FreeDescriptorSet);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window.getWindowHandle(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_context.getInstance();
    init_info.PhysicalDevice = m_physicalDevice->getHandle();
    init_info.Device = m_device.getDevice();
    init_info.QueueFamily = m_graphicsQueue.getFamily();
    init_info.Queue = m_graphicsQueue.getQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_imguiDescriptorPool.getPool();
    init_info.RenderPass = m_renderPass.getRenderPass();
    init_info.Subpass = 0;
    init_info.MinImageCount = m_swapChain.getImageCount();
    init_info.ImageCount = m_swapChain.getImageCount();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);
}

void Renderer::createConfigBuffers()
{
    m_configBuffer = Gfx::Buffer(m_context, m_device,
        256 + m_chunkCount * Constants::chunkSize,
        Gfx::BufferUsage::Bits::Uniform |
        Gfx::BufferUsage::Bits::Storage, false);

    m_configMemory = Gfx::MappedMemory(m_context, m_device,
        m_configBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent,
        m_configBuffer.getMemoryRequirements().size);

    m_configMemory.bindBuffer(m_context, m_device, m_configBuffer);

    m_configSet->write(m_context, m_device, m_configBuffer, 0, 0, sizeof(Config));

    m_configMapping = &m_configMemory.getMapping<Config>(1)[0];

    m_configMapping->contrast = 0.8f;
}

void Renderer::resetChunkBuffers(size_t chunkAmount)
{
    m_gridBuffer.destroy(m_context, m_device);
    m_chunkBuffer.destroy(m_context, m_device);
    m_drawCommandsBuffer.destroy(m_context, m_device);

    m_gridMemory.destroy(m_context, m_device);
    m_chunkMemory.destroy(m_context, m_device);
    m_drawCommandsMemory.destroy(m_context, m_device);

    size_t blockCount = chunkAmount * Constants::chunkSize;
    m_gridBuffer = Gfx::Buffer(m_context, m_device,
        blockCount * sizeof(Id::VoxelState),
        Gfx::BufferUsage::Bits::Storage, false);
    m_gridMemory = Gfx::MappedMemory(m_context, m_device,
        m_gridBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent,
        m_gridBuffer.getMemoryRequirements().size);

    m_chunkBuffer = Gfx::Buffer(m_context, m_device,
        chunkAmount * sizeof(WorldGrid::Chunk),
        Gfx::BufferUsage::Bits::Storage, false);
    m_chunkMemory = Gfx::MappedMemory(m_context, m_device,
        m_chunkBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent,
        m_chunkBuffer.getMemoryRequirements().size);

    m_drawCommandsBuffer = Gfx::Buffer(m_context, m_device,
        sizeof(Gfx::DrawCommand),
        Gfx::BufferUsage::Bits::Storage
        | Gfx::BufferUsage::Bits::Indirect
        | Gfx::BufferUsage::Bits::TransferDst, false);
    m_drawCommandsMemory = Gfx::MappedMemory(m_context, m_device,
        m_drawCommandsBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent,
        m_drawCommandsBuffer.getMemoryRequirements().size);

    m_indicesPool = Gfx::MemoryPool<Gfx::MappedMemory>(m_context, m_device,
        sizeof(Indices) * 120 * Constants::chunkSize * 16,
        Gfx::BufferUsage::Bits::Vertex, Gfx::MemoryProperty::Bits::HostVisibleCoherent, false);

    m_gridMemory.bindBuffer(m_context, m_device, m_gridBuffer);
    m_chunkMemory.bindBuffer(m_context, m_device, m_chunkBuffer);
    m_drawCommandsMemory.bindBuffer(m_context, m_device, m_drawCommandsBuffer);

    m_chunkCount = chunkAmount;

    m_perChunkSet->write(m_context, m_device, m_gridBuffer,
        0, 0, m_gridBuffer.getMemoryRequirements().size);
    m_perChunkSet->write(m_context, m_device, m_chunkBuffer,
        1, 0, m_chunkBuffer.getMemoryRequirements().size);

	m_drawCommands.resize(chunkAmount, std::pair<Gfx::DrawCommand, size_t>(Gfx::DrawCommand(0, 0, 0, 0), 0));
	m_indexAllocations.resize(chunkAmount, Gfx::MemoryPool<Gfx::MappedMemory>::Allocation::getEmptyAllocation());
	m_stagingBuffers.resize(m_poolHandle->getWorkerCount());
    for(size_t i = 0; i < m_stagingBuffers.size(); ++i)
        m_stagingBuffers[i].reserve(Constants::chunkSize * 120);
}

void Renderer::createAndWriteAssets(AssetCache& assetCache, 
    const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStateCache)
{
    assetCache.moveAssetsToGpuStorage(m_context, m_device,
        m_transferQueue, m_temporaryBufferPool, m_stagingMemory,
        m_stagingBuffer, voxelStateCache);
    assetCache.writeToDescriptors(m_context, m_device, m_storageSet, m_sampler,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}

void Renderer::createLayouts()
{
    m_perChunkLayout = Gfx::DescriptorSetLayout(m_context, m_device,
        Gfx::DescriptorDefinitions<
        GridDataLayoutDefinition<>,
        ChunkDataLayoutDefinition<>/*,
        IndicesBufferLayoutDefinition<>,
        CommandBufferLayoutDefinition<>*/
        >());

    m_storageLayout = Gfx::DescriptorSetLayout(m_context, m_device,
        Gfx::DescriptorDefinitions<
        StorageCache::VertexLayoutDefinition<>,
        StorageCache::UvLayoutDefinition<>,
        StorageCache::PolygonLayoutDefinition<>,
        StorageCache::ColoringLayoutDefinition<>,
        StorageCache::PolygonIndexLayoutDefinition<>,
        StorageCache::ColoringIndexLayoutDefinition<>,
        StorageCache::AppearenceLayoutDefinition<>,
        StorageCache::GeometryLayoutDefinition<>,
        StorageCache::ModelLayoutDefinition<>,
        StorageCache::StateToModelLayoutDefinition<>,
        StorageCache::ImageLayoutDefinition<>>(),
        Gfx::DescriptorSetLayoutCreate::Bits::UpdateAfterBind);

    m_configLayout = Gfx::DescriptorSetLayout(m_context, m_device,
        Gfx::DescriptorDefinitions<
        ConfigLayoutDefinition<>>());
}

void Renderer::cleanup()
{
    m_device.waitIdle(m_context);
    m_temporaryBufferPool.destroy(m_context, m_device);    
    for (int i = 0; i < framesInFlight; ++i)
    {
        m_graphicsCommandPool.freeBuffer(m_context, m_device,
            m_perFrameInFlightObjects[i].graphicsCommandBuffer);
        m_perFrameInFlightObjects[i].imageAvailableSemaphore.destroy(m_context, m_device);
        m_perFrameInFlightObjects[i].renderFinishedSemaphore.destroy(m_context, m_device);
        m_perFrameInFlightObjects[i].inFlightFence.destroy(m_context, m_device);
    }
    m_graphicsCommandPool.destroy(m_context, m_device);

    m_sampler.destroy(m_context, m_device);

    m_uniformMemory.destroy(m_context, m_device);
    m_contrastBuffer.destroy(m_context, m_device);
    m_stagingBuffer.destroy(m_context, m_device);

    m_contrastMemory.destroy(m_context, m_device);
    m_stagingMemory.destroy(m_context, m_device);
    m_descriptorPool.destroy(m_context, m_device);

    m_pipeline.destroy(m_context, m_device);

    m_configLayout.destroy(m_context, m_device);
    m_perChunkLayout.destroy(m_context, m_device);
    m_storageLayout.destroy(m_context, m_device);

    m_shaderCache.destroy(m_context, m_device);

    m_swapChain.destroy(m_context, m_device);
    m_renderPass.destroy(m_context, m_device);

    m_device.destroy(m_context);

    m_context.destroy();
}

//Renderer::GridMapping Renderer::getGridMapping()
//{
//    return m_gridMemory.getMapping<Id::VoxelState>(Constants::chunkSize * m_chunkCount);
//}
//
//Renderer::ChunkMapping Renderer::getChunkMapping()
//{
//    return m_chunkMemory.getMapping<WorldGrid::Chunk>(m_chunkCount);
//}

void Renderer::handleResize(const Gfx::Extent2D& extent)
{
    m_device.waitIdle(m_context);
    m_canvas = Gfx::RenderRegion::createFullWindow(extent);
    m_format = Gfx::SwapChainFormat::create(m_context, m_device, m_surface, extent);
    m_swapChain.recreate(m_context, m_device, m_surface, m_renderPass, m_format,
        m_presentQueue.getFamily(), m_graphicsQueue.getFamily());
}

void Renderer::drawFrame(const Graphics::CameraPerspective& camera)
{
    PushConstants constants;
    constants.chunkCount = m_chunkCount;
    constants.proj = camera.getProjection();
    constants.view = camera.getView();

    m_perFrameInFlightObjects[m_currentFrame].inFlightFence.wait(m_context, m_device);
    std::unique_lock<std::shared_mutex> lock(m_drawLock);
    m_perFrameInFlightObjects[m_currentFrame].inFlightFence.reset(m_context, m_device);
        
    uint32_t imageIndex;
    try {
        if (!m_swapChain.acquireNextImage(m_context, m_device,
            m_perFrameInFlightObjects[m_currentFrame].imageAvailableSemaphore, imageIndex))
        {
            return; // Skip frame on swapchain issues
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to acquire swapchain image: " << e.what() << std::endl;
        return;
    }

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->reset(m_context);
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->record(m_context);

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->pushConstants(m_context, m_pipeline,
        Gfx::ShaderStage::Bits::Vertex, 0, sizeof(PushConstants), &constants);

    //if (m_frameCounter == 0)
    //{
    //    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->pushConstants(m_context, m_computePipeline,
    //        Gfx::ShaderStage::Bits::Compute, 0, sizeof(PushConstants), &constants);

    //    // Run compute shader to generate draw commands
    //    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindPipeline(m_context, m_computePipeline);
    //    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindDescriptorSets(
    //        m_context, m_computePipeline, { m_storageSet, m_perChunkSet, m_configSet }, {});

    //    //size_t chunksX = sqrt(m_chunkCount);
    //    //size_t chunksY = (m_chunkCount + chunksX - 1) / chunksX;

    //    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->dispatch(
    //        m_context,
    //        Constants::chunkWidth * m_chunkCount / 16,
    //        Constants::chunkHeight / 8,
    //        Constants::chunkDepth / 8);

    //    // Pipeline barrier: compute -> indirect draw
    //    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->setPipelineBarrier(m_context,
    //        Gfx::PipelineStage::Bits::ComputeShader,
    //        Gfx::PipelineStage::Bits::DrawIndirect
    //        | Gfx::PipelineStage::Bits::VertexInput,
    //        m_drawCommandsBuffer, vk::AccessFlagBits::eShaderWrite,
    //        vk::AccessFlagBits::eIndirectCommandRead);
    //}

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->beginRenderPass(m_context, m_renderPass,
        m_swapChain, imageIndex, Gfx::Color::Green(), 1);

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindPipeline(m_context, m_pipeline);
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->setRenderView(m_context, m_canvas);
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindDescriptorSets(
        m_context, m_pipeline, { m_storageSet, m_perChunkSet, m_configSet }, {});

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindVertexBuffers(m_context,
        std::array<const Gfx::Buffer*, 1>{ &m_indexBuffer },
        std::array<vk::DeviceSize, 1>{ vk::DeviceSize(0) }, 0);

	auto& buffers = m_indicesPool.getBuffers();

    for (size_t i = 0; i < m_chunkCount; ++i)
    {
        if(m_drawCommands[i].first.vertexCount == 0)
			continue;

        m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindVertexBuffers(m_context,
            std::array<const Gfx::Buffer*, 1>{ &buffers[m_drawCommands[i].second] },
            std::array<vk::DeviceSize, 1>{ vk::DeviceSize(0) }, 1);

        m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->draw(m_context,
            m_drawCommands[i].first.vertexCount, m_drawCommands[i].first.instanceCount,
            m_drawCommands[i].first.firstVertex, m_drawCommands[i].first.firstInstance);
    }

    drawGui(camera);

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->endRenderPass(m_context);
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->stopRecord(m_context);

    try
    {
        m_graphicsQueue.submit(
            m_context,
            std::array{ vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput) },
            std::array{ std::ref(m_perFrameInFlightObjects[m_currentFrame].imageAvailableSemaphore) },
            std::array{ std::ref(m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer) },
            std::array{ std::ref(m_perFrameInFlightObjects[m_currentFrame].renderFinishedSemaphore) },
            m_perFrameInFlightObjects[m_currentFrame].inFlightFence
        );
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to submit graphics queue: " << e.what() << std::endl;
    }

    try
    {
        if (!m_presentQueue.present(m_context,
            std::array{ std::ref(m_perFrameInFlightObjects[m_currentFrame].renderFinishedSemaphore) },
            std::array{ std::ref(m_swapChain) },
            std::array{ imageIndex }))
        {
            /*handleResize();*/
            return;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to present swap chain image: " << e.what() << std::endl;
    }

    m_currentFrame = (m_currentFrame + 1) % framesInFlight;
    m_frameCounter = (m_frameCounter + 1) % 1000000;

    m_perFrameInFlightObjects[m_currentFrame].inFlightFence.wait(m_context, m_device);
}

void Renderer::drawGui(const Graphics::CameraPerspective& camera)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(550, 500), ImGuiCond_Always);

    if (ImGui::Begin("Debug Info", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Text("FPS:               %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Position:          %.2f, %.2f, %.2f",
            camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
        ImGui::Text("Yaw:               %.2f", camera.getYaw());
        ImGui::Text("Pitch:             %.2f", camera.getPitch());
        ImGui::Text("Block count:       %.2i", m_chunkCount * Constants::chunkSize);
    }
    ImGui::Text("Memory Pool Usage:");
    size_t bufferCount = m_indicesPool.getBuffers().size();
    ImGui::Text("Total buffer count: %d", bufferCount);

    for (size_t i = 0; i < bufferCount; ++i)
        drawMemoryPoolVisualization(i);

    ImGui::End();

    //ImGui::Text("Draw command:");
    //ImGui::Text("Vertex count:      %i", drawCommand.vertexCount);
    //ImGui::Text("Instance count:    %i", drawCommand.instanceCount);
    //ImGui::Text("First vertex:      %i", drawCommand.firstVertex);
    //ImGui::Text("First instance:    %i", drawCommand.firstInstance);    

    //ImGui::Text("Config:");
    //ImGui::Text("Buffer size:       %i", m_configMapping->bufferSize);
    //ImGui::Text("Buffer threshold:  %i", m_configMapping->bufferThreshold);
    //ImGui::Text("Contrast:          %.2f", m_configMapping->contrast);
    //
    //ImGui::Text("Usage:");
    //ImGui::Text("Buffer usage:      %i", m_usageMapping->bufferUsage);
    //ImGui::Text("Needs expansion:   %i", m_usageMapping->needsExpansion);
    //ImGui::Text("Overflowed:        %i", m_usageMapping->overflowed);

    m_debugConsole.draw();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
        m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->getCommandBuffer());
}

//void Renderer::dumpErrors()
//{
//    if (*m_errorCount <= 0)
//        return;
//
//    std::cout << "Shader errors:" << std::endl;
//
//    for(size_t i = 0; i < *m_errorCount; ++i)
//    {
//        Error& error = m_errorMapping[i];
//        std::cout << "Error " << i << ":" << std::endl;
//        std::cout << "Message:              " << shaderErrorMessages[static_cast<uint32_t>(error.errorCode)] << std::endl;
//        std::cout << "Global invocation id: " << error.globalInvocationId << std::endl;
//        std::cout << "Local invocation id:  " << error.localInvocationId << std::endl;
//        std::cout << "Work group id:        " << error.workGroupId << std::endl;
//    }
//}

void Renderer::updateChunk(const ResourceCache& resources, size_t chunkIndex,
    const WorldGrid::Chunk& chunk, const WorldGrid::Grid& grid, size_t threadId)
{
    auto startStaging = std::chrono::high_resolution_clock::now();

    auto& assets = resources.getAssetCache();
    auto& states = resources.getVoxelStateCache();
    auto& models = assets.getModelCache();
    auto& geometries = assets.getGeometryCache();
    auto& appearances = assets.getAppearanceCache();

    auto& buffer = m_stagingBuffers[threadId];

    for(size_t x = 0; x < Constants::chunkWidth; ++x)
    {
        for(size_t z = 0; z < Constants::chunkDepth; ++z)
        {
            for(size_t y = 0; y < Constants::chunkHeight; ++y)
            {
                auto block = chunk.start + x + z * Constants::chunkWidth + y * Constants::chunkLayerSize;
                const auto& state = grid[block];
                if (state == Constants::emptyStateId)
                    continue;
                const auto& model = models[states[state].m_model];
                const auto& geometryEntry = geometries.entryCache()[model.geometry];
                const auto& appearanceEntry = appearances.entryCache()[model.appearence];

                for (size_t i = 0; i < 1/*geometryEntry.size*/; ++i)
                {
                    Indices indices;
                    indices.polygon = geometries[geometryEntry.start + i];
                    indices.coloring = appearances[appearanceEntry.start + i];
                    indices.block = block;
                    buffer.push_back(indices);
                }
            }
        }
    }

    auto endStaging = std::chrono::high_resolution_clock::now();
    auto stagingDuration = std::chrono::duration_cast<std::chrono::microseconds>(endStaging - startStaging).count();

    auto startAllocation = std::chrono::high_resolution_clock::now();
    auto& allocation = m_indexAllocations[chunkIndex];

    if (allocation.region.size > buffer.size() * sizeof(Indices))
    {
        std::unique_lock<std::mutex> lock(m_poolLock);
        m_indicesPool.shrink(allocation, buffer.size() * sizeof(Indices));
    }
    else if (allocation.region.size < buffer.size() * sizeof(Indices))
    {
        std::unique_lock<std::mutex> lock(m_poolLock);
        m_indicesPool.free(allocation);
        allocation = m_indicesPool.allocate(
            m_context, m_device, buffer.size() * sizeof(Indices));
    }

    auto endAllocation = std::chrono::high_resolution_clock::now();
    auto allocationDuration = std::chrono::duration_cast<std::chrono::microseconds>(endAllocation - startAllocation).count();

    auto startMemoryPopulate = std::chrono::high_resolution_clock::now();
    {
        std::shared_lock<std::shared_mutex> lock(m_drawLock);
        m_perFrameInFlightObjects[m_currentFrame].inFlightFence.wait(m_context, m_device);

        auto& memory = m_indicesPool.getMemories()[allocation.bufferIndex];
        auto mapping = memory.getMapping<Indices>(buffer.size(), allocation.region.offset);
        std::copy(buffer.begin(), buffer.end(), mapping.begin());        

        auto& command = m_drawCommands[chunkIndex];
        command.first.vertexCount = 3;
        command.first.instanceCount = buffer.size();
        command.first.firstVertex = 0;
        command.first.firstInstance = allocation.region.offset / sizeof(Indices);
        command.second = allocation.bufferIndex;
        buffer.clear();

        auto chunkMapping = m_chunkMemory.getMapping<WorldGrid::Chunk>(m_chunkCount);
        chunkMapping[chunkIndex] = chunk;
    }

    auto endMemoryPopulate = std::chrono::high_resolution_clock::now();
    auto memoryPopulateDuration = std::chrono::duration_cast<std::chrono::microseconds>(endMemoryPopulate - startMemoryPopulate).count();


    m_debugConsole.log("Chunk {} updated with {} indices, "
        "Allocation: size {}, offset {}, buffer {}\n"
        "Timings (μs): Staging: {}, Allocation: {}, MemoryPopulate: {}\n",
        chunkIndex, buffer.size(),
        allocation.region.size, allocation.region.offset, allocation.bufferIndex,
        stagingDuration, allocationDuration, memoryPopulateDuration);
}

void Renderer::updateChunkAsync(const ResourceCache& resources, size_t chunkIndex,
    const WorldGrid::Chunk& chunk, const WorldGrid::Grid& grid)
{
    m_poolHandle->pushTask([this, &resources, chunkIndex, &chunk, &grid](size_t threadId) {
        updateChunk(resources, chunkIndex, chunk, grid, threadId);
        });
}

void Renderer::drawMemoryPoolVisualization(size_t chunkIndex) {
    std::unique_lock<std::mutex> lock(m_poolLock);
    float totalSize = m_indicesPool.getChunkSize();
    size_t allocatedSize = m_indicesPool.getChunkAllocatedSize(chunkIndex);
    size_t freeSize = m_indicesPool.getChunkFreeSize(chunkIndex);
    const auto& freeRegions = m_indicesPool.getFreeRegions(chunkIndex);
    const auto& allocations = m_indicesPool.getAllocations(chunkIndex);

    ImGui::Text("Buffer size: %d Buffer usage: %d Buffer free: %d", (size_t)totalSize, allocatedSize, freeSize);
    ImGui::Text("Allocation count: %d free block count: %d", allocations.size(), freeRegions.size());

    ImVec2 barSize(ImGui::GetContentRegionAvail().x, 20.0f);
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();


    // Draw free memory (gray)
    for (const auto& block : freeRegions) {
        float width = (block.size / totalSize) * barSize.x;
        float offset = (block.offset / totalSize) * barSize.x;
        drawList->AddRectFilled(
            ImVec2(cursorPos.x + offset, cursorPos.y),
            ImVec2(cursorPos.x + offset + width, cursorPos.y + barSize.y),
            IM_COL32(100, 100, 100, 255) // Gray for free memory
        );
    }

    // Draw used memory (red)
    for (const auto& block : allocations) {
        float width = (block.region.size / totalSize) * barSize.x;
        float offset = (block.region.offset / totalSize) * barSize.x;
        drawList->AddRectFilled(
            ImVec2(cursorPos.x + offset, cursorPos.y),
            ImVec2(cursorPos.x + offset + width, cursorPos.y + barSize.y),
            IM_COL32(255, 0, 0, 255) // Red for used memory
        );
        offset += width;
    }

    // Optional: Draw border
    drawList->AddRect(
        cursorPos,
        ImVec2(cursorPos.x + barSize.x, cursorPos.y + barSize.y),
        IM_COL32(255, 255, 255, 255)
    );

    ImGui::Dummy(barSize); // Advance cursor
}