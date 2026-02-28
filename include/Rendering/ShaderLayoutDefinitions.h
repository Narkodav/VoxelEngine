#pragma once
#include "Common.h"
#include "Graphics/Graphics.h"

struct Indices {
    uint32_t polygon;
    uint32_t coloring;
    uint32_t block;
};

struct VertexDefinitionPositionId : public Gfx::Utility::VertexDefinitionBase<VertexDefinitionPositionId> {
public:
    using Type = uint32_t;
    static constexpr size_t s_bindingCount = 1;
    static constexpr size_t s_attributeCount = 1;
    static constexpr size_t s_dataSize = sizeof(Type);

    static constexpr Gfx::VertexInputBindingDescription s_bindings[] = {
        {
            0,                                      // binding
            s_dataSize,                             // stride
            Gfx::VertexInputRate::Vertex            // input rate
        }
    };

    static constexpr Gfx::VertexInputAttributeDescription s_attributes[] = {
        {
            0,                                      // location
            0,                                      // binding
            Gfx::PixelFormat::R32Uint,              // format
            0                                       // offset
        }
    };
};

struct VertexDefinitionIndices : public Gfx::Utility::VertexDefinitionBase<VertexDefinitionIndices> {
public:
    using Type = Indices;
    static constexpr size_t s_bindingCount = 1;
    static constexpr size_t s_attributeCount = 3;
    static constexpr size_t s_dataSize = sizeof(Type);

    static constexpr Gfx::VertexInputBindingDescription s_bindings[] = {
        {
            1,                                      // binding
            s_dataSize,                             // stride
            Gfx::VertexInputRate::Instance          // input rate
        }
    };

    static constexpr Gfx::VertexInputAttributeDescription s_attributes[] = {
        {
            1,                                      // location
            1,                                      // binding
            Gfx::PixelFormat::R32Uint,                   // format
            0                                       // offset
        },
        {
            2,                                      // location
            1,                                      // binding
            Gfx::PixelFormat::R32Uint,              // format
            sizeof(uint32_t)                        // offset
        },
        {
            3,                                      // location
            1,                                      // binding
            Gfx::PixelFormat::R32Uint,              // format
            sizeof(uint32_t) * 2                    // offset
        },
    };
};

template <size_t binding = 0>
struct ConfigLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<ConfigLayoutDefinition<binding>> {
    static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
            binding,                                        // binding
            Gfx::DescriptorType::UniformBuffer,				// descriptor type
            1,												// descriptor count
            Gfx::Flags::ShaderStage::Bits::Fragment |
            Gfx::Flags::ShaderStage::Bits::Vertex |
            Gfx::Flags::ShaderStage::Bits::Compute,         // stage flags
    };

    static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = Gfx::Flags::DescriptorBinding::Bits::None;
};

template <size_t binding = 0>
struct GridDataLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<GridDataLayoutDefinition<binding>> {
    static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
            binding,                                        // binding
            Gfx::DescriptorType::StorageBuffer,	            // descriptor type
            1,												// descriptor count
            Gfx::Flags::ShaderStage::Bits::Vertex |
            Gfx::Flags::ShaderStage::Bits::Compute |
            Gfx::Flags::ShaderStage::Bits::MeshEXT,         // stage flags
    };

    static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = Gfx::Flags::DescriptorBinding::Bits::None;
};

template <size_t binding = 1>
struct ChunkDataLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<ChunkDataLayoutDefinition<binding>> {
    static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
            binding,                                        // binding
            Gfx::DescriptorType::StorageBuffer,				// descriptor type
            1,												// descriptor count
            Gfx::Flags::ShaderStage::Bits::Vertex |
            Gfx::Flags::ShaderStage::Bits::Compute |
            Gfx::Flags::ShaderStage::Bits::MeshEXT,         // stage flags
    };

    static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = Gfx::Flags::DescriptorBinding::Bits::None;
};



template <size_t binding = 2>
struct CommandBufferLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<CommandBufferLayoutDefinition<binding>> {
    static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
            binding,                                        // binding
            Gfx::DescriptorType::StorageBuffer,				// descriptor type
            1,												// descriptor count
            Gfx::Flags::ShaderStage::Bits::Vertex |
            Gfx::Flags::ShaderStage::Bits::Compute |
            Gfx::Flags::ShaderStage::Bits::MeshEXT,         // stage flags
    };

    static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags = Gfx::Flags::DescriptorBinding::Bits::None;
};

template <size_t binding = 0>
struct PoolLayoutDefinition : public Gfx::Utility::DescriptorDefinitionBase<PoolLayoutDefinition<binding>> {
    static constexpr Gfx::DescriptorSetLayoutBinding s_descriptorSetLayoutBinding = {
            binding,                                        // binding
            Gfx::DescriptorType::StorageBuffer,				// descriptor type
            1024,										    // descriptor count
            Gfx::Flags::ShaderStage::Bits::Vertex,          // stage flags
    };

    static constexpr Gfx::Flags::DescriptorBinding s_descriptorBindingFlags =
        Gfx::Flags::DescriptorBinding::Bits::PartiallyBound |
        Gfx::Flags::DescriptorBinding::Bits::VariableDescriptorCount |
        Gfx::Flags::DescriptorBinding::Bits::UpdateAfterBind;
};