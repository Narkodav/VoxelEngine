#pragma once
#include <array>
#include <span>
#include <type_traits>

#include "Common.h"

//A volume of voxel state ids without any metadata
class VoxelVolume
{
public:
	using Volume = std::array<Id::VoxelState, Constants::chunkSize>;

	//template<bool isConst, size_t layerSize = 1, size_t... subsequentSizes>
	//class VoxelVolumeProxy
	//{
	//	friend class VoxelVolume;		
	//private:
	//	using VolumeRef = std::conditional_t<isConst, const Volume&, Volume&>;
	//	VolumeRef m_volumeRef;
	//	size_t m_offset;
	//	constexpr VoxelVolumeProxy(VolumeRef volume, size_t offset) :
	//		m_volumeRef(volume), m_offset(offset) {
	//	};

	//	VoxelVolumeProxy(const VoxelVolumeProxy&) = delete;
	//	VoxelVolumeProxy& operator=(const VoxelVolumeProxy&) = delete;
	//	VoxelVolumeProxy(VoxelVolumeProxy&&) = delete;
	//	VoxelVolumeProxy& operator=(VoxelVolumeProxy&&) = delete;

	//public:
	//	constexpr std::enable_if_t<isConst == false,
	//	std::conditional_t<layerSize == 1, Id::Voxel&, VoxelVolumeProxy<false, subsequentSizes...>>>
	//		operator[](size_t i)
	//	{
	//		if constexpr (layerSize == 1)
	//			return m_volumeRef[m_offset + i];
	//		else return VoxelVolumeProxy<false, subsequentSizes...>(m_volumeRef, m_offset + i * layerSize);
	//	}

	//	constexpr std::enable_if_t<isConst == true,
	//		std::conditional_t<layerSize == 1, const Id::Voxel&, VoxelVolumeProxy<true, subsequentSizes...>>>
	//		operator[](size_t i) const
	//	{
	//		if constexpr (layerSize == 1)
	//			return m_volumeRef[m_offset + i];
	//		else return VoxelVolumeProxy<true, subsequentSizes...>(m_volumeRef, m_offset + i * layerSize);
	//	}
	//};

	//using ProxyType = VoxelVolumeProxy<false, Constants::chunkLayerSize>;
	//using ConstProxyType = VoxelVolumeProxy<true, Constants::chunkLayerSize>;

private:
	Volume volume = { Constants::emptyStateId };

public:
	VoxelVolume() = default;

	constexpr Id::VoxelState& operator()(size_t x, size_t y, size_t z)
	{
		return volume[y * Constants::chunkLayerSize + x * Constants::chunkDepth + z];
	}

	constexpr const Id::VoxelState& operator()(size_t x, size_t y, size_t z) const
	{
		return volume[y * Constants::chunkLayerSize + x * Constants::chunkDepth + z];
	}

	constexpr Id::VoxelState& operator[](const Coordinate3D& coord)
	{
		return volume[coord.y * Constants::chunkLayerSize + coord.x * Constants::chunkDepth + coord.z];
	}

	constexpr const Id::VoxelState& operator[](const Coordinate3D& coord) const
	{
		return volume[coord.y * Constants::chunkLayerSize + coord.x * Constants::chunkDepth + coord.z];
	}

	constexpr Id::VoxelState& operator[](size_t index)
	{
		return volume[index];
	}

	constexpr const Id::VoxelState& operator[](size_t index) const
	{
		return volume[index];
	}

	////might introduce insignificant overhead but it looks prettier
	//constexpr ProxyType operator[](size_t x)
	//{
	//	return ProxyType(volume, x * Constants::chunkDepth);
	//}

	//constexpr ConstProxyType operator[](size_t x) const
	//{
	//	return ConstProxyType(volume, x * Constants::chunkDepth);
	//}

	static constexpr size_t width()
	{
		return Constants::chunkWidth;
	}

	static constexpr size_t height()
	{
		return Constants::chunkWidth;
	}

	static constexpr size_t depth()
	{
		return Constants::chunkWidth;
	}

	template <typename Callback>
	void forEach(Callback&& callback)
	{
		for (size_t i = 0; i < volume.size(); i++)
			callback(volume[i]);
	}

	template <typename Callback>
	void forEachWithCoord(Callback callback)
	{
		for (Coordinate3D coord = Coordinate3D(0); coord.y < Constants::chunkHeight; coord.y++)
			for (; coord.x < Constants::chunkWidth; coord.x++)
				for (; coord.z < Constants::chunkDepth; coord.z++)
					callback(volume[coord.y * Constants::chunkLayerSize + coord.x * Constants::chunkDepth + coord.z], coord);
	}

	const Volume& data() const { return volume; };
};

