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

#include "Rendering/Shape.h"

class TestMesh
{
private:
	Gfx::Memory m_instancedMemory;
	Gfx::Buffer m_instancedBuffer;

	std::vector<Shape::FaceData> m_meshData;
public:

	void setup(const Gfx::Context& instance, const Gfx::Device& device,
		const Gfx::Queue& transferQueue, Gfx::CommandPool& temporaryPool, Gfx::MappedMemory& stagingMemory,
		Gfx::Buffer& stagingBuffer, const Id::Cache<Shape::Data, Id::Shape>& parallelograms);

	void destroy(const Gfx::Context& instance, const Gfx::Device& device);

	Gfx::Buffer& getBuffer() { return m_instancedBuffer; };

	const Gfx::Buffer& getBuffer() const { return m_instancedBuffer; };

	uint32_t getIndexCount() const { return Shape::indices.size(); };

	uint32_t getInstanceCount() const { return m_meshData.size(); };
};

