#include "VoxelMeshing.h"

void VoxelMeshing::render(const VoxelVolume& volume, AdjacentVolumes& adjacentVolumes,
	const Coordinate3D& chunkPosition, const ResourceCache& cache)
{
	BufferData data;

	for (Coordinate3D coord = Coordinate3D(0); coord.y < volume.height(); coord.y++)
		for (coord.x = 0; coord.x < volume.width(); coord.x++)
			for (coord.z = 0; coord.z < volume.depth(); coord.z++)
			{
				if (volume[coord] != Constants::airId)
					addBlock(volume, adjacentVolumes, chunkPosition, coord, cache, data);
			}
	updateVertexBuffers(std::move(data));
}

void VoxelMeshing::addBlock(const VoxelVolume& volume, AdjacentVolumes& adjacentVolumes,
	const Coordinate3D& chunkPosition, const Coordinate3D& coord,
	const ResourceCache& cache, BufferData& data)
{
	const auto& voxelCullings = cache.getAssetCache().getVoxelCullingCache();

	addSide<Shape::Side::FRONT	>(volume, adjacentVolumes, chunkPosition, coord, cache, voxelCullings, data);
	addSide<Shape::Side::RIGHT	>(volume, adjacentVolumes, chunkPosition, coord, cache, voxelCullings, data);
	addSide<Shape::Side::TOP	>(volume, adjacentVolumes, chunkPosition, coord, cache, voxelCullings, data);
	addSide<Shape::Side::BACK	>(volume, adjacentVolumes, chunkPosition, coord, cache, voxelCullings, data);
	addSide<Shape::Side::LEFT	>(volume, adjacentVolumes, chunkPosition, coord, cache, voxelCullings, data);
	addSide<Shape::Side::BOTTOM	>(volume, adjacentVolumes, chunkPosition, coord, cache, voxelCullings, data);
}

void VoxelMeshing::updateVertexBuffers(BufferData&& data)
{
	if (m_vao)
		clear();
	// Generate VAO and buffers
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(m_buffers.size(), m_buffers.data());

	glBindVertexArray(m_vao);

	// Instance position buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[enumCast(Buffers::BUFFER_POSITION)]);
	glBufferData(GL_ARRAY_BUFFER, data.positions.size() * sizeof(glm::vec3), data.positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(enumCast(Locations::BUFFER_POSITION),
		3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(enumCast(Locations::BUFFER_POSITION));
	glVertexAttribDivisor(enumCast(Locations::BUFFER_POSITION), 1); // Once per instance

	// Shape ID buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[enumCast(Buffers::BUFFER_PARALLELOGRAM_ID)]);
	glBufferData(GL_ARRAY_BUFFER, data.parallelogramIDs.size() * sizeof(unsigned int), data.parallelogramIDs.data(), GL_STATIC_DRAW);
	glVertexAttribIPointer(enumCast(Locations::BUFFER_PARALLELOGRAM_ID),
		1, GL_UNSIGNED_INT, 0, nullptr);
	glEnableVertexAttribArray(enumCast(Locations::BUFFER_PARALLELOGRAM_ID));
	glVertexAttribDivisor(enumCast(Locations::BUFFER_PARALLELOGRAM_ID), 1);

	// Face buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[enumCast(Buffers::BUFFER_FACE)]);
	glBufferData(GL_ARRAY_BUFFER, data.faces.size() * sizeof(uint8_t), data.faces.data(), GL_STATIC_DRAW);
	glVertexAttribIPointer(enumCast(Locations::BUFFER_FACE),
		1, GL_UNSIGNED_BYTE, 0, nullptr);
	glEnableVertexAttribArray(enumCast(Locations::BUFFER_FACE));
	glVertexAttribDivisor(enumCast(Locations::BUFFER_FACE), 1);

	// Cleanup
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	m_instanceAmount = data.faces.size();
}

void VoxelMeshing::clear()
{
	glDeleteBuffers(m_buffers.size(), m_buffers.data());
	glDeleteVertexArrays(1, &m_vao);
}

void VoxelMeshing::draw(const Coordinate3D& chunkCoord) const
{
	auto signedCoord = convertChunkToSigned(chunkCoord);
	ShaderCache::getShader(ShaderCache::ShaderType::VOXEL).setUniform3f("u_chunkPosition", chunkCoord.x, chunkCoord.y, chunkCoord.z);
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Shape::m_buffers[enumCast(Shape::Buffers::BUFFER_INDEX)]);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, m_instanceAmount);
}