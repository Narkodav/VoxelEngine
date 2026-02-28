#include "GameData/ResourceCache.h"

void ResourceCache::registerResources(const EngineFilesystem& engineFiles)
{
    std::cout << "Registering resources" << std::endl;
    m_assetCache.init(engineFiles);

    const auto& voxelPath = engineFiles.getVoxelDirectory();

    auto emptyStateId = m_voxelStates.add(
        Voxel::State{
            Constants::emptyModelId,
            {},
            "empty"
        }, "empty"
    );

    if (emptyStateId != Constants::emptyStateId)
        throw std::runtime_error("Empty state id mismatch, clear cache before registering voxels");

    for (const auto& entry : std::filesystem::directory_iterator(voxelPath)) {
        if (entry.is_regular_file()) {
            registerVoxel(entry.path().string());
        }
    }
}

void ResourceCache::registerVoxel(std::string_view path)
{
    std::cout << "Registering voxel" << std::endl;
    std::vector<Json::Value> roots;
    try
    {
        roots = Json::Value::fromFile(path);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error parsing JSON file '" << path << "': " << e.what() << std::endl;
        return;
    }

    try
    {
        if (roots.size() != 1)
            throw std::runtime_error("Multiple roots in voxel definition");

        const auto& root = roots.begin();
        if (!root->isObject())
            throw std::runtime_error("Root is not an object in voxel definition");

        const auto& rootData = root->asObject();
        const auto& name = rootData.find("name");

        if (name == rootData.end())
            throw std::runtime_error("Missing required 'name' field in voxel definition");

        if (!name->second.isString())
            throw std::runtime_error("'name' must be a string in voxel definition");

        Voxel voxel;
        voxel.setName(name->second.asString());

        // Check for optional model field
        const auto& model = rootData.find("model");
        if (model != rootData.end())
        {
            if (!model->second.isString())
                throw std::runtime_error("'model' must be a string in voxel definition");

            auto id = m_voxelStates.add(
                Voxel::State{
                    m_assetCache.getModelCache().getId(model->second.asString()),
                    {},
                    name->second.asString()
                }, name->second.asString()
            );
            voxel.addState(id);
        }
        else
        {
            // Check for optional states field
            const auto& states = rootData.find("states");
            if (states != rootData.end())
            {
                if (!states->second.isArray())
                    throw std::runtime_error("'states' must be an array in voxel definition");
                const auto& statesArray = states->second.asArray();
                for (size_t i = 0; i < statesArray.size(); ++i)
                {
                    if(!statesArray[i].isObject())
                        throw std::runtime_error("Each state must be an object in voxel definition");
                    const auto& state = statesArray[i].asObject();
                    const auto& name = state.find("name");
                    if (name == state.end())
                        throw std::runtime_error("Missing required 'name' field in state definition");
                    if (!name->second.isString())
                        throw std::runtime_error("'name' must be a string in state definition");

                    const auto& model = state.find("model");
                    if (model != state.end())
                    {
                        if (!model->second.isString())
                            throw std::runtime_error("'model' must be a string in state definition");

                        auto id = m_voxelStates.add(
                            Voxel::State{
                                m_assetCache.getModelCache().getId(model->second.asString()),
                                {},
                                name->second.asString()
                            }, name->second.asString()
                        );
                        voxel.addState(id);
                    }
                    else //no states, use empty state
                        voxel.addState(Constants::emptyStateId);
                }
            }
            else //no states, use empty state
                voxel.addState(Constants::emptyStateId);
        }

        // Check for optional attributes array
        const auto& attributes = rootData.find("attributes");
        if (attributes != rootData.end())
        {
            if (!attributes->second.isArray())
                throw std::runtime_error("'attributes' must be an array in voxel definition");

            const auto& attributesArray = attributes->second.asArray();
            for (const auto& attribute : attributesArray)
            {
                if (!attribute.isString())
                    throw std::runtime_error("Each attribute must be a string in voxel definition");

                // Process attribute if needed
                // voxel.addAttribute(attribute.asString());
            }
        }

        m_voxels.add(std::move(voxel), voxel.getName());
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error processing voxel file '" << path << "': " << e.what() << std::endl;
    }
}