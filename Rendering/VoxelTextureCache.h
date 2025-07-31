#pragma once
#include "Graphics/DeviceCaching/PhysicalDeviceCache.h"
#include "Graphics/Rendering/Device.h"
#include "Graphics/Rendering/CommandBuffer.h"
#include "Graphics/Rendering/CommandPool.h"
#include "Graphics/Rendering/Semaphore.h"
#include "Graphics/Rendering/Fence.h"
#include "Graphics/MemoryManagement/Buffer.h"
#include "Graphics/Rendering/Queue.h"
#include "Graphics/MemoryManagement/Memory.h"
#include "Graphics/MemoryManagement/MappedMemory.h"
#include "Graphics/Camera.h"

#include "Common.h"

class VoxelTextureCache
{
private:
	std::vector<Gfx::Memory> m_imageMemories;
	std::vector<Gfx::Image> m_images;

	std::vector<std::string> m_textureNames;
	std::unordered_map<std::string, Id::Texture> m_textureNameToId;

	//static inline const std::array<std::string, 6> texturePaths = {
	//"res\\resourcePack\\textures\\dirt.png",
	//"res\\resourcePack\\textures\\grass_block_side.png",
	//"res\\resourcePack\\textures\\grass_block_top.png",
	//"res\\resourcePack\\textures\\stone.png",
	//"res\\resourcePack\\textures\\water.png",
	//"res\\resourcePack\\textures\\highlight_texture.png"
	//};

public:
	void setup(const Gfx::Context& instance, const Gfx::Device& device,
		const Gfx::Queue& transferQueue, Gfx::CommandPool& temporaryPool, Gfx::MappedMemory& stagingMemory,
		Gfx::Buffer& stagingBuffer, const std::vector<std::string>& filepaths);

	void writeToDescriptor(const Gfx::Context& instance, const Gfx::Device& device,
		Gfx::DescriptorSetHandle& descriptorSet, const Gfx::Sampler& sampler, size_t binding) const;

	std::vector<Gfx::Image>& getImages() { return m_images; };
	const std::vector<Gfx::Image>& getImages() const { return m_images; };

	void destroy(const Gfx::Context& instance, const Gfx::Device& device);

	Id::Texture getId(const std::string& name) const
	{
		auto it = m_textureNameToId.find(name);
		if (it != m_textureNameToId.end())
			return it->second;
		throw std::runtime_error("Texture not found: " + name);
		return Id::Texture(0);
	}

private:

	void populateTextureNames(const std::vector<std::string>& filepaths)
	{
		m_textureNames.reserve(filepaths.size());
		m_textureNameToId.reserve(filepaths.size());

		for (const auto& filepath : filepaths)
		{
			size_t lastSlash = filepath.find_last_of("/\\");
			size_t lastDot = filepath.find_last_of(".");
			if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash)
			{
				std::string textureName = filepath.substr(lastSlash + 1, lastDot - lastSlash - 1);
				m_textureNameToId[textureName] = Id::VoxelTexture(m_textureNames.size());
				m_textureNames.push_back(textureName);				
			}
			else
			{
				throw std::runtime_error("Invalid file path: " + filepath);
			}
		}
	}

};

