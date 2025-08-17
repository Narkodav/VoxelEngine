#pragma once
#include "Common.h"

#include "Graphics/BufferDataLayouts.h"

struct Indices {
    uint32_t polygon;
    uint32_t coloring;
    uint32_t block;
};

struct VertexDefinitionPositionId {
public:
    static constexpr size_t BINDING_COUNT = 1;
    static constexpr size_t ATTRIBUTE_COUNT = 1;
    static constexpr size_t DATA_SIZE = sizeof(uint32_t);

    using Type = uint32_t;

    static constexpr vk::VertexInputBindingDescription bindings[] = {
        {
            0,                                      // binding
            DATA_SIZE,                              // stride
            vk::VertexInputRate::eVertex            // input rate
        }
    };

    static constexpr vk::VertexInputAttributeDescription attributes[] = {
        {
            0,                                      // location
            0,                                      // binding
            vk::Format::eR32Uint,                   // format
            0                                       // offset
        }
    };

    static Gfx::ArrayInterface<vk::VertexInputBindingDescription> getBindingDescriptions() {
        return Gfx::ArrayInterface<vk::VertexInputBindingDescription>(bindings, BINDING_COUNT);
    }

    static Gfx::ArrayInterface<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
        return Gfx::ArrayInterface<vk::VertexInputAttributeDescription>(attributes, ATTRIBUTE_COUNT);
    }
};

struct VertexDefinitionIndices {
public:
    static constexpr size_t BINDING_COUNT = 1;
    static constexpr size_t ATTRIBUTE_COUNT = 3;
    static constexpr size_t DATA_SIZE = sizeof(Indices);

    using Type = Indices;

    static constexpr vk::VertexInputBindingDescription bindings[] = {
        {
            1,                                      // binding
            DATA_SIZE,                              // stride
            vk::VertexInputRate::eInstance          // input rate
        }
    };

    static constexpr vk::VertexInputAttributeDescription attributes[] = {
        {
            1,                                      // location
            1,                                      // binding
            vk::Format::eR32Uint,                   // format
            0                                       // offset
        },
        {
            2,                                      // location
            1,                                      // binding
            vk::Format::eR32Uint,                   // format
            sizeof(uint32_t)                        // offset
        },
        {
            3,                                      // location
            1,                                      // binding
            vk::Format::eR32Uint,                   // format
            sizeof(uint32_t) * 2                    // offset
        },
    };

    static Gfx::ArrayInterface<vk::VertexInputBindingDescription> getBindingDescriptions() {
        return Gfx::ArrayInterface<vk::VertexInputBindingDescription>(bindings, BINDING_COUNT);
    }

    static Gfx::ArrayInterface<vk::VertexInputAttributeDescription> getAttributeDescriptions() {
        return Gfx::ArrayInterface<vk::VertexInputAttributeDescription>(attributes, ATTRIBUTE_COUNT);
    }
};

template <size_t binding = 0>
struct ConfigLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                              // binding
            vk::DescriptorType::eUniformBuffer,				// descriptor type
            1,												// descriptor count
            vk::ShaderStageFlagBits::eFragment |
            vk::ShaderStageFlagBits::eCompute,              // stage flags
            nullptr                                         // immutable samplers
    };

    static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;

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

template <size_t binding = 1>
struct UsageLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                        // binding
            vk::DescriptorType::eStorageBuffer,				// descriptor type
            1,												// descriptor count
            vk::ShaderStageFlagBits::eFragment |
            vk::ShaderStageFlagBits::eCompute,               // stage flags
            nullptr                                         // immutable samplers
    };

    static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;

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

template <size_t binding = 2>
struct ErrorLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                        // binding
            vk::DescriptorType::eStorageBuffer,				// descriptor type
            1,												// descriptor count
            vk::ShaderStageFlagBits::eFragment |
            vk::ShaderStageFlagBits::eCompute,               // stage flags
            nullptr                                         // immutable samplers
    };

    static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;

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

template <size_t binding = 0>
struct GridDataLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                        // binding
            vk::DescriptorType::eStorageBuffer,				// descriptor type
            1,												// descriptor count
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eMeshEXT,              // stage flags
            nullptr                                         // immutable samplers
    };

    static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;

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

template <size_t binding = 1>
struct ChunkDataLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                        // binding
            vk::DescriptorType::eStorageBuffer,				// descriptor type
            1,												// descriptor count
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eMeshEXT,              // stage flags
            nullptr                                         // immutable samplers
    };

    static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;

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

template <size_t binding = 2>
struct IndicesBufferLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                        // binding
            vk::DescriptorType::eStorageBuffer,				// descriptor type
            1,												// descriptor count
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eMeshEXT,              // stage flags
            nullptr                                         // immutable samplers
    };

    static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;

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

template <size_t binding = 3>
struct CommandBufferLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                        // binding
            vk::DescriptorType::eStorageBuffer,				// descriptor type
            1,												// descriptor count
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eMeshEXT,              // stage flags
            nullptr                                         // immutable samplers
    };

    static constexpr vk::DescriptorBindingFlags descriptorSetLayoutBindingFlags;

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