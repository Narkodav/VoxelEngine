#pragma once
#include <stdexcept>
#include <cstdint>
#include <array>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <random>
#include <vector>
#include <filesystem>
#include <utility>
#include <shared_mutex>

#include "Graphics/Common.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Gfx = Graphics;

template<typename E>
inline constexpr std::underlying_type_t<E> enumCast(const E& enumObj) {
	return static_cast<std::underlying_type_t<E>>(enumObj);
}

template<typename E, typename T>
inline constexpr E enumCast(const T& obj) {
	return static_cast<E>(obj);
}

template<typename T>
requires std::is_arithmetic_v<T>
inline std::pair<T, T> modDiv(T divident, T divisor) {
	static_assert(false, "This function is not implemented for this type");
}

template<>
inline std::pair<uint64_t, uint64_t> modDiv(uint64_t divident, uint64_t divisor) {
	auto div = std::lldiv(divident, divisor);
	return std::make_pair(div.quot, div.rem);
}

template<>
inline std::pair<int64_t, int64_t> modDiv(int64_t divident, int64_t divisor) {
	auto div = std::lldiv(divident, divisor);
	return std::make_pair(div.quot, div.rem);
}

template<>
inline std::pair<uint32_t, uint32_t> modDiv(uint32_t divident, uint32_t divisor) {
	auto div = std::ldiv(divident, divisor);
	return std::make_pair(div.quot, div.rem);
}

template<>
inline std::pair<int32_t, int32_t> modDiv(int32_t divident, int32_t divisor) {
	auto div = std::ldiv(divident, divisor);
	return std::make_pair(div.quot, div.rem);
}

enum class Directions3D : size_t
{
	FORWARD,
	RIGHT,
	UP,
	BACKWARD,
	LEFT,
	DOWN,
	NUM,
};

using Coordinate3D = glm::vec<3, uint32_t, glm::lowp>;
using CoordinateSigned3D = glm::vec<3, uint32_t, glm::lowp>;
using Position3D = glm::vec<3, float, glm::lowp>;
using Direction3D = glm::ivec3;

enum class Directions2D : size_t
{
	FORWARD,
	RIGHT,
	BACKWARD,
	LEFT,
	NUM,
};

using Coordinate2D = glm::vec<2, uint64_t, glm::lowp>;
using Position2D = glm::vec<2, float, glm::lowp>;
using Direction2D = glm::vec<2, int64_t, glm::lowp>;
using ChunkRelativeCoord3D = glm::vec<3, int64_t, glm::lowp>;

namespace Constants
{
	const std::array<Direction2D, enumCast(Directions2D::NUM)> directions2D = {
		Direction2D{0, -1}, Direction2D{1, 0}, Direction2D{0, 1}, Direction2D{-1, 0},
	};

	const std::array<Direction3D, enumCast(Directions3D::NUM)> directions3D = {
		Direction3D{0, 0, -1}, Direction3D{1, 0, 0}, Direction3D{0, 1, 0},
		Direction3D{0, 0, 1}, Direction3D{-1, 0, 0}, Direction3D{0, -1, 0},
	};

	const std::array<Position3D, enumCast(Directions3D::NUM)> directionsFloat3D = {
		Position3D{0, 0, -1}, Position3D{1, 0, 0}, Position3D{0, 1, 0},
		Position3D{0, 0, 1}, Position3D{-1, 0, 0}, Position3D{0, -1, 0},
	};

	//in voxels
	const size_t chunkWidth = 32;	//along X axis
	const size_t chunkHeight = 32;	//along Y axis
	const size_t chunkDepth = 32;	//along Z axis

	const float gridCellSize = 1.0f;
	const float gridCellCenterOffset = 0.5f;

	const size_t chunkSize = chunkWidth * chunkHeight * chunkDepth;
	const size_t chunkLayerSize = chunkWidth * chunkDepth;

	constexpr Coordinate3D chunkMin{ 0, 0, 0 };
	constexpr Coordinate3D chunkMax{ chunkWidth - 1, chunkHeight - 1, chunkDepth - 1 };

	// chunks positions are represented as unsigned, for performance reasons, 
	// so for the 0,0,0 chunk to be in the middle there is this offset	
	const uint64_t GlobalPositionOffset = std::numeric_limits<uint64_t>::max() / 2;
	const uint64_t ChunkPositionOffsetX = GlobalPositionOffset / chunkWidth;
	const uint64_t ChunkPositionOffsetY = GlobalPositionOffset / chunkHeight;
	const uint64_t ChunkPositionOffsetZ = GlobalPositionOffset / chunkDepth;

	const uint32_t noChunkIndex = std::numeric_limits<uint32_t>::max();
}

static inline constexpr Position3D to3D(Position2D dir) { return Position3D{ dir.x, 0, dir.y }; };
static inline constexpr Position2D to2D(Position3D dir) { return Position2D{ dir.x, dir.z }; };

static inline constexpr Coordinate3D to3D(Coordinate2D dir) { return Coordinate3D{ dir.x, 0, dir.y }; };
static inline constexpr Coordinate2D to2D(Coordinate3D dir) { return Coordinate2D{ dir.x, dir.z }; };

static inline constexpr Directions2D to2D(Directions3D dir) {
	switch (dir) {
	case Directions3D::FORWARD: return Directions2D::FORWARD;
	case Directions3D::RIGHT: return Directions2D::RIGHT;
	case Directions3D::BACKWARD: return Directions2D::BACKWARD;
	case Directions3D::LEFT: return Directions2D::LEFT;
	default: throw std::invalid_argument("Cannot convert UP/DOWN to 2D direction");
	}
}

static inline constexpr Directions3D to3D(Directions2D dir) {
	switch (dir) {
	case Directions2D::FORWARD: return Directions3D::FORWARD;
	case Directions2D::RIGHT: return Directions3D::RIGHT;
	case Directions2D::BACKWARD: return Directions3D::BACKWARD;
	case Directions2D::LEFT: return Directions3D::LEFT;
	default: throw std::invalid_argument("Cannot convert UP/DOWN to 2D direction");
	}
}

static inline constexpr Directions3D reverseDir(Directions3D dir) {
	return enumCast<Directions3D>((enumCast(dir) + enumCast(Directions3D::NUM) / 2) % enumCast(Directions3D::NUM));
}

static inline constexpr Directions2D reverseDir(Directions2D dir) {
	return enumCast<Directions2D>((enumCast(dir) + enumCast(Directions2D::NUM) / 2) % enumCast(Directions2D::NUM));
}

static inline constexpr bool isInChunkBounds(const Coordinate3D& coord) {
	return coord.x >= 0 && coord.x < Constants::chunkWidth &&
		coord.y >= 0 && coord.y < Constants::chunkHeight &&
		coord.z >= 0 && coord.z < Constants::chunkDepth;
}

static inline constexpr Coordinate3D fromChunkToGlobal(const Coordinate3D& chunkCoord, const ChunkRelativeCoord3D& coord) {
	Coordinate3D result;
	result.x = chunkCoord.x * Constants::chunkWidth + coord.x;
	result.y = chunkCoord.y * Constants::chunkHeight + coord.y;
	result.z = chunkCoord.z * Constants::chunkDepth + coord.z;
	return result;
}

static inline constexpr Coordinate3D fromChunkToGlobal(const Coordinate3D& chunkCoord) {
	Coordinate3D result;
	result.x = chunkCoord.x * Constants::chunkWidth;
	result.y = chunkCoord.y * Constants::chunkHeight;
	result.z = chunkCoord.z * Constants::chunkDepth;
	return result;
}

//static inline std::pair<Coordinate3D, Coordinate3D> fromGlobalToChunk(const Coordinate3D& coord) {
//	auto [quotX, remX] = modDiv(coord.x, Constants::chunkWidth);
//	auto [quotY, remY] = modDiv(coord.y, Constants::chunkHeight);
//	auto [quotZ, remZ] = modDiv(coord.z, Constants::chunkDepth);
//	return std::make_pair(Coordinate3D{ quotX, quotY, quotZ }, Coordinate3D{ remX, remY, remZ });
//}

static inline constexpr CoordinateSigned3D convertGlobalToSigned(const Coordinate3D& coord) {
	return CoordinateSigned3D(static_cast<int64_t>(coord.x - Constants::GlobalPositionOffset),
		static_cast<int64_t>(coord.y - Constants::GlobalPositionOffset),
		static_cast<int64_t>(coord.z - Constants::GlobalPositionOffset));
}

static inline constexpr CoordinateSigned3D convertChunkToSigned(const Coordinate3D& coord) {
	return CoordinateSigned3D(static_cast<int64_t>(coord.x - Constants::ChunkPositionOffsetX),
		static_cast<int64_t>(coord.y - Constants::ChunkPositionOffsetY),
		static_cast<int64_t>(coord.z - Constants::ChunkPositionOffsetZ));
}

namespace Id
{
	template<typename Tag, typename IdType = uint32_t>
	struct Id {
	public:
		using DataType = IdType;

	private:
		IdType m_value;

	public:

		constexpr Id(IdType v = 0) : m_value(v) {}
		constexpr operator IdType() const { return m_value; }

		constexpr bool operator==(const Id& other) const { return m_value == other.m_value; }
		constexpr bool operator!=(const Id& other) const { return m_value != other.m_value; }
		constexpr bool operator<(const Id& other) const { return m_value < other.m_value; }
		constexpr bool operator<=(const Id& other) const { return m_value <= other.m_value; }
		constexpr bool operator>(const Id& other) const { return m_value > other.m_value; }
		constexpr bool operator>=(const Id& other) const { return m_value >= other.m_value; }
	};

	template<typename T, typename Id>
	class NamedCache
	{
	private:
		std::vector<T> m_cache;
		std::vector<std::string> m_names;
		std::unordered_map<std::string, Id> m_nameToId;
		mutable std::shared_mutex m_mutex;

	public:
		Id add(T&& object, std::string_view nameView)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			// Try to insert the name->id mapping first
			auto [it, inserted] = m_nameToId.try_emplace(std::move(std::string(nameView)),
				static_cast<Id>(m_cache.size()));

			if (!inserted) {
				return it->second;  // Return existing ID if already cached
			}

			// Only push to vectors if this is a new entry
			m_cache.push_back(std::move(object));
			m_names.push_back(it->first);  // Use the string already in the map
			return it->second;
		}

		inline constexpr const T& get(Id id) const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			assert(static_cast<Id::DataType>(id) < m_cache.size());
			return m_cache[static_cast<Id::DataType>(id)];
		}

		inline T& get(Id id) {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			assert(static_cast<Id::DataType>(id) < m_cache.size());
			return m_cache[static_cast<Id::DataType>(id)];
		}

		inline constexpr const T& operator[](Id id) const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_cache[static_cast<Id::DataType>(id)];
		}

		inline T& operator[](Id id) {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_cache[static_cast<Id::DataType>(id)];
		}

		inline constexpr const T& operator[](const std::string& name) const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			auto it = m_nameToId.find(name);
			if (it == m_nameToId.end()) {
				throw std::runtime_error("Asset not found: " + name);
			}
			return m_cache[it->second];
		}

		inline T& operator[](const std::string& name) {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			auto it = m_nameToId.find(name);
			if (it == m_nameToId.end()) {
				throw std::runtime_error("Asset not found: " + name);
			}
			return m_cache[it->second];
		}

		inline constexpr Id getId(const std::string& name) const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			auto it = m_nameToId.find(name);
			if (it == m_nameToId.end()) {
				throw std::runtime_error("Asset not found: " + name);
			}
			return it->second;
		}

		inline constexpr bool exists(const std::string& name) const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_nameToId.contains(name);
		}

		inline constexpr size_t size() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_cache.size();
		}

		inline constexpr const std::vector<T>& data() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_cache;
		}

		inline void reserve(size_t size) {
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_cache.reserve(size);
			m_names.reserve(size);
			m_nameToId.reserve(size);
		}
	};

	template<typename T, typename Id, typename Comparator = std::equal_to<T>>
	class Cache
	{
	private:
		std::vector<T> m_cache;
		mutable std::shared_mutex m_mutex;
	public:
		Id add(T&& object)
		{
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			auto it = std::find_if(m_cache.begin(), m_cache.end(),
				[&object](const T& cached) {
					return Comparator{}(cached, object);
				});

			if (it != m_cache.end()) {
				return static_cast<Id>(std::distance(m_cache.begin(), it));
			}
			m_cache.push_back(std::move(object));
			return static_cast<Id>(m_cache.size() - 1);
		}

		Id add(const T& object) {
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			auto it = std::find_if(m_cache.begin(), m_cache.end(),
				[&object](const T& cached) {
					return Comparator{}(cached, object);
				});
			if (it != m_cache.end()) {
				return static_cast<Id>(std::distance(m_cache.begin(), it));
			}
			m_cache.push_back(object);
			return static_cast<Id>(m_cache.size() - 1);
		}

		inline constexpr const T& get(Id id) const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			assert(static_cast<Id::DataType>(id) < m_cache.size());
			return m_cache[static_cast<Id::DataType>(id)];
		}

		inline T& get(Id id) {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			assert(static_cast<Id::DataType>(id) < m_cache.size());
			return m_cache[static_cast<Id::DataType>(id)];
		}

		inline constexpr const T& operator[](Id id) const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_cache[static_cast<Id::DataType>(id)];
		}

		inline T& operator[](Id id) {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_cache[static_cast<Id::DataType>(id)];
		}

		inline constexpr size_t size() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_cache.size();
		}

		inline const std::vector<T>& data() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_cache;
		}

		inline void reserve(size_t size) {
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			m_cache.reserve(size);
		}
	};

	template<typename Index, typename Comparator = std::equal_to<Index>>
	class IndexSequenceCache
	{
	public:
		struct EntryTag {};
		using EntryId = Id<EntryTag>;

		struct alignas(4) Entry {
			uint32_t start;
			uint32_t size;

			bool operator==(const Entry& other) const {
				return start == other.start && size == other.size;
			};
		};

		using EntryCache = Cache<Entry, EntryId>;
	private:
		std::vector<Index> m_indices;
		EntryCache m_entries;

		mutable std::shared_mutex m_mutex;

	public:
		EntryId add(const std::vector<Index>& sequence)
		{
			if (sequence.empty()) return m_entries.add(Entry{ 0, 0 });
			std::unique_lock<std::shared_mutex> lock(m_mutex);
			Entry entry;
			entry.size = sequence.size();

			for (size_t i = 0; i < m_indices.size(); ++i)
			{
				if (Comparator()(m_indices[i], sequence[0]))
				{
					size_t j = i + 1;
					size_t k = 1;
					for (; j < m_indices.size() &&
						k < sequence.size() &&
						Comparator()(m_indices[j], sequence[k]);
						++j, ++k);
					if (k == sequence.size())
					{
						entry.start = i;						
						return m_entries.add(entry);
					}
				}
			}

			entry.start = m_indices.size();
			m_indices.insert(m_indices.end(), sequence.begin(), sequence.end());
			return m_entries.add(entry);
		}

		inline constexpr size_t indexSize() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_indices.size();
		}

		inline const auto& indexData() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_indices;
		}

		inline constexpr size_t entrySize() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_entries.size();
		}

		inline const auto& entryCache() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_entries;
		}

		inline const auto& entryData() const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_entries.data();
		}

		inline const auto& operator[](size_t index) const {
			std::shared_lock<std::shared_mutex> lock(m_mutex);
			return m_indices[index];
		}
	};

	struct VertexTag			{};
	struct UvTag				{};
	struct PolygonTag			{};
	struct ColoringTag			{};
	struct TextureTag			{};
	struct ModelTag				{};
	
	struct VoxelTag				{};
	struct VoxelStateTag		{};
	struct VoxelAttributeTag	{};

	using Vertex			= Id<VertexTag>;
	using Uv				= Id<UvTag>;
	using Polygon			= Id<PolygonTag>;
	using Coloring			= Id<ColoringTag>;
	using Texture			= Id<TextureTag>;
	using Model				= Id<ModelTag>;

	using Voxel				= Id<VoxelTag>;
	using VoxelState		= Id<VoxelStateTag>;
	using VoxelAttribute	= Id<VoxelAttributeTag>;
}

namespace Constants
{
	const Id::VoxelState emptyStateId = Id::VoxelState(0); //id for an empty block state (air)
	const Id::Model emptyModelId = Id::Model(std::numeric_limits<uint32_t>::max()); //id for an empty model, isnt actually used just a placeholder
}

static inline std::vector<std::string> getFilesInDirectory(const std::string& directoryPath) {
	std::vector<std::string> files;

	for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
		if (entry.is_regular_file()) {  // Only get regular files, not directories
			files.push_back(entry.path().string());
		}
	}
	return files;
}

static inline std::string getFileName(const std::string& filePath) {
	std::filesystem::path path(filePath);
	return path.stem().string();
}

template <typename T>
struct VecEpsilonEqualComparator {
	bool operator()(const T& a, const T& b) const {
		return glm::all(glm::epsilonEqual(a, b, std::numeric_limits<float>::epsilon()));
	}
};

namespace std {
	template<>
	struct hash<glm::ivec3> {
		size_t operator()(const glm::ivec3& v) const noexcept {
			size_t seed = 0;
			seed ^= std::hash<int>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= std::hash<int>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= std::hash<int>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	};
}