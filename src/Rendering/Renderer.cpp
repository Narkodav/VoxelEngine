#include "Rendering/Renderer.h"

#ifdef _WIN32
#include "imgui_impl_win32.h"
#include <windows.h>
#else
#include "imgui_impl_x11.h"
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Renderer::init(std::string engineName, std::string appName,
    Platform::Window& window, MT::ThreadPool& poolHandle, const EngineFilesystem& engineFiles)
{
    m_poolHandle = &poolHandle;

    m_instance.create(Gfx::AppInfo().setAppName(appName).setEngineName(engineName)
        .setAppVersion({1, 0, 0}).setEngineVersion({1, 0, 0}));

    m_instance.cachePhysicalDevices();    

    m_surface.create(m_instance.getFunctionTable(), m_instance, window);

    window.registerCallback<Platform::WindowEvents::WindowResized>([this](int width, int height) {
        handleResize(Gfx::Extent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
        });

    Gfx::DeviceRequirements requirements;
    requirements.extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME 
    };
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
        {Gfx::DeviceFeature::MeshShader, true},
        {Gfx::DeviceFeature::TaskShader, true},
        {Gfx::DeviceFeature::ShaderDrawParameters, true},
        {Gfx::DeviceFeature::MultiDrawIndirect, true},
        {Gfx::DeviceFeature::DescriptorBindingStorageBufferUpdateAfterBind, true},
        {Gfx::DeviceFeature::NullDescriptor, true}, 
        {Gfx::DeviceFeature::DescriptorIndexing, true}, 
    };

    requirements.queueProperties.push_back(Gfx::RequiredQueueProperties());
    requirements.queueProperties.back().queueProperties
        .insert({ Gfx::QueueProperty::QueueFlags, Gfx::Flags::Queue::Bits::Graphics | Gfx::Flags::Queue::Bits::Transfer });
    requirements.queueProperties.back().shouldSupportPresent = false;

    requirements.queueProperties.push_back(Gfx::RequiredQueueProperties());
    requirements.queueProperties.back().shouldSupportPresent = true;

    auto result = m_instance.getFittingDevice(m_surface, requirements);
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
    
    std::vector<std::vector<float>> queuePriorities = { {1.0f, 1.0f}, {1.0f} };
    std::vector<Gfx::QueueCreateInfo> queueCreateInfos(2);
    queueCreateInfos[0].setQueueFamilyIndex(graphicsIndex)
        .setQueuePriorities(queuePriorities[0]);
    queueCreateInfos[1].setQueueFamilyIndex(presentIndex)
        .setQueuePriorities(queuePriorities[1]);

    Gfx::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos)
        .setEnabledExtensions(requirements.extensions)
        .setEnabledFeatures(result.enabledFeatures);
        
    m_device.create(m_instance, m_physicalDevice, deviceCreateInfo);
    
    m_graphicsQueue = m_device.getQueue(graphicsIndex, 0);
    m_presentQueue = m_device.getQueue(presentIndex, 0);
    m_transferQueue = m_device.getQueue(graphicsIndex, 1);
    
    m_canvas = Gfx::RenderRegion::createFullWindow(window.getWindowExtent());
    
    auto capabilities = m_physicalDevice.getSurfaceCapabilities(m_instance.getFunctionTable(), m_surface);
    auto formats = m_physicalDevice.getSurfaceFormats(m_instance.getFunctionTable(), m_surface);
    auto presentModes = m_physicalDevice.getSurfacePresentModes(m_instance.getFunctionTable(), m_surface);

    Gfx::PixelFormat depthFormat = Gfx::Utility::findDepthFormat(m_instance.getFunctionTable(), m_physicalDevice);

    if (depthFormat == Gfx::PixelFormat::Undefined) {
        throw std::runtime_error("Failed to find supported depth format");
    }

    m_renderPassData = Gfx::Utility::createColorDepthRenderPass(m_device.getFunctionTable(),
        m_device, formats, presentModes, depthFormat);

    m_swapChainData = Gfx::Utility::createBasicSwapChain(m_instance.getFunctionTable(),
        m_device.getFunctionTable(), m_physicalDevice, m_device, m_surface, m_renderPassData.renderPass,
        Gfx::Utility::chooseExtent(capabilities, window.getFrameBufferExtent()),
        m_renderPassData.surfaceFormat.getFormat(), depthFormat, m_renderPassData.surfaceFormat.getColorSpace(),
        Gfx::Flags::ImageUsage::Bits::ColorAttachment, m_renderPassData.presentMode, capabilities.getMinImageCount());

    createLayouts();

    m_shaderCache = ShaderCache(m_device, engineFiles);
    
    Gfx::PushConstantRange pushConstantRange;
    pushConstantRange.setStageFlags(Gfx::Flags::ShaderStage::Bits::Vertex)
        .setOffset(0).setSize(sizeof(PushConstants));

    m_graphicsPipelineData.pipelineLayoutCreateInfo
        .setSetLayouts(m_descriptorSetLayouts)
        .setPushConstantRanges(pushConstantRange);
    m_graphicsPipelineData.pipelineLayout.create(m_device.getFunctionTable(), 
        m_device, m_graphicsPipelineData.pipelineLayoutCreateInfo);

    m_graphicsPipelineData.shaderStages = m_shaderCache.getShaderStageCreateInfos();
    
    m_graphicsPipelineData.dynamicStates = { Gfx::DynamicState::Viewport, Gfx::DynamicState::Scissor };
    m_graphicsPipelineData.viewports = { m_canvas.getViewport() };
    m_graphicsPipelineData.scissors = { m_canvas.getScissor() };
    m_graphicsPipelineData.colorBlendAttachmentStates.push_back(Gfx::PipelineColorBlendAttachmentState(false,
        Gfx::BlendFactor::One, Gfx::BlendFactor::Zero,
        Gfx::BlendOp::Add, Gfx::BlendFactor::One,
        Gfx::BlendFactor::Zero, Gfx::BlendOp::Add,
        Gfx::Flags::ColorComponent::Bits::R | Gfx::Flags::ColorComponent::Bits::G |
        Gfx::Flags::ColorComponent::Bits::B | Gfx::Flags::ColorComponent::Bits::A));

    m_graphicsPipelineData.vertexInputState = {
        m_graphicsPipelineData.vertexBindings, m_graphicsPipelineData.vertexAttributes
    };
    m_graphicsPipelineData.inputAssemblyState = {
        Gfx::PrimitiveTopology::TriangleList, false
    };
    m_graphicsPipelineData.dynamicState = {
        m_graphicsPipelineData.dynamicStates
    };
    m_graphicsPipelineData.viewportState = {
        m_graphicsPipelineData.viewports, m_graphicsPipelineData.scissors
    };
    m_graphicsPipelineData.rasterizationState = { 
        false, false, Gfx::PolygonMode::Fill, Gfx::Flags::CullMode::Bits::Back, 
        Gfx::FrontFace::CounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f 
    };
    m_graphicsPipelineData.multisampleState = { 
        false, Gfx::Flags::SampleCount::Bits::SC1, 1.0f, {}, false, false 
    };
    m_graphicsPipelineData.colorBlendState = { 
        m_graphicsPipelineData.colorBlendAttachmentStates,
        false, Gfx::LogicOp::Copy, Gfx::Color::empty()
    };
    m_graphicsPipelineData.depthStencilState = { 
        Gfx::StencilOpState(), Gfx::StencilOpState(),
        Gfx::CompareOp::Less, false, false, true, true, 0, 1
    };

    m_graphicsPipelineData.graphicsPipelineInfo.setLayout(m_graphicsPipelineData.pipelineLayout)
        .setRenderPass(m_renderPassData.renderPass)
        .setSubpass(0)
        .setStages(m_graphicsPipelineData.shaderStages)
        .setVertexInputState(m_graphicsPipelineData.vertexInputState)
        .setInputAssemblyState(m_graphicsPipelineData.inputAssemblyState)
        .setDynamicState(m_graphicsPipelineData.dynamicState)
        .setViewportState(m_graphicsPipelineData.viewportState)
        .setRasterizationState(m_graphicsPipelineData.rasterizationState)
        .setMultisampleState(m_graphicsPipelineData.multisampleState)
        .setColorBlendState(m_graphicsPipelineData.colorBlendState)
        .setDepthStencilState(m_graphicsPipelineData.depthStencilState);
    m_graphicsPipelineData.graphicsPipeline.create(m_device.getFunctionTable(), 
        m_device, m_graphicsPipelineData.graphicsPipelineInfo);

    // Gfx::Utility::createBasicGraphicsPipeline(m_graphicsPipelineData,
    //     m_device.getFunctionTable(), m_device, m_renderPassData.renderPass,
    //     0, Gfx::PrimitiveTopology::TriangleList,
    //     Gfx::PolygonMode::Fill, Gfx::Flags::CullMode::Bits::Back,
    //     Gfx::FrontFace::CounterClockwise,
    //     Gfx::CompareOp::Always, true, true);

    //m_computePipeline = Gfx::ComputePipeline(m_instance, m_device,
    //    m_shaderCache.getShader(ShaderCache::ShaderPurpose::VoxelCompute),
    //    { &m_storageLayout, &m_perChunkLayout, &m_configLayout },
    //    { Gfx::PushConstantRange{ Gfx::ShaderStage::Bits::Compute, 0, sizeof(PushConstants)} });

    m_graphicsCommandPool.create(m_device.getFunctionTable(), m_device,
        Gfx::CommandPoolCreateInfo(graphicsIndex));
    m_temporaryBufferPool.create(m_device.getFunctionTable(), m_device,
        Gfx::CommandPoolCreateInfo(graphicsIndex));

    //uint32_t maxSets, std::span<const DescriptorPoolSize> poolSizes,
    //    Flags::DescriptorPoolCreate flags = Flags::DescriptorPoolCreate::Bits::None

    std::vector<Gfx::DescriptorPoolSize> poolSizes = {
        Gfx::DescriptorPoolSize(s_framesInFlight + 2, Gfx::DescriptorType::UniformBuffer),
        Gfx::DescriptorPoolSize(11 + 3 + 1024, Gfx::DescriptorType::StorageBuffer),
		Gfx::DescriptorPoolSize(1024, Gfx::DescriptorType::CombinedImageSampler)
    };

    m_descriptorPool.create(m_device.getFunctionTable(), m_device,
        Gfx::DescriptorPoolCreateInfo(s_framesInFlight + 3, poolSizes,
            Gfx::Flags::DescriptorPoolCreate::Bits::UpdateAfterBind));

    {
        Gfx::DescriptorSetAllocateInfo allocInfo;
        //std::vector<Gfx::DescriptorSetAllocateInfo> layoutsVariableSized;
        //std::vector<uint32_t> descriptorCounts;

        //for (size_t i = 0; i < framesInFlight; ++i)
        //    layouts.push_back(&m_perFrameLayout);

        allocInfo.setDescriptorPool(m_descriptorPool)
			.setDescriptorSetLayouts(m_descriptorSetLayouts);

        m_descriptorSets = m_descriptorPool.allocateSets(m_device.getFunctionTable(), 
            m_device, allocInfo);
    }

    //for (int i = 0; i < framesInFlight; ++i)
    //{
    //    m_perFrameInFlightObjects[i].uniformBuffer = Gfx::Buffer(m_instance, m_device,
    //        Gfx::Buffer::Descriptor(sizeof(Gfx::UniformTransforms),
    //            Gfx::BufferUsage::Bits::Uniform, false));
    //}

    //m_uniformMemory = Gfx::MappedMemory(m_instance, m_device,
    //    m_perFrameInFlightObjects[0].uniformBuffer.getMemoryRequirements(),
    //    Gfx::MemoryProperty::Bits::HostVisibleCoherent,
    //    sizeof(Gfx::UniformTransforms) * framesInFlight);

    for (size_t i = 0; i < s_framesInFlight; ++i)
    {
        m_perFrameInFlightObjects[i].graphicsCommandBuffer = 
            m_graphicsCommandPool.allocateCommandBuffer(m_device.getFunctionTable(), m_device);
		m_perFrameInFlightObjects[i].imageAvailableSemaphore.create(m_device.getFunctionTable(), m_device);
        m_perFrameInFlightObjects[i].renderFinishedSemaphore.create(m_device.getFunctionTable(), m_device);
        m_perFrameInFlightObjects[i].inFlightFence.create(m_device.getFunctionTable(), m_device,
            { Gfx::Flags::FenceCreate::Bits::Signaled });

        //m_perFrameInFlightObjects[i].perFrameSet = sets[i];
        //m_uniformMemory.bindBuffer(m_instance, m_device,
        //    m_perFrameInFlightObjects[i].uniformBuffer, sizeof(Gfx::UniformTransforms) * i);
        //m_perFrameInFlightObjects[i].perFrameSet->write(m_instance, m_device,
        //    m_perFrameInFlightObjects[i].uniformBuffer, 0,
        //    0, sizeof(Gfx::UniformTransforms));
    }
    m_sampler.create(m_device.getFunctionTable(), m_device, Gfx::SamplerCreateInfo::defaultSampler());

    configureMemory();

    std::vector<Gfx::DescriptorPoolSize> imguiPoolSizes = {
        Gfx::DescriptorPoolSize(128, Gfx::DescriptorType::UniformBuffer),
        Gfx::DescriptorPoolSize(16, Gfx::DescriptorType::StorageBuffer),
        Gfx::DescriptorPoolSize(512, Gfx::DescriptorType::CombinedImageSampler)
    };

    m_imguiDescriptorPool.create(m_device.getFunctionTable(), m_device,
        Gfx::DescriptorPoolCreateInfo(512, imguiPoolSizes, Gfx::Flags::DescriptorPoolCreate::Bits::FreeDescriptorSet));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

#ifdef _WIN32
    ImGui_ImplWin32_Init(window.getHandle());
    window.registerCallback<Platform::WindowEvents::Native>([](Platform::Win32::WindowHandle windowHandle, uint32_t message,
            Platform::Win32::MessageValueUnsigned auxiliaryData, Platform::Win32::MessageValueSigned payloadData) {
        return ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(windowHandle), 
            static_cast<UINT>(message), static_cast<WPARAM>(auxiliaryData), 
            static_cast<LPARAM>(payloadData));
        });
#else
    //others here
#endif
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_instance.getHandle();
    init_info.PhysicalDevice = m_physicalDevice.getHandle();
    init_info.Device = m_device.getHandle();
    init_info.QueueFamily = graphicsIndex;
    init_info.Queue = m_graphicsQueue.getHandle();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_imguiDescriptorPool.getHandle();
    init_info.RenderPass = m_renderPassData.renderPass.getHandle();
    init_info.Subpass = 0;
    init_info.MinImageCount = m_swapChainData.swapChainImages.size();
    init_info.ImageCount = m_swapChainData.swapChainImages.size();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();
}

void Renderer::configureMemory()
{
    m_deviceMemoryProps = m_physicalDevice.getMemoryProperties(m_instance.getFunctionTable());
    
    Gfx::Utility::createBufferMemoryPairFirstFit(m_device.getFunctionTable(), m_device, m_deviceMemoryProps,
        m_stagingBuffer, m_stagingMemory, 1024 * 1024 * 32, Gfx::Flags::BufferUsage::Bits::TransferSrc,
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent);
    m_stagingMapping = m_stagingMemory.map(m_device.getFunctionTable(), m_device);
    m_stagingMemorySize = 1024 * 1024 * 32;

    Gfx::Utility::createBufferMemoryPairFirstFit(m_device.getFunctionTable(), m_device, m_deviceMemoryProps,
        m_configBuffer, m_configMemory, 256 + m_chunkCount * Constants::chunkSize,
        Gfx::Flags::BufferUsage::Bits::UniformBuffer | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent);
    m_configMapping = m_configMemory.map(m_device.getFunctionTable(), m_device);

    Gfx::DescriptorBufferInfo bufferInfo;
    bufferInfo.setBuffer(m_configBuffer)
        .setOffset(0)
        .setRange(sizeof(Config));

    Gfx::DescriptorSetWrite writeSet;
    writeSet.setBufferInfo(bufferInfo)
        .setDstBinding(0)
        .setDstSet(m_descriptorSets[static_cast<size_t>(DescriptorSetIndex::Config)])
        .setDescriptorType(Gfx::DescriptorType::UniformBuffer)
        .setDstArrayElement(0);

    Gfx::DescriptorSet::update(m_device.getFunctionTable(), m_device, writeSet);

    m_configMapping.get<Config>()->contrast = 0.8f;
}

void Renderer::resetChunkBuffers(const WorldGrid& grid)
{
    if(m_gridBuffer.isValid())
    {   
        m_gridBuffer.destroy(m_device.getFunctionTable(), m_device);
        m_chunkBuffer.destroy(m_device.getFunctionTable(), m_device);
        m_drawCommandsBuffer.destroy(m_device.getFunctionTable(), m_device);

        m_gridMemory.destroy(m_device.getFunctionTable(), m_device);
        m_chunkMemory.destroy(m_device.getFunctionTable(), m_device);
        m_drawCommandsMemory.destroy(m_device.getFunctionTable(), m_device);
    }

    m_chunkCount = grid.getPool().getPoolSize();

    size_t blockCount = m_chunkCount * Constants::chunkSize;

    auto gridBufferMemReq = Gfx::Utility::createBufferMemoryPairFirstFit(m_device.getFunctionTable(), 
        m_device, m_deviceMemoryProps, m_gridBuffer, m_gridMemory, blockCount * sizeof(Id::VoxelState),
        Gfx::Flags::BufferUsage::Bits::StorageBuffer, Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent);
    m_gridMapping = m_gridMemory.map(m_device.getFunctionTable(), m_device);

    auto chunkBufferMemReq = Gfx::Utility::createBufferMemoryPairFirstFit(m_device.getFunctionTable(), 
        m_device, m_deviceMemoryProps, m_chunkBuffer, m_chunkMemory, m_chunkCount * sizeof(WorldGrid::Chunk),
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer, Gfx::Flags::MemoryProperty::Bits::DeviceLocal,
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent);

    auto drawCommandBufferMemReq = Gfx::Utility::createBufferMemoryPairFirstFit(m_device.getFunctionTable(), 
        m_device, m_deviceMemoryProps, m_drawCommandsBuffer, m_drawCommandsMemory, m_chunkCount * sizeof(PoolDrawCommand),
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::IndirectBuffer 
        | Gfx::Flags::BufferUsage::Bits::StorageBuffer, Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent);
    m_drawCommandsMapping = m_drawCommandsMemory.map(m_device.getFunctionTable(), m_device);

    m_indicesPool.create(m_device.getFunctionTable(), m_device, m_deviceMemoryProps,
        sizeof(Indices) * 8 * Constants::chunkSize * m_chunkCount, 
        Gfx::Flags::BufferUsage::Bits::StorageBuffer | Gfx::Flags::BufferUsage::Bits::TransferDst, 
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, Gfx::Flags::MemoryProperty::Bits::None, 
        Gfx::SharingMode::Exclusive, [this](Gfx::MemoryRef memory, Gfx::BufferRef buffer, size_t bufferIndex)
        { this->onPoolBufferAlloc(memory, buffer, bufferIndex); });
        
    std::memset(m_drawCommandsMapping.get(), 0, m_chunkCount * sizeof(PoolDrawCommand));

    std::vector<Gfx::DescriptorBufferInfo> bufferInfos = {
        { m_gridBuffer, 0, blockCount * sizeof(Id::VoxelState) },
        { m_chunkBuffer, 0, m_chunkCount * sizeof(WorldGrid::Chunk) },
        { m_drawCommandsBuffer, 0, m_chunkCount * sizeof(PoolDrawCommand) }
    };

    std::vector<Gfx::DescriptorSetWrite> writes = {
        {m_descriptorSets[static_cast<size_t>(DescriptorSetIndex::PerChunk)], 0, 0, bufferInfos[0], Gfx::DescriptorType::StorageBuffer },
        {m_descriptorSets[static_cast<size_t>(DescriptorSetIndex::PerChunk)], 1, 0, bufferInfos[1], Gfx::DescriptorType::StorageBuffer },
        {m_descriptorSets[static_cast<size_t>(DescriptorSetIndex::PerChunk)], 2, 0, bufferInfos[2], Gfx::DescriptorType::StorageBuffer }
    };

    Gfx::DescriptorSet::update(m_device.getFunctionTable(), m_device, writes);

    m_indexAllocations.resize(m_chunkCount, 
        Gfx::MemoryManagement::MemoryPool::Allocation::getEmptyAllocation());

    m_stagingBuffers.resize(m_poolHandle->getWorkerCount());

    for (size_t i = 0; i < m_stagingBuffers.size(); ++i)
        m_stagingBuffers[i].reserve(Constants::chunkSize * 120);

    m_drawIndexToPoolIndex.clear();
    m_chunkDrawIndices.clear();
    m_chunkDrawIndices.resize(m_chunkCount);
    m_meshedChunks.resize(m_chunkCount, false);

    m_drawCommandAmount = 0;
}

void Renderer::createAndWriteAssets(AssetCache& assetCache, 
    const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStateCache)
{
    auto memProps = m_physicalDevice.getMemoryProperties(m_instance.getFunctionTable());    
    assetCache.moveAssetsToGpuStorage(m_device, memProps,
        m_transferQueue, m_temporaryBufferPool, m_stagingBuffer,
        m_stagingMapping, m_stagingMemorySize, voxelStateCache);
    assetCache.writeToDescriptors(m_device, m_descriptorSets[static_cast<size_t>(DescriptorSetIndex::Storage)], 
        m_sampler, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
}

void Renderer::createLayouts()
{
    Gfx::Utility::DescriptorDefinitionBuilder <
        GridDataLayoutDefinition<>,
        ChunkDataLayoutDefinition<>,
        CommandBufferLayoutDefinition<>> perChunkBuilder;
    Gfx::Utility::DescriptorDefinitionBuilder<
        StorageCache::VertexLayoutDefinition<0>,
        StorageCache::UvLayoutDefinition<1>,
        StorageCache::NormalLayoutDefinition<2>,
        StorageCache::PolygonLayoutDefinition<3>,
        StorageCache::ColoringLayoutDefinition<4>,
        StorageCache::PolygonIndexLayoutDefinition<5>,
        StorageCache::ColoringIndexLayoutDefinition<6>,
        StorageCache::AppearenceLayoutDefinition<7>,
        StorageCache::GeometryLayoutDefinition<8>,
        StorageCache::ModelLayoutDefinition<9>,
        StorageCache::StateToModelLayoutDefinition<10>,
        StorageCache::ImageLayoutDefinition<11>> storageBuilder;
    Gfx::Utility::DescriptorDefinitionBuilder<
        PoolLayoutDefinition<>> poolBuilder;
    Gfx::Utility::DescriptorDefinitionBuilder<
        ConfigLayoutDefinition<>> configBuilder;

    m_descriptorLayoutCreateInfos[static_cast<size_t>(DescriptorSetIndex::PerChunk)]
        .setBindings(perChunkBuilder.enumerateDescriptors());
	m_descriptorLayoutCreateInfos[static_cast<size_t>(DescriptorSetIndex::Storage)]
        .setBindings(storageBuilder.enumerateDescriptors());
    m_descriptorLayoutCreateInfos[static_cast<size_t>(DescriptorSetIndex::Pool)]
        .setBindings(poolBuilder.enumerateDescriptors());
    m_descriptorLayoutCreateInfos[static_cast<size_t>(DescriptorSetIndex::Config)]
		.setBindings(configBuilder.enumerateDescriptors());

    m_descriptorSetLayouts[static_cast<size_t>(DescriptorSetIndex::PerChunk)]
        .create(m_device.getFunctionTable(), m_device,
			m_descriptorLayoutCreateInfos[static_cast<size_t>(DescriptorSetIndex::PerChunk)]);
    m_descriptorSetLayouts[static_cast<size_t>(DescriptorSetIndex::Storage)]
		.create(m_device.getFunctionTable(), m_device,
			m_descriptorLayoutCreateInfos[static_cast<size_t>(DescriptorSetIndex::Storage)]);
	m_descriptorSetLayouts[static_cast<size_t>(DescriptorSetIndex::Pool)]
		.create(m_device.getFunctionTable(), m_device,
			m_descriptorLayoutCreateInfos[static_cast<size_t>(DescriptorSetIndex::Pool)]);
	m_descriptorSetLayouts[static_cast<size_t>(DescriptorSetIndex::Config)]
		.create(m_device.getFunctionTable(), m_device,
			m_descriptorLayoutCreateInfos[static_cast<size_t>(DescriptorSetIndex::Config)]);
}

void Renderer::cleanup(StorageCache& cache)
{
    auto lock = m_poolHandle->pausePool();
    m_device.waitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

	cache.destroy(m_device);
    
    m_temporaryBufferPool.destroy(m_device.getFunctionTable(), m_device);    
    for (size_t i = 0; i < s_framesInFlight; ++i)
    {        
        m_perFrameInFlightObjects[i].imageAvailableSemaphore.destroy(m_device.getFunctionTable(), m_device);
        m_perFrameInFlightObjects[i].renderFinishedSemaphore.destroy(m_device.getFunctionTable(), m_device);
        m_perFrameInFlightObjects[i].inFlightFence.destroy(m_device.getFunctionTable(), m_device);
    }

    for(size_t i = 0; i < m_descriptorSetLayouts.size(); ++i) {
        m_descriptorSetLayouts[i].destroy(m_device.getFunctionTable(), m_device);
    }

    m_graphicsCommandPool.reset(m_device.getFunctionTable(), m_device);
    m_graphicsCommandPool.destroy(m_device.getFunctionTable(), m_device);

    m_sampler.destroy(m_device.getFunctionTable(), m_device);

    m_stagingBuffer.destroy(m_device.getFunctionTable(), m_device);
    
    m_stagingMapping.unmap(m_device.getFunctionTable(), m_device, m_stagingMemory);
    m_stagingMemory.destroy(m_device.getFunctionTable(), m_device);
    m_descriptorPool.destroy(m_device.getFunctionTable(), m_device);

    m_gridBuffer.destroy(m_device.getFunctionTable(), m_device);
    m_chunkBuffer.destroy(m_device.getFunctionTable(), m_device);
    m_drawCommandsBuffer.destroy(m_device.getFunctionTable(), m_device);

    m_gridMapping.unmap(m_device.getFunctionTable(), m_device, m_gridMemory);
    m_gridMemory.destroy(m_device.getFunctionTable(), m_device);
    m_chunkMemory.destroy(m_device.getFunctionTable(), m_device);

    m_drawCommandsMapping.unmap(m_device.getFunctionTable(), m_device, m_drawCommandsMemory);
    m_drawCommandsMemory.destroy(m_device.getFunctionTable(), m_device);

    m_configMapping.unmap(m_device.getFunctionTable(), m_device, m_configMemory);
    m_configMemory.destroy(m_device.getFunctionTable(), m_device);
    m_configBuffer.destroy(m_device.getFunctionTable(), m_device);

    m_imguiDescriptorPool.destroy(m_device.getFunctionTable(), m_device);

    m_indicesPool.destroy(m_device.getFunctionTable(), m_device);

    m_graphicsPipelineData.graphicsPipeline.destroy(m_device.getFunctionTable(), m_device);
    m_graphicsPipelineData.pipelineLayout.destroy(m_device.getFunctionTable(), m_device);
    m_renderPassData.renderPass.destroy(m_device.getFunctionTable(), m_device);

    Gfx::Utility::destroySwapChainData(m_device.getFunctionTable(), m_device, m_swapChainData);

    // m_pipeline.destroy(m_device.getFunctionTable(), m_device);

    // m_configLayout.destroy(m_device.getFunctionTable(), m_device);
    // m_perChunkLayout.destroy(m_device.getFunctionTable(), m_device);
    // m_storageLayout.destroy(m_device.getFunctionTable(), m_device);
    // m_poolLayout.destroy(m_device.getFunctionTable(), m_device);

    m_shaderCache.destroy(m_device);

    // m_swapChain.destroy(m_device.getFunctionTable(), m_device);
    // m_renderPass.destroy(m_device.getFunctionTable(), m_device);
    
    m_surface.destroy(m_instance.getFunctionTable(), m_instance);
    m_device.destroy();

    m_instance.destroy();
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
    std::lock_guard<std::mutex> lockStaging(m_stagingBufferLock);
    m_device.waitIdle();
    auto surfaceCapabilities = m_physicalDevice.getSurfaceCapabilities(m_instance.getFunctionTable(), m_surface);
    m_canvas = Gfx::RenderRegion::createFullWindow(surfaceCapabilities.getCurrentExtent());

    Gfx::Utility::recreateBasicSwapChain(m_swapChainData, m_device.getFunctionTable(),
        m_device, m_renderPassData.renderPass, m_physicalDevice.getMemoryProperties(m_instance.getFunctionTable()), 
        Gfx::Utility::chooseExtent(surfaceCapabilities, extent));
    ImGui_ImplVulkan_SetMinImageCount(m_swapChainData.swapChainImages.size());
    
    // m_format = Gfx::SwapChainFormat::create(m_instance, m_device, m_surface, extent);
    // m_swapChain.recreate(m_instance, m_device, m_surface, m_renderPass, m_format,
    //     m_presentQueue.getFamily(), m_graphicsQueue.getFamily());
    // ImGui_ImplVulkan_SetMinImageCount(newImageCount);
}

void Renderer::drawFrame(const Gfx::Utility::CameraPerspective& camera)
{
    m_pushConstants.proj = camera.getProjection();
    m_pushConstants.view = camera.getView();
    m_pushConstants.viewProj = m_pushConstants.proj * m_pushConstants.view;

    m_perFrameInFlightObjects[m_currentFrame].inFlightFence.wait(m_device.getFunctionTable(), m_device);
    std::unique_lock<std::shared_mutex> lock(m_drawLock);
    m_perFrameInFlightObjects[m_currentFrame].inFlightFence.reset(m_device.getFunctionTable(), m_device);
        
    uint32_t imageIndex;
    auto imageAquireResult = m_swapChainData.swapChain.acquireNextImage(m_device.getFunctionTable(), m_device,
        m_perFrameInFlightObjects[m_currentFrame].imageAvailableSemaphore, imageIndex);

    if (imageAquireResult == Gfx::Result::ErrorOutOfDateKHR ||
        imageAquireResult == Gfx::Result::SuboptimalKHR) return;

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.reset(m_device.getFunctionTable());
    
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.begin(
        m_device.getFunctionTable(), Gfx::CommandBufferBeginInfo()
    );

    std::array<Gfx::ClearValue, 2> clearValues = {
        Gfx::ClearColorValue(Gfx::Color::green()),
        Gfx::ClearDepthStencilValue{1.0f, 0}
    };

    Gfx::RenderPassBeginInfo renderPassBeginInfo = { m_renderPassData.renderPass, 
        m_swapChainData.swapChainFrameBuffers[imageIndex], clearValues, {0, 0}, 
        m_swapChainData.swapChainInfo.getImageExtent() };

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.beginRenderPass(m_device.getFunctionTable(),
        renderPassBeginInfo, Gfx::SubpassContents::Inline);
        
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.bindPipeline(
        m_device.getFunctionTable(), m_graphicsPipelineData.graphicsPipeline, Gfx::PipelineBindPoint::Graphics);

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.setViewport(m_device.getFunctionTable(), m_canvas.getViewport());
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.setScissor(m_device.getFunctionTable(), m_canvas.getScissor());

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.bindDescriptorSets(m_device.getFunctionTable(), Gfx::PipelineBindPoint::Graphics,
        m_graphicsPipelineData.pipelineLayout, 0, m_descriptorSets, {});

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.pushConstants(m_device.getFunctionTable(), m_graphicsPipelineData.pipelineLayout,
        Gfx::Flags::ShaderStage::Bits::Vertex, 0, sizeof(PushConstants), &m_pushConstants);

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.drawIndirect(m_device.getFunctionTable(),
        m_drawCommandsBuffer, 0, m_drawCommandAmount, sizeof(PoolDrawCommand));

    drawGui(camera);

    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.endRenderPass(m_device.getFunctionTable());
    m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.stopRecord(m_device.getFunctionTable());

    try
    {
        std::array<Gfx::Flags::PipelineStage, 1> pipelineStage = { Gfx::Flags::PipelineStage::Bits::ColorAttachmentOutput };

        Gfx::QueueSubmitInfo submitInfo(
            std::span(&m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer, 1),
            pipelineStage,
            std::span(&m_perFrameInFlightObjects[m_currentFrame].imageAvailableSemaphore, 1),
            std::span(&m_perFrameInFlightObjects[m_currentFrame].renderFinishedSemaphore, 1));

        m_graphicsQueue.submit(m_device.getFunctionTable(), submitInfo,
            m_perFrameInFlightObjects[m_currentFrame].inFlightFence);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to submit graphics queue: " << e.what() << std::endl;
    }

    Gfx::QueuePresentInfo presentInfo = {
        std::span(&m_perFrameInFlightObjects[m_currentFrame].renderFinishedSemaphore, 1),
        std::span(&m_swapChainData.swapChain, 1),
        std::span(&imageIndex, 1),
    };
    auto presentResult = m_presentQueue.present(m_device.getFunctionTable(), presentInfo);
    if (presentResult == Gfx::Result::ErrorOutOfDateKHR ||
        presentResult == Gfx::Result::SuboptimalKHR) return;

    m_currentFrame = (m_currentFrame + 1) % s_framesInFlight;
}

void Renderer::drawGui(const Gfx::Utility::CameraPerspective& camera)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(550, 700), ImGuiCond_Always);

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
        ImGui::Text("Block count:       %.2u", static_cast<uint32_t>(m_chunkCount * Constants::chunkSize));
    }
    ImGui::Text("Memory Pool Usage:");
    size_t bufferCount = m_indicesPool.getMemoryChunkCount();
    ImGui::Text("Total buffer count: %u", static_cast<uint32_t>(bufferCount));

    for (size_t i = 0; i < bufferCount; ++i)
        drawMemoryPoolVisualization(i);

    /*Compass::drawCoordinateAxes(camera, ImVec2(100, 500));*/

    ImGui::End();

    m_debugConsole.draw();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_perFrameInFlightObjects[m_currentFrame].graphicsCommandBuffer.getHandle());
}

void Renderer::updateChunk(const ResourceCache& resources, size_t chunkPoolIndex, const WorldGrid& grid, size_t threadId)
{
    auto startStaging = std::chrono::high_resolution_clock::now();

    auto& chunk = grid.getPool().getField<1>()[chunkPoolIndex];

    auto& assets = resources.getAssetCache();
    auto& states = resources.getVoxelStateCache();
    auto& models = assets.getModelCache();
    auto& geometries = assets.getGeometryCache();
    auto& appearances = assets.getAppearanceCache();
    auto& geometriesCache = geometries.entryCache();
    auto& appearancesCache = appearances.entryCache();
	auto& cullingCache = assets.getVoxelCullingCache();

    auto& buffer = m_stagingBuffers[threadId];

    size_t chunkEnd = chunk.start + Constants::chunkSize;
    for (size_t block = chunk.start; block < chunkEnd; ++block)
    {
        cullingCache.populateBuffer(block,
            (block - chunk.start) % Constants::chunkWidth,
            (block - chunk.start) / Constants::chunkLayerSize,
            ((block - chunk.start) % Constants::chunkLayerSize) / Constants::chunkWidth,
            chunk, grid, buffer, states, models, geometriesCache,
            appearancesCache, geometries, appearances);
    }

    auto endStaging = std::chrono::high_resolution_clock::now();
    auto stagingDuration = std::chrono::duration_cast<std::chrono::microseconds>(endStaging - startStaging).count();

    auto startAllocation = std::chrono::high_resolution_clock::now();
    auto& allocation = m_indexAllocations[chunkPoolIndex];

    if (buffer.size() == 0)
    {
        unmeshChunk(chunkPoolIndex);
        return;
    }
    // else if (allocation.region.size > buffer.size() * sizeof(Indices))
    // {
    //     std::unique_lock<std::mutex> lock(m_poolLock);
    //     m_indicesPool.shrink(allocation, buffer.size() * sizeof(Indices));
    // }
    // else if (allocation.region.size < buffer.size() * sizeof(Indices))
    else
    {
        std::unique_lock<std::mutex> lock(m_poolLock);
        m_indicesPool.free(allocation);
        allocation = m_indicesPool.allocate(m_device.getFunctionTable(), m_device, buffer.size() * sizeof(Indices),
            [this](Gfx::MemoryRef memory, Gfx::BufferRef buffer, size_t bufferIndex) {
                (void)memory;
                Gfx::DescriptorBufferInfo bufferInfo = {
                    buffer, 0, m_indicesPool.getChunkSize()
                };
                m_descriptorSets[static_cast<size_t>(DescriptorSetIndex::Pool)].write(
                    m_device.getFunctionTable(), m_device, 0, bufferIndex, bufferInfo,
                    Gfx::DescriptorType::StorageBuffer);
            }
        );
    }

    auto endAllocation = std::chrono::high_resolution_clock::now();
    auto allocationDuration = std::chrono::duration_cast<std::chrono::microseconds>(endAllocation - startAllocation).count();

    auto startMemoryPopulate = std::chrono::high_resolution_clock::now();
    {        
        {
            std::array<Gfx::DataTransferInfo, 2> transferInfos = {
                Gfx::DataTransferInfo{std::span<const Indices>(buffer), m_indicesPool.getBuffer(allocation.bufferIndex), 
                    static_cast<size_t>(allocation.region.offset)},
                Gfx::DataTransferInfo{&chunk, 0, m_chunkBuffer, sizeof(WorldGrid::Chunk) * chunkPoolIndex, sizeof(WorldGrid::Chunk)}
            };

            std::lock_guard<std::mutex> lockStaging(m_stagingBufferLock);
            
            m_temporaryBufferPool.oneTimeDataTransfer(m_device.getFunctionTable(), m_device, 
                m_transferQueue, transferInfos, m_stagingBuffer, m_stagingMapping, m_stagingMemorySize);
        }
        
        std::shared_lock<std::shared_mutex> lockDraw(m_drawLock);
        m_perFrameInFlightObjects[m_currentFrame].inFlightFence.wait(m_device.getFunctionTable(), m_device);
        std::lock_guard<std::mutex> lockCommand(m_drawCommandLock);
        if (!m_meshedChunks[chunkPoolIndex])
        {
            m_chunkDrawIndices[chunkPoolIndex] = m_drawCommandAmount++;
            m_drawIndexToPoolIndex.insert({ m_chunkDrawIndices[chunkPoolIndex], chunkPoolIndex });
        }
        
        auto commands = m_drawCommandsMapping.get<PoolDrawCommand>(0, m_drawCommandAmount);
        auto& command = commands[m_chunkDrawIndices[chunkPoolIndex]];
        command.drawCommand.vertexCount = 3;
        command.drawCommand.instanceCount = buffer.size();
        command.drawCommand.firstVertex = 0;
        command.drawCommand.firstInstance = allocation.region.offset / sizeof(Indices);
        command.bufferId = allocation.bufferIndex;
    }
    buffer.clear();
    m_meshedChunks[chunkPoolIndex] = true;
    
    auto endMemoryPopulate = std::chrono::high_resolution_clock::now();
    auto memoryPopulateDuration = std::chrono::duration_cast<std::chrono::microseconds>(endMemoryPopulate - startMemoryPopulate).count();
    
    m_debugConsole.log("Chunk {} updated with {} indices, "
        "Allocation: size {}, offset {}, buffer {}\n"
        "Timings (μs): Staging: {}, Allocation: {}, MemoryPopulate: {}\n",
        chunkPoolIndex, buffer.size(),
        allocation.region.size, allocation.region.offset, allocation.bufferIndex,
        stagingDuration, allocationDuration, memoryPopulateDuration);
}

void Renderer::unmeshChunk(size_t chunkPoolIndex)
{
    if (!m_meshedChunks[chunkPoolIndex])
    {
        m_debugConsole.log("Chunk {} was generated empty\n", chunkPoolIndex);
        return;
    }

    auto startUnmesh = std::chrono::high_resolution_clock::now();
    {
        std::lock_guard<std::mutex> lockCommand(m_drawCommandLock);
        auto index = m_chunkDrawIndices[chunkPoolIndex];
        const auto& itCur = m_drawIndexToPoolIndex.find(index);
        const auto& itNew = m_drawIndexToPoolIndex.find(m_drawCommandAmount--);
        itCur->second = itNew->second;
        m_chunkDrawIndices[itCur->second] = index;
        m_drawIndexToPoolIndex.erase(itNew);

        std::shared_lock<std::shared_mutex> lockDraw(m_drawLock);
        m_perFrameInFlightObjects[m_currentFrame].inFlightFence.wait(m_device.getFunctionTable(), m_device);        
        auto commands = m_drawCommandsMapping.get<PoolDrawCommand>(0, m_drawCommandAmount);
        commands[index] = commands.back();
    }

    auto& allocation = m_indexAllocations[chunkPoolIndex];
    if (allocation.region.size != 0)
    {
        std::unique_lock<std::mutex> lock(m_poolLock);
        m_indicesPool.free(allocation);
    }
    m_meshedChunks[chunkPoolIndex] = false;

    auto endUnmesh = std::chrono::high_resolution_clock::now();
    auto unmeshDuration = std::chrono::duration_cast<std::chrono::microseconds>(endUnmesh - startUnmesh).count();

    m_debugConsole.log("Chunk {} unmeshed\n Timings (μs): Unmesh: {}\n", chunkPoolIndex, unmeshDuration);
}

void Renderer::updateChunkAsync(const ResourceCache& resources, size_t chunkIndex, const WorldGrid& grid)
{
    m_poolHandle->pushTask([this, &resources, chunkIndex, &grid](size_t threadId) {
        updateChunk(resources, chunkIndex, grid, threadId);
        });
}

void Renderer::drawMemoryPoolVisualization(size_t chunkIndex) {
    (void)chunkIndex;
    // std::unique_lock<std::mutex> lock(m_poolLock);
    // uint32_t totalSize = m_indicesPool.getChunkSize();
    // uint32_t allocatedSize = m_indicesPool.getChunkAllocatedSize(chunkIndex);
    // uint32_t freeSize = m_indicesPool.getChunkFreeSize(chunkIndex);
    // const auto& freeRegions = m_indicesPool.getFreeRegions(chunkIndex);
    // const auto& allocations = m_indicesPool.getAllocations(chunkIndex);

    // ImGui::Text("Buffer size: %u Buffer usage: %u Buffer free: %u", totalSize, allocatedSize, freeSize);
    // ImGui::Text("Allocation count: %u free block count: %u", allocations.size(), freeRegions.size());

    // ImVec2 barSize(ImGui::GetContentRegionAvail().x, 20.0f);
    // ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    // ImDrawList* drawList = ImGui::GetWindowDrawList();

    // for (const auto& block : freeRegions) {
    //     float width = (block.size / (float)totalSize) * barSize.x;
    //     float offset = (block.offset / (float)totalSize) * barSize.x;
    //     drawList->AddRectFilled(
    //         ImVec2(cursorPos.x + offset, cursorPos.y),
    //         ImVec2(cursorPos.x + offset + width, cursorPos.y + barSize.y),
    //         IM_COL32(100, 100, 100, 255)
    //     );
    // }

    // for (const auto& block : allocations) {
    //     float width = (block.region.size / (float)totalSize) * barSize.x;
    //     float offset = (block.region.offset / (float)totalSize) * barSize.x;
    //     drawList->AddRectFilled(
    //         ImVec2(cursorPos.x + offset, cursorPos.y),
    //         ImVec2(cursorPos.x + offset + width, cursorPos.y + barSize.y),
    //         IM_COL32(255, 0, 0, 255)
    //     );
    //     offset += width;
    // }

    // drawList->AddRect(
    //     cursorPos,
    //     ImVec2(cursorPos.x + barSize.x, cursorPos.y + barSize.y),
    //     IM_COL32(255, 255, 255, 255)
    // );

    // ImGui::Dummy(barSize);
}

void Renderer::onPoolBufferAlloc(Gfx::MemoryRef memory, Gfx::BufferRef buffer, size_t bufferIndex)
{
    (void)memory;
    m_descriptorSets[static_cast<size_t>(DescriptorSetIndex::Pool)]
        .write(m_device.getFunctionTable(), m_device, 0, bufferIndex, 
        { buffer, 0, m_indicesPool.getChunkSize() }, Gfx::DescriptorType::StorageBuffer);
}

void Renderer::dumpHandles() {
    for(size_t i = 0; i < m_perFrameInFlightObjects.size(); ++i) {
        std::cout << "graphicsCommandBuffer[" << i << "]: " << m_perFrameInFlightObjects[i].graphicsCommandBuffer.getNumerical() << std::endl;
        std::cout << "imageAvailableSemaphore[" << i << "]: " << m_perFrameInFlightObjects[i].imageAvailableSemaphore.getNumerical() << std::endl;
        std::cout << "inFlightFence[" << i << "]: " << m_perFrameInFlightObjects[i].inFlightFence.getNumerical() << std::endl;
        std::cout << "renderFinishedSemaphore[" << i << "]: " << m_perFrameInFlightObjects[i].renderFinishedSemaphore.getNumerical() << std::endl;
    }

    std::cout << "m_instance: " << m_instance.getReference().getNumerical() << std::endl;
    std::cout << "m_physicalDevice: " << m_physicalDevice.getNumerical() << std::endl;
    std::cout << "m_device: " << m_device.getReference().getNumerical() << std::endl;

    std::cout << "m_renderPassData.renderPass: " << m_renderPassData.renderPass.getNumerical() << std::endl;
    std::cout << "m_swapChainData.swapChain: " << m_swapChainData.swapChain.getNumerical() << std::endl;
    std::cout << "m_pipelineData.pipelineLayout: " << m_graphicsPipelineData.pipelineLayout.getNumerical() << std::endl;
    std::cout << "m_pipelineData.graphicsPipeline: " << m_graphicsPipelineData.graphicsPipeline.getNumerical() << std::endl;

    std::cout << "m_graphicsCommandPool: " << m_graphicsCommandPool.getNumerical() << std::endl;
    std::cout << "m_temporaryBufferPool: " << m_temporaryBufferPool.getNumerical() << std::endl;
    std::cout << "m_descriptorPool: " << m_descriptorPool.getNumerical() << std::endl;
    std::cout << "m_imguiDescriptorPool: " << m_imguiDescriptorPool.getNumerical() << std::endl;

    std::cout << "m_graphicsQueue: " << m_graphicsQueue.getNumerical() << std::endl;
    std::cout << "m_presentQueue: " << m_presentQueue.getNumerical() << std::endl;
    std::cout << "m_transferQueue: " << m_transferQueue.getNumerical() << std::endl;

    std::cout << "m_descriptorSetLayouts[DescriptorSetIndex::Config)]: " <<
        m_descriptorSetLayouts[static_cast<size_t>(DescriptorSetIndex::Config)].getNumerical() << std::endl;
    std::cout << "m_descriptorSetLayouts[DescriptorSetIndex::PerChunk)]: " <<
        m_descriptorSetLayouts[static_cast<size_t>(DescriptorSetIndex::PerChunk)].getNumerical() << std::endl;
    std::cout << "m_descriptorSetLayouts[DescriptorSetIndex::Pool)]: " <<
        m_descriptorSetLayouts[static_cast<size_t>(DescriptorSetIndex::Pool)].getNumerical() << std::endl;
    std::cout << "m_descriptorSetLayouts[DescriptorSetIndex::Storage)]: " <<
        m_descriptorSetLayouts[static_cast<size_t>(DescriptorSetIndex::Storage)].getNumerical() << std::endl;

    for(size_t i = 0; i < m_descriptorSets.size(); ++i) {
        std::cout << "m_descriptorSet[ " << i << "]: " << m_descriptorSets[i].getNumerical() << std::endl;
    }

    std::cout << "m_stagingMemory: " << m_stagingMemory.getNumerical() << std::endl;
    std::cout << "m_stagingBuffer: " << m_stagingBuffer.getNumerical() << std::endl;

    std::cout << "m_sampler: " << m_sampler.getNumerical() << std::endl;
    std::cout << "m_surface: " << m_surface.getNumerical() << std::endl;

    std::cout << "m_gridBuffer: " << m_gridBuffer.getNumerical() << std::endl;
    std::cout << "m_chunkBuffer: " << m_chunkBuffer.getNumerical() << std::endl;
    std::cout << "m_drawCommandsBuffer: " << m_drawCommandsBuffer.getNumerical() << std::endl;

    std::cout << "m_gridMemory: " << m_gridMemory.getNumerical() << std::endl;

    std::cout << "m_chunkMemory: " << m_chunkMemory.getNumerical() << std::endl;
    
    std::cout << "m_drawCommandsMemory: " << m_drawCommandsMemory.getNumerical() << std::endl;
    
    std::cout << "m_configMemory: " << m_configMemory.getNumerical() << std::endl;
    std::cout << "m_configBuffer: " << m_configBuffer.getNumerical() << std::endl;
}