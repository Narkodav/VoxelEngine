#pragma once
#include "Graphics/Common.h"
#include "Graphics/Rendering/Shader.h"
#include "Graphics/Rendering/Device.h"
#include "Graphics/Rendering/Context.h"

#include "Common.h"

class ShaderCache
{
public:
	enum class ShaderPurpose
	{
		VoxelVert = 0,
		VoxelFrag,
        /*VoxelCompute,*/
		NUM
	};

private:
	std::array<Gfx::Shader, enumCast(ShaderPurpose::NUM)> m_shaders;

	std::array<std::string, enumCast(ShaderPurpose::NUM)> m_shaderPaths = {
        "Rendering/Shaders/basic.vert.spv",
        "Rendering/Shaders/basic.frag.spv",
        /*"Rendering/Shaders/basic.comp.spv",*/
	};

	bool m_initialized = false;

public:	

	ShaderCache(const Gfx::Context& context, const Gfx::Device& device) {
		for (size_t i = 0; i < enumCast(ShaderPurpose::NUM); ++i)
			m_shaders[i] = Gfx::Shader(context, device, m_shaderPaths[i]);
        m_initialized = true;
	};

    ShaderCache() {};

    ShaderCache(ShaderCache&& other) noexcept {
        m_shaders = std::exchange(other.m_shaders, std::array<Gfx::Shader, enumCast(ShaderPurpose::NUM)>());
        m_initialized = std::exchange(other.m_initialized, false);
    };

    //moving to an initialized device is undefined behavior, destroy before moving
    ShaderCache& operator=(ShaderCache&& other) noexcept
    {
        if (this == &other)
            return *this;

        assert(!m_initialized && "ShaderCache::operator=() - ShaderCache already initialized");

        m_shaders = std::exchange(other.m_shaders, std::array<Gfx::Shader, enumCast(ShaderPurpose::NUM)>());
        m_initialized = std::exchange(other.m_initialized, false);

        return *this;
    };

    ShaderCache(const ShaderCache&) noexcept = delete;
    ShaderCache& operator=(const ShaderCache&) noexcept = delete;

    ~ShaderCache() { assert(!m_initialized && "ShaderCache was not destroyed!"); };

    void destroy(const Gfx::Context& instance, const Gfx::Device& device) {
        if (!m_initialized)
            return;

        for (size_t i = 0; i < enumCast(ShaderPurpose::NUM); ++i)
            m_shaders[i].destroy(instance, device);

        m_initialized = false;
    }

	const Gfx::Shader& getShader(ShaderPurpose shaderType)
	{
		return m_shaders[enumCast(shaderType)];
	}
};

