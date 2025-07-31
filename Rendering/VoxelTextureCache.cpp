#include "VoxelTextureCache.h"

void VoxelTextureCache::setup(const Gfx::Context& instance, const Gfx::Device& device,
    const Gfx::Queue& transferQueue, Gfx::CommandPool& temporaryPool, Gfx::MappedMemory& stagingMemory,
    Gfx::Buffer& stagingBuffer, const std::vector<std::string>& filepaths)
{
    populateTextureNames(filepaths);

    size_t totalCapacity = 0;
    m_images.resize(filepaths.size());
    m_imageMemories.resize(filepaths.size());

    for (size_t i = 0; i < filepaths.size(); i++)
    {
        m_images[i] = Gfx::Image(instance, device, filepaths[i]);
        m_imageMemories[i] = Gfx::Memory(instance, device, m_images[i].getMemoryRequirements(),
            Gfx::MemoryProperty::Bits::DeviceLocal, m_images[i].getMemoryRequirements().size);
        m_imageMemories[i].bindImage(instance, device, m_images[i]);
        m_images[i].createView(instance, device);
    }

    for (size_t i = 0; i < m_images.size(); i++)
    {
        temporaryPool.makeOneTimeSubmit(instance, device, transferQueue,
            [this, &i, &instance, &stagingMemory, &stagingBuffer](Gfx::CommandBufferHandle temporary) {
                temporary->setPipelineBarrier(instance,
                    vk::PipelineStageFlagBits::eTopOfPipe,
                    vk::PipelineStageFlagBits::eTransfer,
                    m_images[i],
                    vk::ImageLayout::eTransferDstOptimal,
                    vk::AccessFlagBits::eNone,
                    vk::AccessFlagBits::eTransferWrite
                    );

                auto pixelData = m_images[i].getPixelData();
                auto mappedMemory = stagingMemory.getMapping<uint8_t>(pixelData.size());
                std::copy(pixelData.begin(), pixelData.end(), mappedMemory.begin());

                temporary->transferImageData(instance, stagingBuffer, m_images[i]);

                temporary->setPipelineBarrier(instance,
                    vk::PipelineStageFlagBits::eTransfer,
                    vk::PipelineStageFlagBits::eFragmentShader,
                    m_images[i],
                    vk::ImageLayout::eShaderReadOnlyOptimal,
                    vk::AccessFlagBits::eTransferWrite,
                    vk::AccessFlagBits::eShaderRead
                );
            });
    }
}

void VoxelTextureCache::writeToDescriptor(const Gfx::Context& instance, const Gfx::Device& device,
    Gfx::DescriptorSetHandle& descriptorSet, const Gfx::Sampler& sampler, size_t binding) const
{
    std::vector<const Gfx::Sampler*> samplerHandles(m_images.size(), &sampler);
    descriptorSet->write(instance, device,
        m_images, samplerHandles, binding);
}

void VoxelTextureCache::destroy(const Gfx::Context& instance, const Gfx::Device& device)
{
    for (auto& image : m_images)
        image.destroy(instance, device);
    for (auto& memory : m_imageMemories)
        memory.destroy(instance, device);
}