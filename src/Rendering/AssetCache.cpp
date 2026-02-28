#include "Rendering/AssetCache.h"

void AssetCache::init(const EngineFilesystem& engineFiles)
{
    registerAssets(engineFiles);
}

void AssetCache::clearCache(const Gfx::Wrappers::Device& device)
{
    m_storageCache.destroy(device);
}

void AssetCache::registerAssets(const EngineFilesystem& engineFiles)
{
    std::cout << "Registering assets" << std::endl;

    const auto& textureDir = engineFiles.getTextureDirectory();
    for (const auto& entry : std::filesystem::directory_iterator(textureDir)) {
        if (entry.is_regular_file()) {
            registerTexture(entry.path().string(), entry.path().filename().replace_extension().string());
        }
    }

    //ensures standard block geometry is at index 0
    {
        std::vector<Id::Polygon> geometry;
        std::vector<Id::Vertex> verticesInd;
        Shape::Polygon polygon;

        for (size_t i = 0; i < 6; i++) {
            for (size_t j = 0; j < 6; j++)
                verticesInd.push_back(m_vertexCache.add(Shape::s_conversionMatrices[i] * glm::vec4(
                    Shape::s_frontFacePositions[Shape::indices[j]].x,
                    Shape::s_frontFacePositions[Shape::indices[j]].y,
                    Shape::s_frontFacePositions[Shape::indices[j]].z,
                    1)));
            polygon.position[0] = verticesInd[verticesInd.size() - 6];
            polygon.position[1] = verticesInd[verticesInd.size() - 5];
            polygon.position[2] = verticesInd[verticesInd.size() - 4];
            polygon.normal = m_normalCache.add(glm::vec4(
                Constants::directionsFloat3D[enumCast(static_cast<Directions3D>(i))], 0));

            geometry.push_back(m_polygonCache.add(polygon));

            polygon.position[0] = verticesInd[verticesInd.size() - 3];
            polygon.position[1] = verticesInd[verticesInd.size() - 2];
            polygon.position[2] = verticesInd[verticesInd.size() - 1];
            polygon.normal = m_normalCache.add(glm::vec4(
                Constants::directionsFloat3D[enumCast(static_cast<Directions3D>(i))], 0));

            geometry.push_back(m_polygonCache.add(polygon));
        }

        m_standartBlockGeometryId = m_geometryCache.add(geometry, { Shape::GeometryType::Cube });
    }

    const auto& modelDir = engineFiles.getModelDirectory();
    for (const auto& entry : std::filesystem::directory_iterator(modelDir)) {
        if (entry.is_regular_file()) {
            registerVoxelModel(entry.path().string());
        }
    }
    m_voxelCullingCache.init(m_geometryCache, m_polygonCache, m_vertexCache, m_normalCache, m_standartBlockGeometryId);
}

void AssetCache::moveAssetsToGpuStorage(const Gfx::Wrappers::Device& device,
    const Gfx::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
    Gfx::Queue transferQueue, Gfx::CommandPoolRef temporaryPool,
    Gfx::BufferRef stagingBuffer, Gfx::MemoryMapping& stagingMapping, size_t stagingBuffeSize,
    const Id::NamedCache<Voxel::State, Id::VoxelState>& voxelStateCache)
{
    m_storageCache.setup(device, physicalDeviceMemoryProperties, transferQueue, temporaryPool,
        stagingBuffer, stagingMapping, stagingBuffeSize, m_vertexCache, m_uvCache, m_normalCache,
        voxelStateCache, m_modelCache, m_geometryCache, m_appearanceCache,
        m_polygonCache, m_coloringCache, m_textureCache);
}

void AssetCache::registerTexture(std::string_view texturePath, std::string_view textureName)
{
    std::cout << "Registering texture" << std::endl;
    m_textureCache.add(Gfx::Utility::PixelData2D(texturePath), textureName);
}

void AssetCache::registerVoxelModel(std::string_view path)
{
    std::cout << "Registering voxel model" << std::endl;
    std::vector<Json::Value> roots;
    try
    {
        roots = Json::Value::fromFile(path);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error parsing JSON file: " << e.what() << std::endl;
        return;
    }

    try
    {
        if (roots.size() != 1)
            throw std::runtime_error("multiple roots in voxel model");

        const auto& root = *(roots.begin());
        if (!root.isObject())
            throw std::runtime_error("roots is not an object in voxel model");

        const auto& rootData = root.asObject();
        const auto& name = rootData.find("name");
        const auto& shapes = rootData.find("shapes");

        if (name == rootData.end() || shapes == rootData.end()) {
            throw std::runtime_error("Missing required fields in voxel model");
        };

        // Get the model name
        if (!name->second.isString())
            throw std::runtime_error("voxel model name must be a string");

        // Get the shapes array
        if (!shapes->second.isArray())
            throw std::runtime_error("voxel model shapes must be an array");
        const auto& shapesArray = shapes->second.asArray();

        std::vector<Id::Polygon> geometry;
        std::vector<Id::Coloring> appearence;
        Shape::GeometryType geometryType;

        if(shapesArray.size() > 1) {        
            for (const auto& shape : shapesArray) {
                parseShape(shape, geometry, appearence);
            }
            geometryType = Shape::GeometryType::Generic;
        }
        else geometryType = parseShape(shapesArray.front(), geometry, appearence);

        auto geometryId = m_geometryCache.add(geometry, { geometryType });
        auto appearenceId = m_appearanceCache.add(appearence);

        Shape::Model model = { geometryId, appearenceId };

        m_modelCache.add(std::move(model), name->second.asString());
    } catch (const std::exception& e) {
        std::cerr << "Error processing voxel model file '" << path << "': " << e.what() << std::endl;
    }
}

Shape::GeometryType AssetCache::parseShape(const Json::Value& shape,
    std::vector<Id::Polygon>& geometry,
    std::vector<Id::Coloring>& appearence) {

    glm::vec3 dimension;
    glm::vec3 position;
    glm::vec3 rotation;

    if (!shape.isObject())
        throw std::runtime_error("Shape is not an object in parallelogram definition");

    const auto& shapeData = shape.asObject();
    const auto& dimensions = shapeData.find("dimensions");

    if (dimensions == shapeData.end()) {
        dimension = Shape::s_defaultDimensions;
    } else if (!dimensions->second.isObject()) {
        throw std::runtime_error("'dimensions' must be an object in parallelogram definition");
    }
    else {        
        const auto& dimData = dimensions->second.asObject();
        const auto& width = dimData.find("width");
        const auto& height = dimData.find("height");
        const auto& depth = dimData.find("depth");

        if (width == dimData.end())
            throw std::runtime_error("Missing required 'width' in dimensions");
        if (height == dimData.end())
            throw std::runtime_error("Missing required 'height' in dimensions");
        if (depth == dimData.end())
            throw std::runtime_error("Missing required 'depth' in dimensions");
        
        if (!width->second.isNumber())
            throw std::runtime_error("'width' must be a number in dimensions");
        if (!height->second.isNumber())
            throw std::runtime_error("'height' must be a number in dimensions");
        if (!depth->second.isNumber())
            throw std::runtime_error("'depth' must be a number in dimensions");

        dimension = glm::vec3(
            width->second.asNumber(),
            height->second.asNumber(),
            depth->second.asNumber()
        );
    }

    // Parse position (optional)
    const auto& positionHandle = shapeData.find("position");
    if (positionHandle != shapeData.end())
    {
        if (!positionHandle->second.isObject())
            throw std::runtime_error("'position' must be an object in parallelogram definition");

        const auto& posData = positionHandle->second.asObject();
        const auto& x = posData.find("x");
        const auto& y = posData.find("y");
        const auto& z = posData.find("z");

        if (x == posData.end())
            throw std::runtime_error("Missing 'x' coordinate in position");
        if (y == posData.end())
            throw std::runtime_error("Missing 'y' coordinate in position");
        if (z == posData.end())
            throw std::runtime_error("Missing 'z' coordinate in position");

        if (!x->second.isNumber())
            throw std::runtime_error("'x' coordinate must be a number");
        if (!y->second.isNumber())
            throw std::runtime_error("'y' coordinate must be a number");
        if (!z->second.isNumber())
            throw std::runtime_error("'z' coordinate must be a number");

        position = glm::vec3(
            x->second.asNumber(),
            y->second.asNumber(),
            z->second.asNumber()
        );
    }
    else position = Shape::s_defaultPosition;

    // Parse rotation (optional)
    const auto& rotationHandle = shapeData.find("rotation");
    if (rotationHandle != shapeData.end())
    {
        if (!rotationHandle->second.isObject())
            throw std::runtime_error("'rotation' must be an object in parallelogram definition");
            
        const auto& rotData = rotationHandle->second.asObject();
        const auto& x = rotData.find("x");
        const auto& y = rotData.find("y");
        const auto& z = rotData.find("z");

        if (x == rotData.end())
            throw std::runtime_error("Missing 'x' angle in rotation");
        if (y == rotData.end())
            throw std::runtime_error("Missing 'y' angle in rotation");
        if (z == rotData.end())
            throw std::runtime_error("Missing 'z' angle in rotation");
            
        if (!x->second.isNumber())
            throw std::runtime_error("'x' angle must be a number");
        if (!y->second.isNumber())
            throw std::runtime_error("'y' angle must be a number");
        if (!z->second.isNumber())
            throw std::runtime_error("'z' angle must be a number");

        rotation = glm::vec3(
            x->second.asNumber(),
            y->second.asNumber(),
            z->second.asNumber()
        );
    }
    else rotation = Shape::s_defaultRotations;

    Id::Texture textureIds[6];
    glm::vec2 uvs[6][4];

    // Parse textures and UVs (optional)
    const auto& textures = shapeData.find("textures");
    if (textures != shapeData.end())
    {
        if (!textures->second.isObject())
            throw std::runtime_error("'textures' must be an object in parallelogram definition");
            
        const auto& texturesData = textures->second.asObject();

        // Process each face
        for (size_t i = 0; i < enumCast(Shape::Side::Count); ++i)
        {
            auto& faceName = Shape::s_sideFaceNames[i];
            const auto& face = texturesData.find(faceName);
            
            if (face != texturesData.end())
            {
                if (!face->second.isObject())
                    throw std::runtime_error("Face '" + faceName + "' must be an object");
                    
                const auto& faceData = face->second.asObject();
                const auto& name = faceData.find("name");

                if (name != faceData.end())
                {
                    if (!name->second.isString())
                        throw std::runtime_error("Texture name for face '" + faceName + "' must be a string");
                        
                    textureIds[i] = m_textureCache.getId(name->second.asString());
                }

                const auto& uv = faceData.find("uv");
                if (uv != faceData.end())
                {
                    if (!uv->second.isObject())
                        throw std::runtime_error("'uv' for face '" + faceName + "' must be an object");
                        
                    const auto& uvData = uv->second.asObject();
                    const auto& u1 = uvData.find("u1");
                    const auto& v1 = uvData.find("v1");
                    const auto& u2 = uvData.find("u2");
                    const auto& v2 = uvData.find("v2");
                    const auto& u3 = uvData.find("u3");
                    const auto& v3 = uvData.find("v3");
                    const auto& u4 = uvData.find("u4");
                    const auto& v4 = uvData.find("v4");

                    if (u1 == uvData.end())
                        throw std::runtime_error("Missing 'u1' in UV for face '" + faceName + "'");
                    if (v1 == uvData.end())
                        throw std::runtime_error("Missing 'v1' in UV for face '" + faceName + "'");
                    if (u2 == uvData.end())
                        throw std::runtime_error("Missing 'u2' in UV for face '" + faceName + "'");
                    if (v2 == uvData.end())
                        throw std::runtime_error("Missing 'v2' in UV for face '" + faceName + "'");
                    if (u3 == uvData.end())
                        throw std::runtime_error("Missing 'u3' in UV for face '" + faceName + "'");
                    if (v3 == uvData.end())
                        throw std::runtime_error("Missing 'v3' in UV for face '" + faceName + "'");
                    if (u4 == uvData.end())
                        throw std::runtime_error("Missing 'u4' in UV for face '" + faceName + "'");
                    if (v4 == uvData.end())
                        throw std::runtime_error("Missing 'v4' in UV for face '" + faceName + "'");
                    
                    if (!u1->second.isNumber() || !v1->second.isNumber() ||
                        !u2->second.isNumber() || !v2->second.isNumber() ||
                        !u3->second.isNumber() || !v3->second.isNumber() ||
                        !u4->second.isNumber() || !v4->second.isNumber())
                    {
                        throw std::runtime_error("All UV coordinates for face '" + faceName + "' must be numbers");
                    }

                    uvs[i][0] = glm::vec2(u1->second.asNumber(), v1->second.asNumber());
                    uvs[i][1] = glm::vec2(u2->second.asNumber(), v2->second.asNumber());
                    uvs[i][2] = glm::vec2(u3->second.asNumber(), v3->second.asNumber());
                    uvs[i][3] = glm::vec2(u4->second.asNumber(), v4->second.asNumber());
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
        Shape::GeometryType geometryType;
        if(dimension == Shape::s_defaultDimensions && 
            position == Shape::s_defaultPosition &&
            rotation == Shape::s_defaultRotations)
            geometryType = Shape::GeometryType::Cube;
        else geometryType = Shape::GeometryType::Generic;

        Shape::registerParallelogram(
            textureIds,
            uvs,
            dimension,
            // position,
            // rotation,
            m_vertexCache,
            m_uvCache,
            m_normalCache,
            m_polygonCache,
            m_coloringCache,
            geometry,
            appearence);
        return geometryType;
    }
}