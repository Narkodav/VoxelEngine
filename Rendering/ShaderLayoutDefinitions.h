#pragma once
#include "Common.h"

template <size_t binding = 0>
struct ContrastLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                              // binding
            vk::DescriptorType::eUniformBuffer,				// descriptor type
            1,												// descriptor count
            vk::ShaderStageFlagBits::eFragment,             // stage flags
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

//layout(set = 2, binding = 0, std430) readonly buffer ChunkData {
//    uint stateIds[4096];
//};

//layout(set = 2, binding = 1, std430) writeonly buffer RangeStartBuffer {
//    RangeStart rangeStarts[4096];
//};

//layout(set = 2, binding = 2, std430) writeonly buffer IndirectCommandBuffer {
//    DrawCommand commands[4096];
//};

////per chunk data from the compute shader
//layout(set = 2, binding = 3) uniform ChunkPos {
//    vec3 chunkPos;
//};

template <size_t binding = 0>
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

template <size_t binding = 1>
struct RangeStartBufferLayoutDefinition {
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
struct IndirectCommandBufferLayoutDefinition {
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
struct ChunkPosLayoutDefinition {
    static constexpr vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding = {
            binding,                                        // binding
            vk::DescriptorType::eUniformBuffer,				// descriptor type
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