#pragma once
#include "Shape.h"
#include "GameData/Voxel.h"

#include "Graphics/MemoryManagement/PixelData.h"
#include "Graphics/Rendering/CommandPool.h"
#include "Graphics/Rendering/CommandBuffer.h"

class StorageCache
{
public:

    template<size_t bindingVertex = 0>
    struct VertexLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingVertex,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingUv = 1>
    struct UvLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingUv,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingNormal = 2>
    struct NormalLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingNormal,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingPolygon = 3>
    struct PolygonLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingPolygon,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingColoring = 4>
    struct ColoringLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingColoring,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingPolygonIndex = 5>
    struct PolygonIndexLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingPolygonIndex,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingColoringIndex = 6>
    struct ColoringIndexLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingColoringIndex,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingGeometry = 7>
    struct GeometryLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingGeometry,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingAppearence = 8>
    struct AppearenceLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingAppearence,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingModel = 9>
    struct ModelLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingModel,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template<size_t bindingStateToModel = 10>
    struct StateToModelLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingStateToModel,
                vk::DescriptorType::eStorageBuffer,
                1,
                vk::ShaderStageFlagBits::eVertex |
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eMeshEXT,
                nullptr
        };
        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;
        static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding() { return descriptorSetLayoutBinding; };
        static vk::DescriptorBindingFlags getDescriptorBindingFlags() { return descriptorSetLayoutBindingFlags; };
    };

    template <size_t bindingImage = 11>
    struct ImageLayoutDefinition {
        static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
                bindingImage,                                        // binding
                vk::DescriptorType::eCombinedImageSampler,      // descriptor type
                1024,                                           // descriptor count
                vk::ShaderStageFlagBits::eFragment,             // stage flags
                nullptr                                         // immutable samplers
        };

        static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags =
            vk::DescriptorBindingFlagBits::ePartiallyBound |
            vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
            vk::DescriptorBindingFlagBits::eUpdateAfterBind;

        static vk::DescriptorSetLayoutBinding
            getDescriptorSetLayoutBinding()
        {
            return descriptorSetLayoutBinding;
        };

        static vk::DescriptorBindingFlags
            getDescriptorBindingFlags()
        {
            return descriptorSetLayoutBindingFlags;
        };
    };

private:
    Gfx::Buffer m_vertexBuffer;
    Gfx::Buffer m_uvBuffer;
    Gfx::Buffer m_normalBuffer;
    Gfx::Buffer m_polygonBuffer;
    Gfx::Buffer m_coloringBuffer;
    Gfx::Buffer m_polygonIndexBuffer;
    Gfx::Buffer m_coloringIndexBuffer;
    Gfx::Buffer m_geometryBuffer;
    Gfx::Buffer m_appearenceBuffer;
    Gfx::Buffer m_modelBuffer;
    Gfx::Buffer m_stateToModelBuffer;

    Gfx::Memory m_vertexMemory;
    Gfx::Memory m_uvMemory;
    Gfx::Memory m_normalMemory;
    Gfx::Memory m_polygonMemory;
    Gfx::Memory m_coloringMemory;
    Gfx::Memory m_polygonIndexMemory;
    Gfx::Memory m_coloringIndexMemory;
    Gfx::Memory m_geometryMemory;
    Gfx::Memory m_appearenceMemory;
    Gfx::Memory m_modelMemory;
    Gfx::Memory m_stateToModelMemory;

    std::vector<Gfx::Memory> m_imageMemories;
    std::vector<Gfx::Image> m_images;

    //in bytes
    size_t m_vertexBufferSize;
    size_t m_uvBufferSize;
    size_t m_normalBufferSize;
    size_t m_polygonBufferSize;
    size_t m_coloringBufferSize;
    size_t m_polygonIndexBufferSize;
    size_t m_coloringIndexBufferSize;
    size_t m_geometryBufferSize;
    size_t m_appearenceBufferSize;
    size_t m_modelBufferSize;
    size_t m_stateToModelBufferSize;

    std::vector<size_t> m_imageSizes;

public:

    using VertexCache = Id::Cache<glm::vec4, Id::Vertex>;
    using UvCache = Id::Cache<glm::vec2, Id::Uv>;

    void setup(const Gfx::Context& instance, const Gfx::Device& device,
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
        const Id::NamedCache<Gfx::PixelData, Id::Texture>& textureCache);

    void destroy(const Gfx::Context& instance, const Gfx::Device& device);

    void writeToDescriptors(const Gfx::Context& instance, const Gfx::Device& device,
        Gfx::DescriptorSetHandle& descriptorStorage, const Gfx::Sampler& sampler,
        uint32_t bindingVertex, uint32_t bindingUv, uint32_t bindingNormal, uint32_t bindingPolygon,
        uint32_t bindingColoring, uint32_t bindingPolygonIndex, uint32_t bindingColoringIndex,
        uint32_t bindingGeometry, uint32_t bindingAppearence, uint32_t bindingModel,
        uint32_t bindingStateToModel, uint32_t bindingImage) const;
};

