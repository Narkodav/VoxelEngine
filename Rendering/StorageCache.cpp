#include "StorageCache.h"

void StorageCache::setup(const Gfx::Context& instance, const Gfx::Device& device,
    const Gfx::Queue& transferQueue, Gfx::CommandPool& temporaryPool, Gfx::MappedMemory& stagingMemory,
    Gfx::Buffer& stagingBuffer,
    const Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertexCache,
    const Id::Cache<glm::vec2, Id::Uv, VecEpsilonEqualComparator<glm::vec2>>& uvCache,
    const Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normalCache,
    const Id::NamedCache<Voxel::State, Id::VoxelState>& stateCache,
    const Id::NamedCache<Shape::Model, Id::Model>& modelCache,
    const Shape::PolygonIndexBuffer& geometryCache,
    const Shape::ColoringIndexBuffer& appearenceCache,
    const Id::Cache<Shape::Polygon, Id::Polygon>& polygonCache,
    const Id::Cache<Shape::Coloring, Id::Coloring>& coloringCache,
    const Id::NamedCache<Gfx::PixelData, Id::Texture>& textureCache)
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

    m_vertexBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_vertexBufferSize,
            Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_vertexMemory = Gfx::Memory(instance, device, m_vertexBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_vertexBuffer.getMemoryRequirements().size);
    m_vertexMemory.bindBuffer(instance, device, m_vertexBuffer);
    temporaryPool.makeOneTimeDataTransfer<glm::vec4>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_vertexBuffer,
        vertexCache.data());

    m_uvBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_uvBufferSize,
            Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_uvMemory = Gfx::Memory(instance, device, m_uvBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_uvBuffer.getMemoryRequirements().size);
    m_uvMemory.bindBuffer(instance, device, m_uvBuffer);
    temporaryPool.makeOneTimeDataTransfer<glm::vec2>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_uvBuffer,
        uvCache.data());

    m_normalBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_normalBufferSize,
            Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_normalMemory = Gfx::Memory(instance, device, m_normalBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_normalBuffer.getMemoryRequirements().size);
    m_normalMemory.bindBuffer(instance, device, m_normalBuffer);
    temporaryPool.makeOneTimeDataTransfer<glm::vec2>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_normalBuffer,
        normalCache.data());

    m_polygonBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_polygonBufferSize,
        Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_polygonMemory = Gfx::Memory(instance, device, m_polygonBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_polygonBuffer.getMemoryRequirements().size);
    m_polygonMemory.bindBuffer(instance, device, m_polygonBuffer);
    temporaryPool.makeOneTimeDataTransfer<Shape::Polygon>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_polygonBuffer,
        polygonCache.data());

    m_coloringBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_coloringBufferSize,
        Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_coloringMemory = Gfx::Memory(instance, device, m_coloringBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_coloringBuffer.getMemoryRequirements().size);
    m_coloringMemory.bindBuffer(instance, device, m_coloringBuffer);
    temporaryPool.makeOneTimeDataTransfer<Shape::Coloring>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_coloringBuffer,
        coloringCache.data());

    m_polygonIndexBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_polygonIndexBufferSize,
        Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_polygonIndexMemory = Gfx::Memory(instance, device, m_polygonIndexBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_polygonIndexBuffer.getMemoryRequirements().size);
    m_polygonIndexMemory.bindBuffer(instance, device, m_polygonIndexBuffer);
    temporaryPool.makeOneTimeDataTransfer<uint32_t>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_polygonIndexBuffer,
        geometryCache.indexData());

    m_coloringIndexBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_coloringIndexBufferSize,
        Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_coloringIndexMemory = Gfx::Memory(instance, device, m_coloringIndexBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_coloringIndexBuffer.getMemoryRequirements().size);
    m_coloringIndexMemory.bindBuffer(instance, device, m_coloringIndexBuffer);
    temporaryPool.makeOneTimeDataTransfer<uint32_t>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_coloringIndexBuffer,
        appearenceCache.indexData());

    m_geometryBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_geometryBufferSize,
        Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_geometryMemory = Gfx::Memory(instance, device, m_geometryBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_geometryBuffer.getMemoryRequirements().size);
    m_geometryMemory.bindBuffer(instance, device, m_geometryBuffer);
    temporaryPool.makeOneTimeDataTransfer<Shape::PolygonIndexBuffer::Entry>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_geometryBuffer,
        geometryCache.entryData());

    m_appearenceBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_appearenceBufferSize,
        Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_appearenceMemory = Gfx::Memory(instance, device, m_appearenceBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_appearenceBuffer.getMemoryRequirements().size);
    m_appearenceMemory.bindBuffer(instance, device, m_appearenceBuffer);
    temporaryPool.makeOneTimeDataTransfer<Shape::ColoringIndexBuffer::Entry>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_appearenceBuffer,
        appearenceCache.entryData());

    m_modelBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_modelBufferSize,
        Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_modelMemory = Gfx::Memory(instance, device, m_modelBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_modelBuffer.getMemoryRequirements().size);
    m_modelMemory.bindBuffer(instance, device, m_modelBuffer);
    temporaryPool.makeOneTimeDataTransfer<Shape::Model>(
        instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_modelBuffer,
        modelCache.data());

    m_stateToModelBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(m_stateToModelBufferSize,
        Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Storage, false));
    m_stateToModelMemory = Gfx::Memory(instance, device, m_stateToModelBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal, m_stateToModelBuffer.getMemoryRequirements().size);
    m_stateToModelMemory.bindBuffer(instance, device, m_stateToModelBuffer);
    temporaryPool.makeOneTimeSubmit(instance, device, transferQueue,
        [this, &instance, &stagingMemory, &stagingBuffer, &stateCache]
        (Gfx::CommandBufferHandle temporary) {
            auto mappedMemory = stagingMemory.getMapping<uint32_t>(stateCache.size());
            for (size_t i = 0; i < mappedMemory.size(); ++i)
                mappedMemory[i] = stateCache[i].m_model;
            temporary->transferBufferData(instance, stagingBuffer,
                m_stateToModelBuffer, Gfx::CopyRegion(0, 0,
                    mappedMemory.size() * sizeof(uint32_t)));
        });

    
    m_images.reserve(textureCache.size());
    m_imageMemories.resize(textureCache.size());

    for (size_t i = 0; i < textureCache.size(); i++)
    {
        m_images.push_back(Gfx::Image(instance, device, 
        textureCache[i].getWidth(), textureCache[i].getHeight()));
        m_imageMemories[i] = Gfx::Memory(instance, device, m_images[i].getMemoryRequirements(),
            Gfx::MemoryProperty::Bits::DeviceLocal, m_images[i].getMemoryRequirements().size);
        m_imageMemories[i].bindImage(instance, device, m_images[i]);
        m_images[i].createView(instance, device);
    }

    for (size_t i = 0; i < m_images.size(); ++i)
    {
        temporaryPool.makeOneTimeImageDataTransfer(instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_images[i], textureCache[i]);       
    }
}

void StorageCache::writeToDescriptors(const Gfx::Context& instance, const Gfx::Device& device,
    Gfx::DescriptorSetHandle& descriptorStorage, const Gfx::Sampler& sampler,
    uint32_t bindingVertex, uint32_t bindingUv, uint32_t bindingNormal, uint32_t bindingPolygon,
    uint32_t bindingColoring, uint32_t bindingPolygonIndex, uint32_t bindingColoringIndex,
    uint32_t bindingGeometry, uint32_t bindingAppearence, uint32_t bindingModel,
    uint32_t bindingStateToModel, uint32_t bindingImage) const
{
    descriptorStorage->write(instance, device, m_vertexBuffer, bindingVertex, 0, m_vertexBufferSize);
    descriptorStorage->write(instance, device, m_uvBuffer, bindingUv, 0, m_uvBufferSize);
    descriptorStorage->write(instance, device, m_polygonBuffer, bindingPolygon, 0, m_polygonBufferSize);
    descriptorStorage->write(instance, device, m_coloringBuffer, bindingColoring, 0, m_coloringBufferSize);
    descriptorStorage->write(instance, device, m_polygonIndexBuffer, bindingPolygonIndex, 0, m_polygonIndexBufferSize);
    descriptorStorage->write(instance, device, m_coloringIndexBuffer, bindingColoringIndex, 0, m_coloringIndexBufferSize);
    descriptorStorage->write(instance, device, m_geometryBuffer, bindingGeometry, 0, m_geometryBufferSize);
    descriptorStorage->write(instance, device, m_appearenceBuffer, bindingAppearence, 0, m_appearenceBufferSize);
    descriptorStorage->write(instance, device, m_modelBuffer, bindingModel, 0, m_modelBufferSize);
    descriptorStorage->write(instance, device, m_stateToModelBuffer, bindingStateToModel, 0, m_stateToModelBufferSize);

    std::vector<const Gfx::Sampler*> samplerHandles(m_images.size(), &sampler);
    descriptorStorage->write(instance, device, m_images, samplerHandles, bindingImage);
}

void StorageCache::destroy(const Gfx::Context& instance, const Gfx::Device& device)
{
    m_vertexBuffer.destroy(instance, device);
    m_uvBuffer.destroy(instance, device);
    m_normalBuffer.destroy(instance, device);
    m_polygonBuffer.destroy(instance, device);
    m_coloringBuffer.destroy(instance, device);
    m_polygonIndexBuffer.destroy(instance, device);
    m_coloringIndexBuffer.destroy(instance, device);
    m_geometryBuffer.destroy(instance, device);
    m_appearenceBuffer.destroy(instance, device);
    m_modelBuffer.destroy(instance, device);
    m_stateToModelBuffer.destroy(instance, device);
    
    m_vertexMemory.destroy(instance, device);
    m_uvMemory.destroy(instance, device);
    m_normalMemory.destroy(instance, device);
    m_polygonMemory.destroy(instance, device);
    m_coloringMemory.destroy(instance, device);
    m_polygonIndexMemory.destroy(instance, device);
    m_coloringIndexMemory.destroy(instance, device);
    m_geometryMemory.destroy(instance, device);
    m_appearenceMemory.destroy(instance, device);
    m_modelMemory.destroy(instance, device);
    m_stateToModelMemory.destroy(instance, device);

    for (size_t i = 0; i < m_images.size(); i++)
        m_images[i].destroy(instance, device);
    for (auto& memory : m_imageMemories)
        memory.destroy(instance, device);
}