#pragma once
#include <vector>

#include "Shape.h"

class VoxelModel
{
private:
	Id::Model m_shape;
	std::string m_name;

public:
	VoxelModel() = default;
	~VoxelModel() = default;

	void addShape(Id::Shape parallelogram) { m_parallelograms.push_back(parallelogram); };
	void setName(const std::string& name) { m_name = name; };

	const std::string& getName() const { return m_name; };
	const std::vector<Id::Shape>& getShapes() const { return m_parallelograms; };
};

