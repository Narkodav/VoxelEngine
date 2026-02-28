#pragma once
#include "Common.h"
#include "GameData/EngineFilesystem.h"

class ShaderCache
{
public:
	enum class ShaderPurpose
	{
		VoxelVert = 0,
        CubeVert = 0,
		VoxelFrag,
        //ModelVert,
        //ModelFrag,
        /*VoxelCompute,*/
		Count
	};

private:
	Gfx::Utility::ShaderModuleData m_shaderModuleData;
    static inline const std::array<std::string, enumCast(ShaderPurpose::Count)> m_shaderFileNames = {
        "Voxel.vert.spv",
        "Cube.vert.spv",
        "Voxel.frag.spv",
        //"Rendering/Shaders/Model.vert.spv",
        //"Rendering/Shaders/Model.frag.spv",
        /*"Rendering/Shaders/basic.comp.spv",*/
    };

	bool m_initialized = false;

public:	

	ShaderCache(const Gfx::Wrappers::Device& device, const EngineFilesystem& engineFiles) {        
        m_shaderModuleData.shaderCodes.resize(m_shaderFileNames.size());
        m_shaderModuleData.shaderModules.resize(m_shaderFileNames.size());
        m_shaderModuleData.shaderModuleCreateInfos.resize(m_shaderFileNames.size());
        m_shaderModuleData.shaderStages.resize(m_shaderFileNames.size());

        for (size_t i = 0; i < m_shaderFileNames.size(); ++i)
        {
            auto filePath = engineFiles.getFile(EngineFilesystem::Directory::Shaders, m_shaderFileNames[i]);
            m_shaderModuleData.shaderCodes[i] = Graphics::ShaderModule::parseShaderCodeSPIRV(filePath.string());
            m_shaderModuleData.shaderModuleCreateInfos[i].setShaderCode(m_shaderModuleData.shaderCodes[i]);
            m_shaderModuleData.shaderModules[i].create(device.getFunctionTable(), device, m_shaderModuleData.shaderModuleCreateInfos[i]);
            m_shaderModuleData.shaderStages[i] = Graphics::ShaderModule::inferShaderStage(m_shaderModuleData.shaderCodes[i]);
        }
        m_initialized = true;
	};

    ShaderCache() {};

    ShaderCache(ShaderCache&&) noexcept = default;

    //moving to an initialized device is undefined behavior, destroy before moving
    ShaderCache& operator=(ShaderCache&& other) noexcept
    {
        if (this == &other)
            return *this;

        assert(!m_initialized && "ShaderCache::operator=() - ShaderCache already initialized");

        m_shaderModuleData = std::exchange(other.m_shaderModuleData, Gfx::Utility::ShaderModuleData());
        m_initialized = std::exchange(other.m_initialized, false);

        return *this;
    };

    ShaderCache(const ShaderCache&) noexcept = delete;
    ShaderCache& operator=(const ShaderCache&) noexcept = delete;

    ~ShaderCache() { assert(!m_initialized && "ShaderCache was not destroyed!"); };

    void destroy(const Gfx::Wrappers::Device& device) {
        if (!m_initialized)
            return;

        for (auto& shader : m_shaderModuleData.shaderModules)
            shader.destroy(device.getFunctionTable(), device);

        m_initialized = false;
    }

    std::vector<Gfx::PipelineShaderStageCreateInfo> getShaderStageCreateInfos()
    {
        return Gfx::Utility::createShaderStageInfos(m_shaderModuleData);
    }
};

