#include "Rendering/StorageCache.h"

void StorageCache::setup(const Gfx::Wrappers::Device& device, 
    const Gfx::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
    Gfx::Queue transferQueue, Gfx::CommandPoolRef temporaryPool, Gfx::BufferRef stagingBuffer, 
    Gfx::MemoryMapping& stagingMapping, size_t stagingBuffeSize,
    const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertexCache,
    const Id::Cache<glm::vec2, Id::Uv, VecEpsilonEqualComparator<glm::vec2>>& uvCache,
    const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normalCache,
    const Id::NamedCache<Voxel::State, Id::VoxelState>& stateCache,
    const Id::NamedCache<Shape::Model, Id::Model>& modelCache,
    const Shape::PolygonIndexBuffer& geometryCache,
    const Shape::ColoringIndexBuffer& appearenceCache,
    const Id::Cache<Shape::Polygon, Id::Polygon>& polygonCache,
    const Id::Cache<Shape::Coloring, Id::Coloring>& coloringCache,
    const Id::NamedCache<Gfx::Utility::PixelData2D, Id::Texture>& textureCache)
{
    m_vertexBufferSize = vertexCache.size() * sizeof(glm::vec4);
    m_uvBufferSize = uvCache.size() * sizeof(glm::vec2);
    m_normalBufferSize = normalCache.size() * sizeof(glm::vec4);
    m_polygonBufferSize = polygonCache.size() * sizeof(Shape::Polygon);
    m_coloringBufferSize = coloringCache.size() * sizeof(Shape::Coloring);
    m_polygonIndexBufferSize = geometryCache.indexSize() * sizeof(uint32_t);
    m_coloringIndexBufferSize = appearenceCache.indexSize() * sizeof(uint32_t);
    m_geometryBufferSize = geometryCache.entrySize() * sizeof(Shape::PolygonIndexBuffer::Entry);
    m_appearenceBufferSize = appearenceCache.entrySize() * sizeof(Shape::ColoringIndexBuffer::Entry);
    m_modelBufferSize = modelCache.size() * sizeof(Shape::Model);
    m_stateToModelBufferSize = stateCache.size() * sizeof(uint32_t);

    m_vertexBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_vertexBuffer, m_vertexMemory, m_vertexBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_uvBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_uvBuffer, m_uvMemory, m_uvBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_normalBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_normalBuffer, m_normalMemory, m_normalBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_polygonBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_polygonBuffer, m_polygonMemory, m_polygonBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_coloringBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_coloringBuffer, m_coloringMemory, m_coloringBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_polygonIndexBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_polygonIndexBuffer, m_polygonIndexMemory, m_polygonIndexBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_coloringIndexBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_coloringIndexBuffer, m_coloringIndexMemory, m_coloringIndexBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_geometryBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_geometryBuffer, m_geometryMemory, m_geometryBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_appearenceBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_appearenceBuffer, m_appearenceMemory, m_appearenceBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_modelBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_modelBuffer, m_modelMemory, m_modelBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);

    m_stateToModelBufferMemoryRequirements = Gfx::Utility::createBufferMemoryPairFirstFit(
        device.getFunctionTable(), device, physicalDeviceMemoryProperties,
        m_stateToModelBuffer, m_stateToModelMemory, m_stateToModelBufferSize,
        Gfx::Flags::BufferUsage::Bits::TransferDst | Gfx::Flags::BufferUsage::Bits::StorageBuffer,
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated);
        
    std::vector<Gfx::DataTransferInfo> dataTransferInfos = {
        {std::span(vertexCache.data()),             m_vertexBuffer},
        {std::span(uvCache.data()),                 m_uvBuffer},
        {std::span(normalCache.data()),             m_normalBuffer},
        {std::span(polygonCache.data()),            m_polygonBuffer},
        {std::span(coloringCache.data()),           m_coloringBuffer},
        {std::span(geometryCache.indexData()),      m_polygonIndexBuffer},
        {std::span(appearenceCache.indexData()),    m_coloringIndexBuffer},
        {std::span(geometryCache.entryData()),      m_geometryBuffer},
        {std::span(appearenceCache.entryData()),    m_appearenceBuffer},
        {std::span(modelCache.data()),              m_modelBuffer}
    };

    temporaryPool.oneTimeDataTransfer(device.getFunctionTable(), device, transferQueue,
        dataTransferInfos, stagingBuffer, stagingMapping, stagingBuffeSize);

    temporaryPool.oneTimeDataTransfer<uint32_t, Voxel::State>(device.getFunctionTable(), device, transferQueue,
			stateCache.data(), m_stateToModelBuffer, stagingBuffer, 0, stagingMapping, 
            [](uint32_t& model, const Voxel::State& state){ model = state.m_model; });

    m_images.resize(textureCache.size());
    m_imageMemories.resize(textureCache.size());
    m_imageCreateInfos.resize(textureCache.size());
    m_imageMemoryRequirements.resize(textureCache.size());
    m_imageViewCreateInfos.resize(textureCache.size());
    m_imageViews.resize(textureCache.size());

    for (size_t i = 0; i < textureCache.size(); i++)
    {
        m_imageCreateInfos[i] = { Gfx::ImageType::Image2D, Gfx::PixelFormat::R8G8B8A8Srgb,
            {static_cast<uint32_t>(textureCache[i].getWidth()), static_cast<uint32_t>(textureCache[i].getHeight()), 1},
            Gfx::Utility::calculateMipLevelCount(textureCache[i].getWidth(), textureCache[i].getHeight()),
            1, Gfx::Flags::SampleCount::Bits::SC1, Gfx::ImageTiling::Optimal,
            Gfx::Flags::ImageUsage::Bits::TransferDst | Gfx::Flags::ImageUsage::Bits::Sampled |
            Gfx::Flags::ImageUsage::Bits::TransferSrc, Gfx::SharingMode::Exclusive
        };

        m_images[i].create(device.getFunctionTable(), device, m_imageCreateInfos[i]);
        m_imageMemoryRequirements[i] = m_images[i].getMemoryRequirements(device.getFunctionTable(), device);
        m_imageMemories[i].create(device.getFunctionTable(), device, 
        {m_imageMemoryRequirements[i].getSize(), Gfx::Utility::findMemoryTypeFirstFit(physicalDeviceMemoryProperties,
        m_imageMemoryRequirements[i].getMemoryTypeBits(), 
        Gfx::Flags::MemoryProperty::Bits::DeviceLocal, 
        Gfx::Flags::MemoryProperty::Bits::HostVisibleCoherent | Gfx::Flags::MemoryProperty::Bits::LazilyAllocated)
        });
        m_imageMemories[i].bindImage(device.getFunctionTable(), device, m_images[i]);
        
        m_imageViewCreateInfos[i] = { m_images[i], Gfx::ImageViewType::T2D, Gfx::PixelFormat::R8G8B8A8Srgb,
            {
                Gfx::ComponentSwizzle::Identity, Gfx::ComponentSwizzle::Identity, 
                Gfx::ComponentSwizzle::Identity, Gfx::ComponentSwizzle::Identity
            },
            {
                Gfx::Flags::ImageAspect::Bits::Color,
                0, m_imageCreateInfos[i].getMipLevels(), 0, 1
            }
        };
        m_imageViews[i].create(device.getFunctionTable(), device, m_imageViewCreateInfos[i]);
    }

    temporaryPool.oneTimeSubmit(device.getFunctionTable(), device, transferQueue, 
        [&] (Gfx::Result result, Gfx::CommandBuffer& temporary) {
                if(result != Gfx::Result::Success) return result;
                size_t offset = 0;
                for (size_t i = 0; i < m_images.size(); ++i)
                {
                    uint32_t mipWidth = textureCache[i].getWidth();
                    uint32_t mipHeight = textureCache[i].getHeight();
                    uint32_t mipWidthNext = 0;
                    uint32_t mipHeightNext = 0;
                    uint32_t mipLevelCount = m_imageCreateInfos[i].getMipLevels();

                    Gfx::ImageMemoryBarrier barrierMipStart = {
                        m_images[i],
                        Gfx::ImageLayout::Undefined,
                        Gfx::ImageLayout::TransferDstOptimal,
                        Gfx::Flags::Access::Bits::None,
                        Gfx::Flags::Access::Bits::TransferWrite,
                        Gfx::ImageMemoryBarrier::s_queueFamilyIgnored,
                        Gfx::ImageMemoryBarrier::s_queueFamilyIgnored,
                        m_imageViewCreateInfos[i].getSubresourceRange()
                    };

                    temporary.setPipelineBarrier(device.getFunctionTable(),
                        Gfx::Flags::PipelineStage::Bits::TopOfPipe,
                        Gfx::Flags::PipelineStage::Bits::Transfer,
                        Gfx::Flags::Dependency::Bits::None, {}, {},
                        std::span(&barrierMipStart, 1)
                    );

                    auto pixelData = textureCache[i].getPixelData();
                    std::memcpy(stagingMapping.get(offset), pixelData.data(), pixelData.size());

                    Gfx::BufferImageCopy imageCopy = {
                        offset, 0, 0, { Gfx::Flags::ImageAspect::Bits::Color, 0, 0, 1 },
                        {0, 0, 0}, textureCache[i].getExtent3D()
                    };

                    temporary.copyBufferToImage(device.getFunctionTable(), stagingBuffer,
                        m_images[i], Gfx::ImageLayout::TransferDstOptimal, imageCopy);

                    offset += pixelData.size();

                    Gfx::ImageSubresourceLayers layerThis;
                    Gfx::ImageSubresourceLayers layerNext = { Gfx::Flags::ImageAspect::Bits::Color, 0, 0, 1 };                    
                    Gfx::ImageMemoryBarrier barrierMipEnd;

                    for (uint32_t j = 0; j < mipLevelCount - 1; ++j) {
                        layerThis = layerNext;
                        layerNext = { Gfx::Flags::ImageAspect::Bits::Color, j + 1, 0, 1 };

                        barrierMipStart = {
                            m_images[i],
                            Gfx::ImageLayout::TransferDstOptimal,
                            Gfx::ImageLayout::TransferSrcOptimal,
                            Gfx::Flags::Access::Bits::TransferWrite,
                            Gfx::Flags::Access::Bits::TransferRead,
                            Gfx::ImageMemoryBarrier::s_queueFamilyIgnored,
                            Gfx::ImageMemoryBarrier::s_queueFamilyIgnored,
                            {
                                Gfx::Flags::ImageAspect::Bits::Color,
                                j, 1, 0, 1
                            }
                        };

                        temporary.setPipelineBarrier(device.getFunctionTable(),
                            Gfx::Flags::PipelineStage::Bits::Transfer,
                            Gfx::Flags::PipelineStage::Bits::Transfer,
                            Gfx::Flags::Dependency::Bits::None, {}, {},
                            std::span(&barrierMipStart, 1)
                        );

                        if (mipWidth > 1u) mipWidthNext = mipWidth / 2u; else mipWidthNext = 1u;
                        if (mipHeight > 1u) mipHeightNext = mipHeight / 2u; else mipHeightNext = 1u;

                        Gfx::ImageBlit blit = {
                            layerThis,
                            { 0, 0, 0 }, { static_cast<int32_t>(mipWidth), static_cast<int32_t>(mipHeight), 1 },
                            layerNext,
                            { 0, 0, 0 }, { static_cast<int32_t>(mipWidthNext), static_cast<int32_t>(mipHeightNext), 1 },
                        };

                        temporary.blitImage(device.getFunctionTable(), m_images[i], m_images[i],
                        Gfx::ImageLayout::TransferSrcOptimal, Gfx::ImageLayout::TransferDstOptimal,
                        std::span(&blit, 1), Gfx::Filter::Linear);

                        mipWidth = mipWidthNext;
                        mipHeight = mipHeightNext;

                        barrierMipEnd = {
                            m_images[i],
                            Gfx::ImageLayout::TransferSrcOptimal,
                            Gfx::ImageLayout::ShaderReadOnlyOptimal,
                            Gfx::Flags::Access::Bits::TransferRead,
                            Gfx::Flags::Access::Bits::ShaderRead,
                            Gfx::ImageMemoryBarrier::s_queueFamilyIgnored,
                            Gfx::ImageMemoryBarrier::s_queueFamilyIgnored,
                            {
                                Gfx::Flags::ImageAspect::Bits::Color,
                                j, 1, 0, 1
                            }
                        };

                        temporary.setPipelineBarrier(device.getFunctionTable(),
                            Gfx::Flags::PipelineStage::Bits::Transfer,
                            Gfx::Flags::PipelineStage::Bits::FragmentShader,
                            Gfx::Flags::Dependency::Bits::None, {}, {},
                            std::span(&barrierMipEnd, 1)
                        );
                    }

                    barrierMipEnd = {
                        m_images[i],
                        Gfx::ImageLayout::TransferDstOptimal,
                        Gfx::ImageLayout::ShaderReadOnlyOptimal,
                        Gfx::Flags::Access::Bits::TransferWrite,
                        Gfx::Flags::Access::Bits::ShaderRead,
                        Gfx::ImageMemoryBarrier::s_queueFamilyIgnored,
                        Gfx::ImageMemoryBarrier::s_queueFamilyIgnored,
                        {
                            Gfx::Flags::ImageAspect::Bits::Color,
                            mipLevelCount - 1, 1, 0, 1
                        }
                    };

                    temporary.setPipelineBarrier(device.getFunctionTable(),
                        Gfx::Flags::PipelineStage::Bits::Transfer,
                        Gfx::Flags::PipelineStage::Bits::FragmentShader,
                        Gfx::Flags::Dependency::Bits::None, {}, {},
                        std::span(&barrierMipEnd, 1)
                    );                    
                }
                return result;
            }
        );
}

void StorageCache::writeToDescriptors(const Gfx::Wrappers::Device& device,
    Gfx::DescriptorSet descriptorStorage, Gfx::SamplerRef sampler,
    uint32_t bindingVertex, uint32_t bindingUv, uint32_t bindingNormal, uint32_t bindingPolygon,
    uint32_t bindingColoring, uint32_t bindingPolygonIndex, uint32_t bindingColoringIndex,
    uint32_t bindingGeometry, uint32_t bindingAppearence, uint32_t bindingModel,
    uint32_t bindingStateToModel, uint32_t bindingImage) const
{
    std::vector<Gfx::DescriptorBufferInfo> bufferInfos = {
        { m_vertexBuffer, 0, m_vertexBufferSize },
        { m_uvBuffer, 0, m_uvBufferSize },
        { m_normalBuffer, 0, m_normalBufferSize },
        { m_polygonBuffer, 0, m_polygonBufferSize },
        { m_coloringBuffer, 0, m_coloringBufferSize },
        { m_polygonIndexBuffer, 0, m_polygonIndexBufferSize },
        { m_coloringIndexBuffer, 0, m_coloringIndexBufferSize },
        { m_geometryBuffer, 0, m_geometryBufferSize },
        { m_appearenceBuffer, 0, m_appearenceBufferSize },
        { m_modelBuffer, 0, m_modelBufferSize },
        { m_stateToModelBuffer, 0, m_stateToModelBufferSize },
    };

    std::vector<Gfx::DescriptorImageInfo> imageInfos(m_images.size());

    for(size_t i = 0; i < m_images.size(); ++i) {
        imageInfos[i] = { sampler, m_imageViews[i], Gfx::ImageLayout::ShaderReadOnlyOptimal };
    }

    std::vector<Gfx::DescriptorSetWrite> writes = {
        { descriptorStorage, bindingVertex, 0, bufferInfos[0], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingUv, 0, bufferInfos[1], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingNormal, 0, bufferInfos[2], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingPolygon, 0, bufferInfos[3], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingColoring, 0, bufferInfos[4], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingPolygonIndex, 0, bufferInfos[5], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingColoringIndex, 0, bufferInfos[6], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingGeometry, 0, bufferInfos[7], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingAppearence, 0, bufferInfos[8], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingModel, 0, bufferInfos[9], Gfx::DescriptorType::StorageBuffer },
        { descriptorStorage, bindingStateToModel, 0, bufferInfos[10], Gfx::DescriptorType::StorageBuffer },

        { descriptorStorage, bindingImage, 0, imageInfos, Gfx::DescriptorType::CombinedImageSampler },
    };

    Gfx::DescriptorSet::update(device.getFunctionTable(), device, writes);
}

void StorageCache::destroy(const Gfx::Wrappers::Device& device)
{
    m_vertexBuffer.destroy(device.getFunctionTable(), device);
    m_uvBuffer.destroy(device.getFunctionTable(), device);
    m_normalBuffer.destroy(device.getFunctionTable(), device);
    m_polygonBuffer.destroy(device.getFunctionTable(), device);
    m_coloringBuffer.destroy(device.getFunctionTable(), device);
    m_polygonIndexBuffer.destroy(device.getFunctionTable(), device);
    m_coloringIndexBuffer.destroy(device.getFunctionTable(), device);
    m_geometryBuffer.destroy(device.getFunctionTable(), device);
    m_appearenceBuffer.destroy(device.getFunctionTable(), device);
    m_modelBuffer.destroy(device.getFunctionTable(), device);
    m_stateToModelBuffer.destroy(device.getFunctionTable(), device);
    
    m_vertexMemory.destroy(device.getFunctionTable(), device);
    m_uvMemory.destroy(device.getFunctionTable(), device);
    m_normalMemory.destroy(device.getFunctionTable(), device);
    m_polygonMemory.destroy(device.getFunctionTable(), device);
    m_coloringMemory.destroy(device.getFunctionTable(), device);
    m_polygonIndexMemory.destroy(device.getFunctionTable(), device);
    m_coloringIndexMemory.destroy(device.getFunctionTable(), device);
    m_geometryMemory.destroy(device.getFunctionTable(), device);
    m_appearenceMemory.destroy(device.getFunctionTable(), device);
    m_modelMemory.destroy(device.getFunctionTable(), device);
    m_stateToModelMemory.destroy(device.getFunctionTable(), device);

    for (size_t i = 0; i < m_images.size(); i++)
    {
        m_imageViews[i].destroy(device.getFunctionTable(), device);
        m_images[i].destroy(device.getFunctionTable(), device);
    }
    for (auto& memory : m_imageMemories)
        memory.destroy(device.getFunctionTable(), device);
}