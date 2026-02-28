#pragma once
#include "JsonParser/Value.h"
#include "Common.h"

#include <filesystem>
#include <string>

class EngineFilesystem {
public:
    enum class Path {
        EngineConfig,
        Count,
    };

    enum class Directory {
        Root,
        Executable,
        Assets,
        Shaders,

        Textures,
        Models,
        Voxels,

        Count,
    };

private:
    static inline const std::string s_defaultShaderDirectoryPath = "shaders";
    static inline const std::string s_defaultAssetDirectoryPath = "res/resourcePack";
    static inline const std::string s_defaultTextureDirectoryPath = "textures";
    static inline const std::string s_defaultModelDirectoryPath = "models";
    static inline const std::string s_defaultVoxelDirectoryPath = "voxels";


    std::array<std::filesystem::path, enumCast(Path::Count)> m_paths;
    std::array<std::filesystem::path, enumCast(Directory::Count)> m_directories;

public:

    void init() {
        m_paths[enumCast(Path::EngineConfig)] = getConfigFilepath();
        m_directories[enumCast(Directory::Executable)] = std::filesystem::current_path();
        
        Json::Value config = Json::Value::fromFile(m_paths[enumCast(Path::EngineConfig)].string()).front();
        
        if(!config.isObject()) throw std::runtime_error("Ill formed EngineConfig.json, root is not an object");
        auto& root = config.asObject();

        auto rootDirIt = root.find("rootDirectory");
        if(rootDirIt == root.end()) throw std::runtime_error("EngineConfig.json must contain a rootDirectory entry");
        auto& rootDir = rootDirIt->second;
        if(!rootDir.isString()) throw std::runtime_error("Ill formed EngineConfig.json, rootDirectory must contain a string");
        m_directories[enumCast(Directory::Root)] = rootDir.asString();
        m_directories[enumCast(Directory::Root)] = resolve(
            m_paths[enumCast(Path::EngineConfig)].parent_path(),
            m_directories[enumCast(Directory::Root)]
        );
        
        registerDirectory(Directory::Root, Directory::Assets, "assetDirectory", s_defaultAssetDirectoryPath, root);
        registerDirectory(Directory::Root, Directory::Shaders, "shaderDirectory", s_defaultShaderDirectoryPath, root);
        registerDirectory(Directory::Assets, Directory::Textures, "textureDirectory", s_defaultTextureDirectoryPath, root);
        registerDirectory(Directory::Assets, Directory::Models, "modelDirectory", s_defaultModelDirectoryPath, root);
        registerDirectory(Directory::Assets, Directory::Voxels, "voxelDirectory", s_defaultVoxelDirectoryPath, root);
    }

    const auto& getRootDirectory() const { return m_directories[enumCast(Directory::Root)]; }    
    const auto& getShaderDirectory() const { return m_directories[enumCast(Directory::Shaders)]; }
    const auto& getAssetDirectory() const { return m_directories[enumCast(Directory::Assets)]; }
    const auto& getTextureDirectory() const { return m_directories[enumCast(Directory::Textures)]; }
    const auto& getModelDirectory() const { return m_directories[enumCast(Directory::Models)]; }
    const auto& getVoxelDirectory() const { return m_directories[enumCast(Directory::Voxels)]; }

    void printDirectories() {
        std::cout << "Engine directories: " << std::endl;
        for(size_t i = 0; i < enumCast(Directory::Count); ++i) {
            std::cout << "- " << m_directories[i].string() << std::endl;
        }
    }

    void printPaths() {
        std::cout << "Engine paths: " << std::endl;
        for(size_t i = 0; i < enumCast(Path::Count); ++i) {
            std::cout << "- " << m_paths[i].string() << std::endl;
        }
    }

    auto getFile(Directory dir, std::string_view filename) const {
        return std::filesystem::weakly_canonical(m_directories[enumCast(dir)] / filename);
    }

private:

    void registerDirectory(Directory base, Directory target, std::string_view configName, 
        std::string_view defaultPath, Json::Value::Object& root) {
        
        auto targetDirIt = root.find(configName);
        if(targetDirIt == root.end()) {
            m_directories[enumCast(target)] = resolve(
                m_directories[enumCast(base)],
                defaultPath
            );
        }
        else {
            auto& targetDir = targetDirIt->second;
            if(!targetDir.isString()) {
                std::string error = std::string("Ill formed EngineConfig.json, ") 
                + std::string(configName) + std::string(" must contain a string");
                throw std::runtime_error(error);
            }
            m_directories[enumCast(target)] = targetDir.asString();
            m_directories[enumCast(target)] = resolve(
                m_directories[enumCast(base)],
                m_directories[enumCast(target)]
            );
        }
        
    }

    std::filesystem::path resolve(
        const std::filesystem::path& base,
        const std::filesystem::path& path)
    {
        if (path.is_absolute())
            return std::filesystem::weakly_canonical(path);
        return std::filesystem::weakly_canonical(base / path);
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

    // static inline std::filesystem::path getExeDirectory() {
    // #ifdef _WIN32
    //     char path[4096];
    //     GetModuleFileNameA(nullptr, path, MAX_PATH);
    //     return std::filesystem::path(path).parent_path();
    // #elif defined(__APPLE__)
    //     char path[4096];
    //     uint32_t size = sizeof(path);
    //     if (_NSGetExecutablePath(path, &size) == 0) {
    //         return std::filesystem::path(path).parent_path();
    //     }
    //     return std::filesystem::current_path();
    // #elif defined(__linux__)
    //     char path[4096];
    //     ssize_t count = readlink("/proc/self/exe", path, 4096);
    //     if (count != -1) {
    //         return std::filesystem::path(std::string(path, count)).parent_path();
    //     }
    //     return std::filesystem::current_path();
    // #else
    //     // Unknown platform - fallback to current directory
    //     return std::filesystem::current_path();
    // #endif
    // }

    static inline std::filesystem::path getConfigFilepath() {
        auto path = std::filesystem::current_path();
        try {
            for (size_t i = 0; i < 3; ++i)
            {
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    if (entry.is_regular_file() && entry.path().filename().string() == "EngineConfig.json")
                        return entry.path();
                }
                path = path.parent_path();
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cerr << "Error: asset directory not found" << std::endl;
        return std::filesystem::path();
    }

    static inline std::string getFileName(const std::string& filePath) {
        std::filesystem::path path(filePath);
        return path.stem().string();
    }
};