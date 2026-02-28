#pragma once
#include "Rendering/Shape.h"
#include "GameData/Voxel.h"

#include "Common.h"

class StorageCache
{
public:

    template <size_t binding = 0>
    struct VertexLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<VertexLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 1>
    struct UvLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<UvLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 2>
    struct NormalLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<NormalLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 3>
    struct PolygonLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<PolygonLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 4>
    struct ColoringLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<ColoringLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 5>
    struct PolygonIndexLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<PolygonIndexLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 6>
    struct ColoringIndexLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<ColoringIndexLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 7>
    struct GeometryLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<GeometryLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 8>
    struct AppearenceLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<AppearenceLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 9>
    struct ModelLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<ModelLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 10>
    struct StateToModelLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<StateToModelLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::StorageBuffer,
                1,
                Gfx::Flags::ShaderStage::Bits::Vertex |
                Gfx::Flags::ShaderStage::Bits::Compute |
                Gfx::Flags::ShaderStage::Bits::MeshEXT,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = {};
    };

    template <size_t binding = 11>
    struct ImageLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<ImageLayoutDefinition<binding>> {
        static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
                binding,
                Gfx::DescriptorType::CombinedImageSampler,
                1024,
                Gfx::Flags::ShaderStage::Bits::Fragment,
        };

        static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags =
            Gfx::Flags::DescriptorBinding::Bits::PartiallyBound |
            Gfx::Flags::DescriptorBinding::Bits::VariableDescriptorCount |
            Gfx::Flags::DescriptorBinding::Bits::UpdateAfterBind;
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
    std::vector<Gfx::ImageCreateInfo> m_imageCreateInfos;
    std::vector<Gfx::ImageView> m_imageViews;
    std::vector<Gfx::ImageViewCreateInfo> m_imageViewCreateInfos;
    std::vector<Gfx::MemoryRequirements> m_imageMemoryRequirements;

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

    Gfx::MemoryRequirements m_vertexBufferMemoryRequirements;
    Gfx::MemoryRequirements m_uvBufferMemoryRequirements;
    Gfx::MemoryRequirements m_normalBufferMemoryRequirements;
    Gfx::MemoryRequirements m_polygonBufferMemoryRequirements;
    Gfx::MemoryRequirements m_coloringBufferMemoryRequirements;
    Gfx::MemoryRequirements m_polygonIndexBufferMemoryRequirements;
    Gfx::MemoryRequirements m_coloringIndexBufferMemoryRequirements;
    Gfx::MemoryRequirements m_geometryBufferMemoryRequirements;
    Gfx::MemoryRequirements m_appearenceBufferMemoryRequirements;
    Gfx::MemoryRequirements m_modelBufferMemoryRequirements;
    Gfx::MemoryRequirements m_stateToModelBufferMemoryRequirements;

    std::vector<size_t> m_imageSizes;

public:

    using VertexCache = Id::Cache<glm::vec4, Id::Vertex>;
    using UvCache = Id::Cache<glm::vec2, Id::Uv>;

    void setup(const Gfx::Wrappers::Device& device, 
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
        const Id::NamedCache<Gfx::Utility::PixelData2D, Id::Texture>& textureCache);

    void destroy(const Gfx::Wrappers::Device& device);

    void writeToDescriptors(const Gfx::Wrappers::Device& device,
        Gfx::DescriptorSet descriptorStorage, Gfx::SamplerRef sampler,
        uint32_t bindingVertex, uint32_t bindingUv, uint32_t bindingNormal, uint32_t bindingPolygon,
        uint32_t bindingColoring, uint32_t bindingPolygonIndex, uint32_t bindingColoringIndex,
        uint32_t bindingGeometry, uint32_t bindingAppearence, uint32_t bindingModel,
        uint32_t bindingStateToModel, uint32_t bindingImage) const;
};

