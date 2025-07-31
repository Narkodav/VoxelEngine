#pragma once
#include "Common.h"
#include "VoxelAttribute.h"

class Voxel
{
public:
	struct State {
		Id::Model m_model;
		std::vector<Id::VoxelAttribute> m_attributes;
		std::string m_name;
	};

private:
	std::vector<Id::VoxelState> m_states; //can't be empty
	std::string m_name;

public:

	void setName(const std::string& name)
	{
		m_name = name;
	}

	void addState(Id::VoxelState state)
	{
		m_states.push_back(state);
	}

	const std::vector<Id::VoxelState>& getStates() const { return m_states; };
	const std::string& getName() const { return m_name; };
};

