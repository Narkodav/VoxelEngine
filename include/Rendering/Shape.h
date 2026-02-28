#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Common.h"

#include <array>

struct Shape
{
public:
	struct FaceData
	{
		uint32_t data1;
		uint32_t data2; //might be used for metadata, is here to force 16 byte allignment
		uint32_t parallelogramId;
		uint32_t side; //higher bits are reserved
		glm::vec4 position;

		FaceData() = default;

		FaceData(uint32_t parallelogramId, uint32_t side, glm::vec4 position)
			: parallelogramId(parallelogramId), side(side), position(position)
		{
		};
	};

	template<size_t binding, size_t location>
	struct PositionVertex : public Gfx::Utility::VertexDefinitionBase<PositionVertex<binding, location>> {
	public:
		using Type = glm::vec4;

		static constexpr size_t s_bindingCount = 1;
		static constexpr size_t s_attributeCount = 1;
		static constexpr size_t s_dataSize = sizeof(Type);

		static constexpr Gfx::VertexInputBindingDescription s_bindings[] = {
			{
				binding,                                // binding
				s_dataSize,                             // stride
				Gfx::VertexInputRate::Vertex            // input rate
			}
		};

		static constexpr Gfx::VertexInputAttributeDescription s_attributes[] = {
			{
				location,                               // location
				binding,                                // binding
				Gfx::PixelFormat::R32G32B32A32Sfloat,   // format
				0                                       // offset
			}
		};
	};

	//layout(location = 1) in uint inShapeId;
	//layout(location = 2) in uint inCubeSide;
	//layout(location = 3) in vec4 inCubePosition;

	template<size_t binding, size_t parallelogramIdLocation,
	size_t cubeSideLocation, size_t cubePositionLocation>
	struct FaceDataInstanced : public Gfx::Utility::VertexDefinitionBase<FaceDataInstanced<
	binding, parallelogramIdLocation, cubeSideLocation, cubePositionLocation>> {
	public:
		using Type = FaceData;

		static constexpr size_t s_bindingCount = 1;
		static constexpr size_t s_attributeCount = 3;
		static constexpr size_t s_dataSize = sizeof(Type);
		
		static constexpr Gfx::VertexInputBindingDescription bindings[] = {
			{
				binding,                                // binding
				s_dataSize,                             // stride
				Gfx::VertexInputRate::Instance          // input rate
			}
		};

		static constexpr Gfx::VertexInputAttributeDescription attributes[] = {
			{
				parallelogramIdLocation,                // location
				binding,                                // binding
				Gfx::PixelFormat::R32Uint,				// format
				sizeof(uint32_t) * 2                    // offset
			},
			{
				cubeSideLocation,						// location
				binding,                                // binding
				Gfx::PixelFormat::R32Uint,				// format
				sizeof(uint32_t) * 3					// offset
			},
			{
				cubePositionLocation,                   // location
				binding,                                // binding
				Gfx::PixelFormat::R32G32B32A32Sfloat,	// format
				sizeof(uint32_t) * 4					// offset
			},
		};
	};

	enum class Side : uint8_t
	{
		Front	= enumCast(Directions3D::FORWARD),
		Right	= enumCast(Directions3D::RIGHT),
		Top		= enumCast(Directions3D::UP),
		Back	= enumCast(Directions3D::BACKWARD),
		Left	= enumCast(Directions3D::LEFT),
		Bottom	= enumCast(Directions3D::DOWN),
		Count,
	};

	static inline const std::array<std::string, enumCast(Side::Count)> s_sideFaceNames = {
		"front",
		"right",
		"top",
		"back",
		"left",
		"bottom"
	};

	enum class FaceCorner : uint8_t
	{
		BottomLeft,
		BottomRight,
		TopRight,
		TopLeft,
		Count
	};

	using UVArray = std::array<glm::vec2, enumCast(FaceCorner::Count)>;

	static inline const UVArray defaultUvs = {
		glm::vec2(0, 1),
		glm::vec2(1, 1),
		glm::vec2(1, 0),
		glm::vec2(0, 0)
		//glm::vec2(1, 1),
		//glm::vec2(1, 0),
		//glm::vec2(0, 1),
		//glm::vec2(0, 0)
	};

	static_assert(enumCast(Side::Front)		== enumCast(Directions3D::FORWARD),		"Side and Directions3D FRONT do not match");
	static_assert(enumCast(Side::Right)		== enumCast(Directions3D::RIGHT),		"Side and Directions3D RIGHT do not match");
	static_assert(enumCast(Side::Top)		== enumCast(Directions3D::UP),			"Side and Directions3D TOP do not match");
	static_assert(enumCast(Side::Back)		== enumCast(Directions3D::BACKWARD),	"Side and Directions3D BACK do not match");
	static_assert(enumCast(Side::Left)		== enumCast(Directions3D::LEFT),		"Side and Directions3D LEFT do not match");
	static_assert(enumCast(Side::Bottom)	== enumCast(Directions3D::DOWN),		"Side and Directions3D BOTTOM do not match");

	//specifies positions for the front face
	static inline const std::array<glm::vec4, enumCast(FaceCorner::Count)> s_frontFacePositions = {
		// Bottom-right
		glm::vec4(
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].x * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::RIGHT)].x * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::DOWN)].x * 0.5f,
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].y * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::RIGHT)].y * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::DOWN)].y * 0.5f,
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].z * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::RIGHT)].z * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::DOWN)].z * 0.5f,
		1),
		// Bottom-left
		glm::vec4(
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].x * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::LEFT)].x * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::DOWN)].x * 0.5f,
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].y * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::LEFT)].y * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::DOWN)].y * 0.5f,
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].z * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::LEFT)].z * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::DOWN)].z * 0.5f,
		1),
		// Top-left
		glm::vec4(
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].x * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::LEFT)].x * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::UP)].x * 0.5f,
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].y * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::LEFT)].y * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::UP)].y * 0.5f,
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].z * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::LEFT)].z * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::UP)].z * 0.5f,
		1),
		// Top-right
		glm::vec4(
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].x * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::RIGHT)].x * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::UP)].x * 0.5f,
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].y * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::RIGHT)].y * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::UP)].y * 0.5f,
		Constants::directionsFloat3D[enumCast(Directions3D::FORWARD)].z * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::RIGHT)].z * 0.5f +
		Constants::directionsFloat3D[enumCast(Directions3D::UP)].z * 0.5f,
		1)
	};

	static inline const std::array<std::array<glm::vec4, 6>, enumCast(Side::Count)> s_cubeVertices = []{
		std::array<std::array<glm::vec4, 6>, enumCast(Side::Count)> out{};

		// Map face -> normal, right, up
		const std::array<std::tuple<Directions3D, Directions3D, Directions3D>, 6> faceAxes = {
			std::make_tuple(Directions3D::FORWARD,  Directions3D::RIGHT,  Directions3D::DOWN),
			std::make_tuple(Directions3D::RIGHT,    Directions3D::BACKWARD, Directions3D::DOWN),
			std::make_tuple(Directions3D::UP,       Directions3D::RIGHT,  Directions3D::FORWARD),
			std::make_tuple(Directions3D::BACKWARD, Directions3D::LEFT,   Directions3D::DOWN),
			std::make_tuple(Directions3D::LEFT,     Directions3D::FORWARD, Directions3D::DOWN),
			std::make_tuple(Directions3D::DOWN,     Directions3D::RIGHT,  Directions3D::BACKWARD)
		};

		for (size_t face = 0; face < faceAxes.size(); ++face) {
			auto [normalDir, rightDir, upDir] = faceAxes[face];

			const glm::vec3 n = Constants::directionsFloat3D[enumCast(normalDir)];
			const glm::vec3 r = Constants::directionsFloat3D[enumCast(rightDir)];
			const glm::vec3 u = Constants::directionsFloat3D[enumCast(upDir)];

			// Each face: two triangles
			out[face][0] = glm::vec4(+0.5f*r + 0.5f*u + 0.5f*n, 1.0f); // bottom-right
			out[face][1] = glm::vec4(-0.5f*r + 0.5f*u + 0.5f*n, 1.0f); // bottom-left
			out[face][2] = glm::vec4(-0.5f*r - 0.5f*u + 0.5f*n, 1.0f); // top-left

			out[face][3] = glm::vec4(+0.5f*r + 0.5f*u + 0.5f*n, 1.0f); // bottom-right
			out[face][4] = glm::vec4(-0.5f*r - 0.5f*u + 0.5f*n, 1.0f); // top-left
			out[face][5] = glm::vec4(+0.5f*r - 0.5f*u + 0.5f*n, 1.0f); // top-right
		}

		return out;
	}();

	static inline const glm::vec3 s_defaultDimensions = glm::vec3(1, 1, 1);
	static inline const glm::vec3 s_defaultPosition = glm::vec3(Constants::gridCellCenterOffset, 
		Constants::gridCellCenterOffset, Constants::gridCellCenterOffset);
	static inline const glm::vec3 s_defaultRotations = glm::vec3(0, 0, 0);

	static inline const std::array<glm::mat4, enumCast(Side::Count)> s_conversionMatrices = {
		// Front face - no rotation needed (identity matrix)
		glm::mat4(1.0f),
		// Right face - 90 degrees around Y axis
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		// Top face - -90 degrees around X axis
		glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
		// Back face - 180 degrees around Y axis
		glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		// Left face - -90 degrees around Y axis
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		// Bottom face - 90 degrees around X axis
		glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f))
	};

	static inline const std::array<uint32_t, 6> indices = {
		0, 1, 2,
		2, 3, 0
	};

	//shape geometry
	struct alignas(4) Polygon
	{
		Id::Vertex position[3]; //polygon
		Id::Normal normal;

		bool operator==(const Polygon& other) const {
			return position[0] == other.position[0] &&
				position[1] == other.position[1] &&
				position[2] == other.position[2];
		};
	};

	//face coloring
	struct alignas(4) Coloring
	{
		Id::Uv uvs[3];
		Id::Texture textureId;

		bool operator==(const Coloring& other) const {
			return textureId == other.textureId &&
				uvs[0] == other.uvs[0] &&
				uvs[1] == other.uvs[1] &&
				uvs[2] == other.uvs[2];
		};
	};

	enum class GeometryType : uint32_t {
		Generic,
		ParallelogramArray,
		Parallelogram,
		Cube,
	};

	struct GeometryMetadata {
		GeometryType geometryType;

		bool operator==(const GeometryMetadata& other) const {
			return geometryType == other.geometryType;
		};
	};

	using ColoringIndexBuffer = Id::IndexSequenceCache<Id::Coloring>;
	using AppearanceId = ColoringIndexBuffer::EntryId;
	using PolygonIndexBuffer = Id::IndexSequenceCache<Id::Polygon, std::equal_to<Id::Polygon>, GeometryMetadata>;
	using GeometryId = PolygonIndexBuffer::EntryId;

	//shape data
	struct alignas(4) Model
	{
		//Id::Geometry geometry[enumCast(Side::Count)];
		//Id::Appearence appearence[enumCast(Side::Count)];

		//bool operator==(const Model& other) const {
		//	for (size_t i = 0; i < enumCast(Side::Count); i++)
		//		if (geometry[i] != other.geometry[i]
		//			|| appearence[i] != other.appearence[i]) return false;
		//};

		Shape::GeometryId geometry;
		Shape::AppearanceId appearence;

		bool operator==(const Model& other) const {
			return geometry == other.geometry && appearence == other.appearence;
		};
	};

	using PolygonCache = Id::Cache<Shape::Polygon, Id::Polygon>;
	using ColoringCache = Id::Cache<Shape::Coloring, Id::Coloring>;

	using VertexCache = Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>;
	using UvCache = Id::Cache<glm::vec2, Id::Uv, VecEpsilonEqualComparator<glm::vec2>>;

	static void registerParallelogram(
		Id::Texture textureIds[6],
		glm::vec2 uvs[6][4],
		glm::vec3 dimension,
		// glm::vec3 position,
		// glm::vec3 rotation,
		Id::Cache<glm::vec4, Id::Vertex, VecEpsilonEqualComparator<glm::vec4>>& vertexCache,
		Id::Cache<glm::vec2, Id::Uv, VecEpsilonEqualComparator<glm::vec2>>& uvCache,
		Id::Cache<glm::vec4, Id::Normal, VecEpsilonEqualComparator<glm::vec4>>& normalCache,
		Id::Cache<Shape::Polygon, Id::Polygon>& polygonCache,
		Id::Cache<Shape::Coloring, Id::Coloring>& coloringCache,
		std::vector<Id::Polygon>& geometry,
		std::vector<Id::Coloring>& appearence) {
		std::vector<Id::Vertex> verticesInd;
		std::vector<Id::Uv> uvsInd;
		Polygon polygon;
		Coloring coloring;

		for (size_t i = 0; i < 6; i++) {
			for (size_t j = 0; j < 6; j++)
			{
				verticesInd.push_back(vertexCache.add((s_conversionMatrices[i] * glm::vec4(
					s_frontFacePositions[indices[j]].x,
					s_frontFacePositions[indices[j]].y,
					s_frontFacePositions[indices[j]].z,
					1)) * glm::vec4(dimension, 1)));
				uvsInd.push_back(uvCache.add(uvs[i][indices[j]]));
			}
			polygon.position[0] = verticesInd[verticesInd.size() - 6];
			polygon.position[1] = verticesInd[verticesInd.size() - 5];
			polygon.position[2] = verticesInd[verticesInd.size() - 4];
			polygon.normal = normalCache.add(glm::vec4(
				Constants::directionsFloat3D[enumCast(static_cast<Directions3D>(i))], 0));

			geometry.push_back(polygonCache.add(polygon));

			polygon.position[0] = verticesInd[verticesInd.size() - 3];
			polygon.position[1] = verticesInd[verticesInd.size() - 2];
			polygon.position[2] = verticesInd[verticesInd.size() - 1];
			polygon.normal = normalCache.add(glm::vec4(
				Constants::directionsFloat3D[enumCast(static_cast<Directions3D>(i))], 0));

			geometry.push_back(polygonCache.add(polygon));

			coloring.uvs[0] = uvsInd[uvsInd.size() - 6];
			coloring.uvs[1] = uvsInd[uvsInd.size() - 5];
			coloring.uvs[2] = uvsInd[uvsInd.size() - 4];
			coloring.textureId = textureIds[i];

			appearence.push_back(coloringCache.add(coloring));

			coloring.uvs[0] = uvsInd[uvsInd.size() - 3];
			coloring.uvs[1] = uvsInd[uvsInd.size() - 2];
			coloring.uvs[2] = uvsInd[uvsInd.size() - 1];
			coloring.textureId = textureIds[i];

			appearence.push_back(coloringCache.add(coloring));
		}
	}

	static inline Gfx::Buffer indexBuffer;
	static inline Gfx::Buffer positionBuffer;

	static inline Gfx::Memory indexMemory;
	static inline Gfx::Memory positionMemory;

	static inline const Gfx::Buffer& getIndexBuffer() { return indexBuffer; };
	static inline const Gfx::Buffer& getPositionBuffer() { return positionBuffer; };
		
	//rotates scales and transposes a front face vertex
	static void transformFaceVertex(glm::vec4& vertex, glm::vec3 dimentions, glm::vec3 position, Side side)
	{
		vertex = Shape::s_conversionMatrices[enumCast(side)] * vertex;
		vertex.x = vertex.x * dimentions.x + position.x;
		vertex.y = vertex.y * dimentions.y + position.y;
		vertex.z = vertex.z * dimentions.z + position.z;
		vertex.w = 1;
	}

	//checks if face2 is contained in face1
	static bool isFaceContained(const std::array<glm::vec4, 4>& face1, const std::array<glm::vec4, 4>& face2) {
		// First, find the normal of the faces (they should be the same since faces are parallel)
		glm::vec3 normal = glm::normalize(glm::cross(
			glm::vec3(face1[1] - face1[0]),
			glm::vec3(face1[2] - face1[0])
		));

		// Create a coordinate system in the plane of the faces
		glm::vec3 u = glm::normalize(face1[1] - face1[0]);
		glm::vec3 v = glm::vec4(glm::cross(normal, u), 1);

		// Project all points onto the UV plane
		auto projectToUV = [&](const glm::vec3& point) -> glm::vec2 {
			return glm::vec2(
				glm::dot(point, u),
				glm::dot(point, v)
			);
			};

		// Get 2D bounds of both faces
		glm::vec2 min1(std::numeric_limits<float>::max());
		glm::vec2 max1(-std::numeric_limits<float>::max());
		glm::vec2 min2(std::numeric_limits<float>::max());
		glm::vec2 max2(-std::numeric_limits<float>::max());

		// Project and find bounds for face1
		for (const auto& vertex : face1) {
			glm::vec2 projected = projectToUV(vertex);
			min1 = glm::min(min1, projected);
			max1 = glm::max(max1, projected);
		}

		// Project and find bounds for face2
		for (const auto& vertex : face2) {
			glm::vec2 projected = projectToUV(vertex);
			min2 = glm::min(min2, projected);
			max2 = glm::max(max2, projected);
		}

		// Check if face2 is contained within face1
		return min1.x <= min2.x && max1.x >= max2.x &&
			min1.y <= min2.y && max1.y >= max2.y;
	}
};