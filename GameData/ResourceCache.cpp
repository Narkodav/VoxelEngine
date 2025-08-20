#include "ResourceCache.h"

void ResourceCache::registerResources(const std::string& voxelsPath, const std::string& texturePath, const std::string& modelsPath)
{
    m_assetCache.init(texturePath, modelsPath);

    auto voxelFiles = getFilesInDirectory(voxelsPath);

    auto emptyStateId = m_voxelStates.add(
        Voxel::State{
            Constants::emptyModelId,
            {},
            "empty"
        }, "empty"
    );

    if (emptyStateId != Constants::emptyStateId)
        throw std::runtime_error("Empty state id mismatch, clear cache before registering voxels");

    for (size_t i = 0; i < voxelFiles.size(); i++)
        registerVoxel(voxelFiles[i]);
}

void ResourceCache::registerVoxel(const std::string& path)
{
    Json::Document document;
    try
    {
        document = Json::Parser::parseFile(path);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error parsing JSON file '" << path << "': " << e.what() << std::endl;
        return;
    }

    try
    {
        if (document.roots().size() != 1)
            throw std::runtime_error("Multiple roots in voxel definition");

        const auto& root = *(document.roots().begin());
        if (!root->isObject())
            throw std::runtime_error("Root is not an object in voxel definition");

        const auto& rootData = root->asObject().data();
        const auto& name = rootData.find("name");

        if (name == rootData.end())
            throw std::runtime_error("Missing required 'name' field in voxel definition");

        if (!name->second->isString())
            throw std::runtime_error("'name' must be a string in voxel definition");

        Voxel voxel;
        voxel.setName(name->second->asString().data());

        // Check for optional model field
        const auto& model = rootData.find("model");
        if (model != rootData.end())
        {
            if (!model->second->isString())
                throw std::runtime_error("'model' must be a string in voxel definition");

            auto id = m_voxelStates.add(
                Voxel::State{
                    m_assetCache.getModelCache().getId(model->second->asString().data()),
                    {},
                    name->second->asString().data()
                }, name->second->asString().data()
            );
            voxel.addState(id);
        }
        else
        {
            // Check for optional states field
            const auto& states = rootData.find("states");
            if (states != rootData.end())
            {
                if (!states->second->isArray())
                    throw std::runtime_error("'states' must be an array in voxel definition");
                const auto& statesArray = states->second->asArray().data();
                for (size_t i = 0; i < statesArray.size(); ++i)
                {
                    if(!statesArray[i]->isObject())
                        throw std::runtime_error("Each state must be an object in voxel definition");
                    const auto& state = statesArray[i]->asObject().data();
                    const auto& name = state.find("name");
                    if (name == state.end())
                        throw std::runtime_error("Missing required 'name' field in state definition");
                    if (!name->second->isString())
                        throw std::runtime_error("'name' must be a string in state definition");

                    const auto& model = state.find("model");
                    if (model != state.end())
                    {
                        if (!model->second->isString())
                            throw std::runtime_error("'model' must be a string in state definition");

                        auto id = m_voxelStates.add(
                            Voxel::State{
                                m_assetCache.getModelCache().getId(model->second->asString().data()),
                                {},
                                name->second->asString().data()
                            }, name->second->asString().data()
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
            if (!attributes->second->isArray())
                throw std::runtime_error("'attributes' must be an array in voxel definition");

            const auto& attributesArray = attributes->second->asArray().data();
            for (const auto& attribute : attributesArray)
            {
                if (!attribute->isString())
                    throw std::runtime_error("Each attribute must be a string in voxel definition");

                // Process attribute if needed
                // voxel.addAttribute(attribute->asString().data());
            }
        }

        m_voxels.add(std::move(voxel), voxel.getName());
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error processing voxel file '" << path << "': " << e.what() << std::endl;
    }
}