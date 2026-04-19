#include "WorldManagement/WorldGrid.h"

void WorldGrid::generate(const GeneratorSettings& settings) {
	// auto& generatorSettings = config.asObject().at("Generator").asObject();
	// if(generatorSettings.at("Type") == "Cube") {
	// 	auto edge = generatorSettings.at("Edge").asInteger();
	// 	glm::ivec3 cornerPos = Json::getVector<glm::ivec3>(generatorSettings.at("CornerPosition"));
	// 	generateCube(edge, cornerPos);
	// }
	// else if(generatorSettings.at("Type") == "Parallelepiped") {
	// 	auto width = generatorSettings.at("Width").asInteger();
	// 	auto height = generatorSettings.at("Height").asInteger();
	// 	auto depth = generatorSettings.at("Depth").asInteger();
	// 	glm::ivec3 cornerPos = Json::getVector<glm::ivec3>(generatorSettings.at("CornerPosition"));
	// 	generateParallelogram(width, height, depth, cornerPos);
	// }
	// else if(generatorSettings.at("Type") == "Cube") {
	// 	auto edge = generatorSettings.at("Edge").asInteger();
	// 	glm::ivec3 cornerPos = Json::getVector<glm::ivec3>(generatorSettings.at("CornerPosition"));
	// 	generateCube(edge, cornerPos);
	// }
	// else if(generatorSettings.at("Type") == "Sphere") {
	// 	auto radius = generatorSettings.at("Radius").asInteger();
	// 	glm::ivec3 centerPos = Json::getVector<glm::ivec3>(generatorSettings.at("CenterPosition"));
	// 	generateSphere(radius, centerPos);
	// }
	// else if(generatorSettings.at("Type") == "Cylinder") {
	// 	auto radius = generatorSettings.at("Radius").asInteger();
	// 	auto height = generatorSettings.at("Height").asInteger();
	// 	glm::ivec3 bottomCenterPostition = Json::getVector<glm::ivec3>(generatorSettings.at("BottomCenterPosition"));
	// 	generateCylinder(radius, height, bottomCenterPostition);
	// }
	// else throw std::runtime_error("Shape not implemented");

	switch (settings.shape) {
		case ShapeToGenerate::Sphere:
			generateSphere(*static_cast<const SphereGeneratorParams*>(settings.params.get()));
			break;
		case ShapeToGenerate::Cylinder:
			generateCylinder(*static_cast<const CylinderGeneratorParams*>(settings.params.get()));
			break;
		case ShapeToGenerate::Parallelepiped:
			generateParallelepiped(*static_cast<const ParallelepipedGeneratorParams*>(settings.params.get()));
			break;
		case ShapeToGenerate::Cube:
			generateCube(*static_cast<const CubeGeneratorParams*>(settings.params.get()));
			break;
		default:
			throw std::runtime_error("Shape not implemented");
	}
}

void WorldGrid::generateSphere(const SphereGeneratorParams& params)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool((params.radius * 2 + 1) * (params.radius * 2 + 1) * (params.radius * 2 + 1) * 2);

	glm::ivec3 pos;

	for (pos.x = -static_cast<int32_t>(params.radius);
		pos.x < static_cast<int32_t>(params.radius + 1); ++pos.x)
	{
		for (pos.z = -static_cast<int32_t>(params.radius);
			pos.z < static_cast<int32_t>(params.radius + 1); ++pos.z)
		{
			for (pos.y = -static_cast<int32_t>(params.radius);
				pos.y < static_cast<int32_t>(params.radius + 1); ++pos.y)
			{
				glm::ivec3 posRel = pos + params.centerPosition;
				if (glm::length(glm::vec3(pos)) <= params.radius)
					addChunk(posRel);
			}
		}
	}
}

void WorldGrid::generateCylinder(const CylinderGeneratorParams& params)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool((params.radius * 2 + 1) * (params.radius * 2 + 1) * params.height * 2);

	glm::ivec3 pos;

	for (pos.x = -static_cast<int32_t>(params.radius);
		pos.x < static_cast<int32_t>(params.radius + 1); ++pos.x)
	{
		for (pos.z = -static_cast<int32_t>(params.radius);
			pos.z < static_cast<int32_t>(params.radius + 1); ++pos.z)
		{
			for (pos.y = 0; pos.y < static_cast<int64_t>(params.height); ++pos.y)
			{
				glm::ivec3 posRel = pos + params.bottomCenterPosition;
				if (glm::length(glm::vec2(pos.x, pos.z)) <= params.radius)
					addChunk(posRel);
			}
		}
	}
}

void WorldGrid::generateParallelepiped(const ParallelepipedGeneratorParams& params)
{
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool(params.width * params.height * params.depth * 2);
	glm::ivec3 pos;

	for (pos.x = params.cornerPosition.x; pos.x < params.cornerPosition.x + static_cast<int32_t>(params.width); ++pos.x)
		for (pos.z = params.cornerPosition.z; pos.z < params.cornerPosition.z + static_cast<int32_t>(params.depth); ++pos.z)
			for (pos.y = params.cornerPosition.y; pos.y < params.cornerPosition.y + static_cast<int32_t>(params.height); ++pos.y)
				addChunk(pos);
}

void WorldGrid::generateCube(const CubeGeneratorParams& params) {
	m_pool.clear();
	m_allocations.clear();
	m_coordToAllocation.clear();

	m_pool = GridPool(params.edge * params.edge * params.edge * 2);
	glm::ivec3 pos;

	for (pos.x = params.cornerPosition.x; pos.x < params.cornerPosition.x + static_cast<int32_t>(params.edge); ++pos.x)
		for (pos.z = params.cornerPosition.z; pos.z < params.cornerPosition.z + static_cast<int32_t>(params.edge); ++pos.z)
			for (pos.y = params.cornerPosition.y; pos.y < params.cornerPosition.y + static_cast<int32_t>(params.edge); ++pos.y)
				addChunk(pos);
}

void WorldGrid::sortAllocationsByDistance(glm::ivec3 centerPos)
{	
	std::sort(m_allocations.begin(), m_allocations.end(),
		[&centerPos](const GridPool::Allocation& left, const GridPool::Allocation& right) -> bool {
			float lengthRight = glm::length(glm::vec3(glm::ivec3(left.getField<1>().coord) - centerPos));
			float lengthLeft = glm::length(glm::vec3(glm::ivec3(right.getField<1>().coord) - centerPos));
			return lengthLeft > lengthRight;
		});

	m_coordToAllocation.clear();
	for (size_t i = 0; i < m_allocations.size(); ++i)
		m_coordToAllocation.insert({ glm::ivec3(m_allocations[i].getField<1>().coord), i });
}