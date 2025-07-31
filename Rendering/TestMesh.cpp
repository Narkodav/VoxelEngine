#include "TestMesh.h"

void TestMesh::setup(const Gfx::Context& instance, const Gfx::Device& device,
	const Gfx::Queue& transferQueue, Gfx::CommandPool& temporaryPool, Gfx::MappedMemory& stagingMemory,
    Gfx::Buffer& stagingBuffer, const Id::Cache<Shape::Data, Id::Shape>& parallelograms)
{
    //makes a grid of all parallelograms
    uint32_t gridWidth = sqrt(parallelograms.size());
    //size_t gridHeight = parallelograms.size() / gridWidth;
    
    for(uint32_t i = 0; i < parallelograms.size(); ++i)
    {
        m_meshData.push_back(Shape::FaceData(
            i,
            0,
            { 2 * i % gridWidth, 0, 2 * i / gridWidth, 1 }));
        m_meshData.push_back(Shape::FaceData(
            i,
            1,
            { 2 * i % gridWidth, 0, 2 * i / gridWidth, 1 }));
        m_meshData.push_back(Shape::FaceData(
            i,
            2,
            { 2 * i % gridWidth, 0, 2 * i / gridWidth, 1 }));
        m_meshData.push_back(Shape::FaceData(
            i,
            3,
            { 2 * i % gridWidth, 0, 2 * i / gridWidth, 1 }));
        m_meshData.push_back(Shape::FaceData(
            i,
            4,
            { 2 * i % gridWidth, 0, 2 * i / gridWidth, 1 }));
        m_meshData.push_back(Shape::FaceData(
            i,
            5,
            { 2 * i % gridWidth, 0, 2 * i / gridWidth, 1 }));
    }

    m_instancedBuffer = Gfx::Buffer(instance, device,
        Gfx::Buffer::Descriptor(Shape::FaceDataInstanced<1, 1, 2, 3>::DATA_SIZE * m_meshData.size(),
            Gfx::BufferUsage::Bits::TransferDst | Gfx::BufferUsage::Bits::Vertex, false));

    m_instancedMemory = Gfx::Memory(instance, device, m_instancedBuffer.getMemoryRequirements(),
        Gfx::MemoryProperty::Bits::DeviceLocal,
        Gfx::VertexDefinitionModelTransform::DATA_SIZE * m_meshData.size());
    
    m_instancedMemory.bindBuffer(instance, device, m_instancedBuffer);

    temporaryPool.makeOneTimeDataTransfer<Shape::FaceData>(instance, device, transferQueue,
        stagingMemory, stagingBuffer, m_instancedBuffer, m_meshData);
}

void TestMesh::destroy(const Gfx::Context& instance, const Gfx::Device& device)
{
    m_instancedBuffer.destroy(instance, device);
    m_instancedMemory.destroy(instance, device);
}