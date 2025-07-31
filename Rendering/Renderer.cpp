#include "Renderer.h"

void Renderer::init(std::string engineName, std::string appName, Window& window)
{
    m_context = Gfx::Context(engineName, appName,
        Gfx::Version(1, 0, 0), Gfx::Version(1, 0, 0), Gfx::DebugMessageSeverity::Bits::Warning
        | Gfx::DebugMessageSeverity::Bits::Error | Gfx::DebugMessageSeverity::Bits::All);

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
        Gfx::VertexDefinitions<>(),
        { &m_perFrameLayout, &m_storageLayout, &m_perChunkLayout, &m_configLayout },
        Gfx::Pipeline::CullMode::None);

    m_computePipeline = Gfx::ComputePipeline(m_context, m_device,
        m_shaderCache.getShader(ShaderCache::ShaderPurpose::VoxelCompute),
        { &m_perFrameLayout, &m_storageLayout, &m_perChunkLayout, &m_configLayout });

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

        for (size_t i = 0; i < framesInFlight; ++i)
            layouts.push_back(&m_perFrameLayout);
        layouts.push_back(&m_perChunkLayout);
        layouts.push_back(&m_configLayout);
        layoutsVariableSized.push_back(&m_storageLayout);
        descriptorCounts.push_back(1024);

        sets = m_descriptorPool.allocateSets(m_context, m_device, layouts);        
        setsVariableSized = m_descriptorPool.allocateSets(m_context, m_device,
            layoutsVariableSized, descriptorCounts);
    }

    for (int i = 0; i < framesInFlight; ++i)
    {
        m_perFrameInFlightObjects[i].uniformBuffer = Gfx::Buffer(m_context, m_device,
            Gfx::Buffer::Descriptor(sizeof(Gfx::UniformTransforms),
                Gfx::BufferUsage::Bits::Uniform, false));
    }

    m_uniformMemory = Gfx::MappedMemory(m_context, m_device,
        m_perFrameInFlightObjects[0].uniformBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent,
        sizeof(Gfx::UniformTransforms) * framesInFlight);

    m_perChunkSet = sets[framesInFlight];
    m_configSet = sets[framesInFlight + 1];

    for (int i = 0; i < framesInFlight; ++i)
    {
        m_perFrameInFlightObjects[i].graphicsCommandBuffer = m_graphicsCommandPool.allocateBuffer(m_context, m_device);
        m_perFrameInFlightObjects[i].imageAvailableSemaphore = Gfx::Semaphore(m_context, m_device);
        m_perFrameInFlightObjects[i].renderFinishedSemaphore = Gfx::Semaphore(m_context, m_device);
        m_perFrameInFlightObjects[i].inFlightFence = Gfx::Fence(m_context, m_device, true);

        m_perFrameInFlightObjects[i].perFrameSet = sets[i];
        m_uniformMemory.bindBuffer(m_context, m_device,
            m_perFrameInFlightObjects[i].uniformBuffer, sizeof(Gfx::UniformTransforms) * i);
        m_perFrameInFlightObjects[i].perFrameSet->write(m_context, m_device,
            m_perFrameInFlightObjects[i].uniformBuffer, 0,
            0, sizeof(Gfx::UniformTransforms));
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

    m_configSet->write(m_context, m_device, m_contrastBuffer, 0, 0, sizeof(float));
    setupPerChunkBuffers();


    m_indexBuffer = Gfx::Buffer(m_context, m_device,
        3 * sizeof(uint32_t),
        Gfx::BufferUsage::Bits::Index | Gfx::BufferUsage::Bits::TransferDst, false);
    m_indexMemory = Gfx::Memory(m_context, m_device,
        m_indexBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal,
        3 * sizeof(uint32_t));

    m_indexMemory.bindBuffer(m_context, m_device, m_indexBuffer);
    
    m_temporaryBufferPool.makeOneTimeDataTransfer<uint32_t>(
        m_context, m_device, m_transferQueue,
        m_stagingMemory, m_stagingBuffer,
        m_indexBuffer, indices);

    m_imguiDescriptorPool = Gfx::DescriptorPool(m_context, m_device, {
    Gfx::DescriptorPool::Size(100, Gfx::DescriptorType::CombinedImageSampler),
    Gfx::DescriptorPool::Size(100, Gfx::DescriptorType::UniformBuffer),
    Gfx::DescriptorPool::Size(100, Gfx::DescriptorType::StorageBuffer)
        }, 100, Gfx::DescriptorPoolCreateFlags::Bits::FreeDescriptorSet);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
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

void Renderer::setupPerChunkBuffers()
{
    m_chunkDataBuffer = Gfx::Buffer(m_context, m_device,
        Constants::chunkSize * sizeof(Id::VoxelState),
        Gfx::BufferUsage::Bits::Storage, false);
    m_chunkDataMemory = Gfx::MappedMemory(m_context, m_device,
        m_chunkDataBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent,
        Constants::chunkSize * sizeof(Id::VoxelState));

    m_rangeStartsBuffer = Gfx::Buffer(m_context, m_device,
        Constants::chunkSize * sizeof(RangeStarts),
        Gfx::BufferUsage::Bits::Storage, false);
    m_rangeStartsMemory = Gfx::Memory(m_context, m_device,
        m_rangeStartsBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal,
        Constants::chunkSize * sizeof(RangeStarts));

    m_drawCommandsBuffer = Gfx::Buffer(m_context, m_device,
        Constants::chunkSize * sizeof(Gfx::DrawCommand),
        Gfx::BufferUsage::Bits::Storage 
        | Gfx::BufferUsage::Bits::Indirect 
        | Gfx::BufferUsage::Bits::TransferDst, false);
    m_drawCommandsMemory = Gfx::Memory(m_context, m_device,
        m_drawCommandsBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal,
        Constants::chunkSize * sizeof(Gfx::DrawCommand));
    
    m_chunkPositionBuffer = Gfx::Buffer(m_context, m_device,
        sizeof(glm::vec4),
        Gfx::BufferUsage::Bits::Uniform, false);
    m_chunkPositionMemory = Gfx::MappedMemory(m_context, m_device,
        m_chunkPositionBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::HostVisibleCoherent,
        m_chunkPositionBuffer.getMemoryRequirements().size);

    m_chunkDataMemory.bindBuffer(m_context, m_device, m_chunkDataBuffer);
    m_rangeStartsMemory.bindBuffer(m_context, m_device, m_rangeStartsBuffer);
    m_drawCommandsMemory.bindBuffer(m_context, m_device, m_drawCommandsBuffer);
    m_chunkPositionMemory.bindBuffer(m_context, m_device, m_chunkPositionBuffer);

    //// Initialize buffer with zeros to prevent garbage draw commands
    //std::vector<Gfx::DrawCommand> zeroCommands(Constants::chunkSize, { 0, 0, 0, 0 });
    //m_temporaryBufferPool.makeOneTimeDataTransfer<Gfx::DrawCommand>(
    //    m_context, m_device, m_transferQueue,
    //    m_stagingMemory, m_stagingBuffer,
    //    m_drawCommandsBuffer, zeroCommands);

    m_perChunkSet->write(m_context, m_device, m_chunkDataBuffer,
        0, 0, m_chunkDataBuffer.getMemoryRequirements().size);
    m_perChunkSet->write(m_context, m_device, m_rangeStartsBuffer,
        1, 0, m_rangeStartsBuffer.getMemoryRequirements().size);
    m_perChunkSet->write(m_context, m_device, m_drawCommandsBuffer,
        2, 0, m_drawCommandsBuffer.getMemoryRequirements().size);
    m_perChunkSet->write(m_context, m_device, m_chunkPositionBuffer,
        3, 0, sizeof(glm::vec4));
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
    m_perFrameLayout = Gfx::DescriptorSetLayout(m_context, m_device,
        Gfx::DescriptorDefinitions<Gfx::UniformTransformsDefinition>());

    m_perChunkLayout = Gfx::DescriptorSetLayout(m_context, m_device,
        Gfx::DescriptorDefinitions<
        ChunkDataLayoutDefinition<>,
        RangeStartBufferLayoutDefinition<>,
        IndirectCommandBufferLayoutDefinition<>,
        ChunkPosLayoutDefinition<>
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
        Gfx::DescriptorDefinitions<ContrastLayoutDefinition<>>());
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
        m_perFrameInFlightObjects[i].uniformBuffer.destroy(m_context, m_device);
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
    m_perFrameLayout.destroy(m_context, m_device);
    m_storageLayout.destroy(m_context, m_device);

    m_shaderCache.destroy(m_context, m_device);

    m_swapChain.destroy(m_context, m_device);
    m_renderPass.destroy(m_context, m_device);

    m_device.destroy(m_context);

    m_context.destroy();
}

//void Renderer::writeChunkData(const VoxelVolume& volume, const glm::vec4& pos)
//{
//    m_temporaryBufferPool.makeOneTimeDataTransfer<uint32_t>(
//        m_context, m_device, m_transferQueue,
//        m_stagingMemory, m_stagingBuffer,
//        m_chunkDataBuffer, volume.data());
//
//    std::array<glm::vec4, 1> arr = { pos };
//
//    m_temporaryBufferPool.makeOneTimeDataTransfer<glm::vec4>(
//        m_context, m_device, m_transferQueue,
//        m_stagingMemory, m_stagingBuffer,
//        m_chunkPositionBuffer, arr);
//}

Renderer::ChunkDataMapping Renderer::getChunkDataMapping()
{
    return m_chunkDataMemory.getMapping<Id::VoxelState>(Constants::chunkSize);
}

Renderer::ChunkCoordMapping Renderer::getChunkCoordMapping()
{
    return m_chunkPositionMemory.getMapping<glm::vec4>(1);
}

void Renderer::handleResize(const Gfx::Extent2D& extent)
{
    m_device.waitIdle(m_context);
    m_canvas = Gfx::RenderRegion::createFullWindow(extent);
    m_format = Gfx::SwapChainFormat::create(m_context, m_device, m_surface, extent);
    m_swapChain.recreate(m_context, m_device, m_surface, m_renderPass, m_format,
        m_presentQueue.getFamily(), m_graphicsQueue.getFamily());
}

void Renderer::updatePerFrameUniform(const Gfx::CameraPerspective& camera)
{
    m_transforms.view = camera.getView();
    m_transforms.proj = camera.getProjection();
    auto mapping = m_uniformMemory.getMapping<Gfx::UniformTransforms>(framesInFlight);
    mapping[m_currentFrame] = m_transforms;
}

void Renderer::drawFrame(const Graphics::CameraPerspective& camera)
{
    m_perFrameInFlightObjects[m_currentFrame].inFlightFence.wait(m_context, m_device);
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

    // Run compute shader to generate draw commands
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindPipeline(m_context, m_computePipeline);
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindDescriptorSets(
        m_context, m_computePipeline, { m_perFrameInFlightObjects[m_currentFrame].perFrameSet,
        m_storageSet, m_perChunkSet, m_configSet }, {});
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->dispatch(m_context, Constants::chunkSize / 128, 1, 1); // 32 * 128 = 4096 threads

    // Pipeline barrier: compute -> indirect draw
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->setPipelineBarrier(m_context,
        Gfx::PipelineStage::Bits::ComputeShader,
        Gfx::PipelineStage::Bits::DrawIndirect 
        | Gfx::PipelineStage::Bits::VertexInput,
        m_drawCommandsBuffer);

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->beginRenderPass(m_context, m_renderPass,
        m_swapChain, imageIndex, Gfx::Color::Green(), 1);
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindPipeline(m_context, m_pipeline);
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->setRenderView(m_context, m_canvas);

    // No vertex buffers needed - data comes from storage buffers
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindIndexBuffer(m_context, m_indexBuffer, 0); // {0,1,2}
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->bindDescriptorSets(
        m_context, m_pipeline, { m_perFrameInFlightObjects[m_currentFrame].perFrameSet, m_storageSet, m_perChunkSet, m_configSet }, {});

    // Indirect draw call
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->drawIndirect(m_context,
        m_drawCommandsBuffer, 0, Constants::chunkSize, sizeof(Gfx::DrawCommand));

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
}

void Renderer::drawGui(const Graphics::CameraPerspective& camera)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Your GUI code
    ImGui::Begin("Debug Info");
    ImGui::Text("FPS:        %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Position:   %.2f, %.2f, %.2f",
        camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
    ImGui::Text("Yaw:        %.2f", camera.getYaw());
    ImGui::Text("Pitch:      %.2f", camera.getPitch());
    ImGui::End();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
        m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer->getCommandBuffer());
}