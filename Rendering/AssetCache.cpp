#include "AssetCache.h"

void AssetCache::init()
{
    registerAssets("res/resourcePack/textures", "res/resourcePack/models");
}

void AssetCache::clearCache(const Gfx::Context& instance, const Gfx::Device& device)
{
    m_storageCache.destroy(instance, device);
}

void AssetCache::registerAssets(const std::string& texturesPath, const std::string& modelsPath)
{
    auto textureFiles = getFilesInDirectory(texturesPath);
    auto modelFiles = getFilesInDirectory(modelsPath);

    for (size_t i = 0; i < textureFiles.size(); i++)
        registerTexture(textureFiles[i]);

    for (size_t i = 0; i < modelFiles.size(); i++)
        registerVoxelModel(modelFiles[i]);

}

void AssetCache::moveAssetsToGpuStorage(const Gfx::Context& instance, const Gfx::Device& device,
    const Gfx::Queue& transferQueue, Gfx::CommandPool& temporaryPool, Gfx::MappedMemory& stagingMemory,
    Gfx::Buffer& stagingBuffer, const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStateCache)
{
    m_storageCache.setup(instance, device, transferQueue, temporaryPool,
        stagingMemory, stagingBuffer, m_vertexCache, m_uvCache,
        voxelStateCache, m_modelCache, m_geometryCache, m_appearanceCache,
        m_polygonCache, m_coloringCache, m_textureCache);
}

void AssetCache::registerTexture(const std::string& texturePath)
{
    m_textureCache.add(Gfx::PixelData(texturePath), getFileName(texturePath));
}

void AssetCache::registerVoxelModel(const std::string& path)
{
    Json::Document document;
    try
    {
        document = Json::Parser::parseFile(path);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return;
    }

    try
    {
        if (document.roots().size() != 1)
            throw std::runtime_error("multiple roots in voxel model");

        const auto& root = *(document.roots().begin());
        if (!root->isObject())
            throw std::runtime_error("roots is not an object in voxel model");

        const auto& rootData = root->asObject().data();
        const auto& name = rootData.find("name");
        const auto& shapes = rootData.find("shapes");

        if (name == rootData.end() || shapes == rootData.end()) {
            throw std::runtime_error("Missing required fields in voxel model");
        };

        // Get the model name
        if (!name->second->isString())
            throw std::runtime_error("voxel model name must be a string");

        // Get the shapes array
        if (!shapes->second->isArray())
            throw std::runtime_error("voxel model shapes must be an array");
        const auto& shapesArray = shapes->second->asArray().data();

        std::vector<Id::Polygon> geometry;
        std::vector<Id::Coloring> appearence;

        for (const auto& shape : shapesArray) {
            parseShape(shape, geometry, appearence);
        }

        auto geometryId = m_geometryCache.add(geometry);
        auto appearenceId = m_appearanceCache.add(appearence);

        Shape::Model model = { geometryId, appearenceId };

        m_modelCache.add(std::move(model), name->second->asString().data());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error processing voxel model file '" << path << "': " << e.what() << std::endl;
    }
}

void AssetCache::parseShape(const std::unique_ptr<Json::Value>& shape,
    std::vector<Id::Polygon>& geometry,
    std::vector<Id::Coloring>& appearence) {

    if (!shape->isObject())
        throw std::runtime_error("Shape is not an object in parallelogram definition");

    const auto& shapeData = shape->asObject().data();
    const auto& dimensions = shapeData.find("dimensions");

    if (dimensions == shapeData.end())
        throw std::runtime_error("Missing required 'dimensions' field in parallelogram definition");

    // Parse dimensions (required)
    if (!dimensions->second->isObject())
        throw std::runtime_error("'dimensions' must be an object in parallelogram definition");

    const auto& dimData = dimensions->second->asObject().data();
    const auto& width = dimData.find("width");
    const auto& height = dimData.find("height");
    const auto& depth = dimData.find("depth");

    if (width == dimData.end())
        throw std::runtime_error("Missing required 'width' in dimensions");
    if (height == dimData.end())
        throw std::runtime_error("Missing required 'height' in dimensions");
    if (depth == dimData.end())
        throw std::runtime_error("Missing required 'depth' in dimensions");
    
    if (!width->second->isNumber())
        throw std::runtime_error("'width' must be a number in dimensions");
    if (!height->second->isNumber())
        throw std::runtime_error("'height' must be a number in dimensions");
    if (!depth->second->isNumber())
        throw std::runtime_error("'depth' must be a number in dimensions");


    glm::vec3 dimension = glm::vec3(
        width->second->asNumber().data(),
        height->second->asNumber().data(),
        depth->second->asNumber().data()
    );
    glm::vec3 position;
    glm::vec3 rotation;

    // Parse position (optional)
    const auto& positionHandle = shapeData.find("position");
    if (positionHandle != shapeData.end())
    {
        if (!positionHandle->second->isObject())
            throw std::runtime_error("'position' must be an object in parallelogram definition");

        const auto& posData = positionHandle->second->asObject().data();
        const auto& x = posData.find("x");
        const auto& y = posData.find("y");
        const auto& z = posData.find("z");

        if (x == posData.end())
            throw std::runtime_error("Missing 'x' coordinate in position");
        if (y == posData.end())
            throw std::runtime_error("Missing 'y' coordinate in position");
        if (z == posData.end())
            throw std::runtime_error("Missing 'z' coordinate in position");

        if (!x->second->isNumber())
            throw std::runtime_error("'x' coordinate must be a number");
        if (!y->second->isNumber())
            throw std::runtime_error("'y' coordinate must be a number");
        if (!z->second->isNumber())
            throw std::runtime_error("'z' coordinate must be a number");

        position = glm::vec3(
            x->second->asNumber().data(),
            y->second->asNumber().data(),
            z->second->asNumber().data()
        );
    }
    else position = glm::vec3(Constants::gridCellCenterOffset,
        Constants::gridCellCenterOffset, Constants::gridCellCenterOffset);

    // Parse rotation (optional)
    const auto& rotationHandle = shapeData.find("rotation");
    if (rotationHandle != shapeData.end())
    {
        if (!rotationHandle->second->isObject())
            throw std::runtime_error("'rotation' must be an object in parallelogram definition");
            
        const auto& rotData = rotationHandle->second->asObject().data();
        const auto& x = rotData.find("x");
        const auto& y = rotData.find("y");
        const auto& z = rotData.find("z");

        if (x == rotData.end())
            throw std::runtime_error("Missing 'x' angle in rotation");
        if (y == rotData.end())
            throw std::runtime_error("Missing 'y' angle in rotation");
        if (z == rotData.end())
            throw std::runtime_error("Missing 'z' angle in rotation");
            
        if (!x->second->isNumber())
            throw std::runtime_error("'x' angle must be a number");
        if (!y->second->isNumber())
            throw std::runtime_error("'y' angle must be a number");
        if (!z->second->isNumber())
            throw std::runtime_error("'z' angle must be a number");

        rotation = glm::vec3(
            x->second->asNumber().data(),
            y->second->asNumber().data(),
            z->second->asNumber().data()
        );
    }
    else rotation = glm::vec3(0, 0, 0);

    Id::Texture textureIds[6];
    glm::vec2 uvs[6][4];

    // Parse textures and UVs (optional)
    const auto& textures = shapeData.find("textures");
    if (textures != shapeData.end())
    {
        if (!textures->second->isObject())
            throw std::runtime_error("'textures' must be an object in parallelogram definition");
            
        const auto& texturesData = textures->second->asObject().data();

        // Process each face
        for (size_t i = 0; i < enumCast(Shape::Side::NUM); ++i)
        {
            auto& faceName = Shape::sideFaceNames[i];
            const auto& face = texturesData.find(faceName.data());
            
            if (face != texturesData.end())
            {
                if (!face->second->isObject())
                    throw std::runtime_error("Face '" + std::string(faceName.data()) + "' must be an object");
                    
                const auto& faceData = face->second->asObject().data();
                const auto& name = faceData.find("name");

                if (name != faceData.end())
                {
                    if (!name->second->isString())
                        throw std::runtime_error("Texture name for face '" + std::string(faceName.data()) + "' must be a string");
                        
                    textureIds[i] = m_textureCache.getId(name->second->asString().data());
                }

                const auto& uv = faceData.find("uv");
                if (uv != faceData.end())
                {
                    if (!uv->second->isObject())
                        throw std::runtime_error("'uv' for face '" + std::string(faceName.data()) + "' must be an object");
                        
                    const auto& uvData = uv->second->asObject().data();
                    const auto& u1 = uvData.find("u1");
                    const auto& v1 = uvData.find("v1");
                    const auto& u2 = uvData.find("u2");
                    const auto& v2 = uvData.find("v2");
                    const auto& u3 = uvData.find("u3");
                    const auto& v3 = uvData.find("v3");
                    const auto& u4 = uvData.find("u4");
                    const auto& v4 = uvData.find("v4");

                    if (u1 == uvData.end())
                        throw std::runtime_error("Missing 'u1' in UV for face '" + std::string(faceName.data()) + "'");
                    if (v1 == uvData.end())
                        throw std::runtime_error("Missing 'v1' in UV for face '" + std::string(faceName.data()) + "'");
                    if (u2 == uvData.end())
                        throw std::runtime_error("Missing 'u2' in UV for face '" + std::string(faceName.data()) + "'");
                    if (v2 == uvData.end())
                        throw std::runtime_error("Missing 'v2' in UV for face '" + std::string(faceName.data()) + "'");
                    if (u3 == uvData.end())
                        throw std::runtime_error("Missing 'u3' in UV for face '" + std::string(faceName.data()) + "'");
                    if (v3 == uvData.end())
                        throw std::runtime_error("Missing 'v3' in UV for face '" + std::string(faceName.data()) + "'");
                    if (u4 == uvData.end())
                        throw std::runtime_error("Missing 'u4' in UV for face '" + std::string(faceName.data()) + "'");
                    if (v4 == uvData.end())
                        throw std::runtime_error("Missing 'v4' in UV for face '" + std::string(faceName.data()) + "'");
                    
                    if (!u1->second->isNumber() || !v1->second->isNumber() ||
                        !u2->second->isNumber() || !v2->second->isNumber() ||
                        !u3->second->isNumber() || !v3->second->isNumber() ||
                        !u4->second->isNumber() || !v4->second->isNumber())
                    {
                        throw std::runtime_error("All UV coordinates for face '" + std::string(faceName.data()) + "' must be numbers");
                    }

                    uvs[i][0] = glm::vec2(u1->second->asNumber().data(), v1->second->asNumber().data());
                    uvs[i][1] = glm::vec2(u2->second->asNumber().data(), v2->second->asNumber().data());
                    uvs[i][2] = glm::vec2(u3->second->asNumber().data(), v3->second->asNumber().data());
                    uvs[i][3] = glm::vec2(u4->second->asNumber().data(), v4->second->asNumber().data());
                }
                else
                {
                    uvs[i][0] = glm::vec2(Shape::defaultUvs[0].x, Shape::defaultUvs[0].y);
                    uvs[i][1] = glm::vec2(Shape::defaultUvs[1].x, Shape::defaultUvs[1].y);
                    uvs[i][2] = glm::vec2(Shape::defaultUvs[2].x, Shape::defaultUvs[2].y);
                    uvs[i][3] = glm::vec2(Shape::defaultUvs[3].x, Shape::defaultUvs[3].y);
                }
            }
        }

        Shape::registerParallelogram(
            textureIds,
            uvs,
            dimension,
            position,
            rotation,
            m_vertexCache,
            m_uvCache,
            m_polygonCache,
            m_coloringCache,
            geometry,
            appearence);
    }
}