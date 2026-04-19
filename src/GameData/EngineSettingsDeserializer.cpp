#include "GameData/EngineSettingsDeserializer.h"

InputSettings EngineSettingsDeserializer::parseInput(const Json::Value& config) {

    InputSettings input;
    const auto& inputSettings = getInner(config, "Config", "Input");
    input.mouseSensitivity = getNumber<float>(getInner(inputSettings, "Input", "MouseSensitivity"));
    return input;
}

MovementSettings EngineSettingsDeserializer::parseMovement(const Json::Value& config) {
    MovementSettings movement;
    const auto& movementSettings = getInner(config, "Config", "Movement");
    movement.moveVelocity = getNumberWithUnit<float>(getInner(movementSettings, "Movement", "MoveVelocity"));
    movement.fastMoveVelocity = getNumberWithUnit<float>(getInner(movementSettings, "Movement", "FastMoveVelocity"));
    return movement;
}

WorldGrid::GeneratorSettings EngineSettingsDeserializer::parseGenerator(const Json::Value& config) {
    WorldGrid::GeneratorSettings generator;
    const auto& generatorSettings = getInner(config, "Config", "Generator");
    const auto& shape = getInner(generatorSettings, "Generator", "Shape");
    if(!shape.isString()) throw std::runtime_error("Shape must be a string");
    generator.shape = getEnum(s_shapeStringToEnum, shape.asString());

    const auto& paramsConfig = getInner(generatorSettings, "Generator", "Params");
    
    switch(generator.shape) {
        case WorldGrid::ShapeToGenerate::Sphere:
            {
                generator.params = std::make_unique<WorldGrid::SphereGeneratorParams>();
                auto& params = *static_cast<WorldGrid::SphereGeneratorParams*>(generator.params.get());
                params.radius = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Radius"));
                params.centerPosition = getVectorWithUnit<glm::ivec3>(getInner(paramsConfig, "Params", "CenterPosition"));
            }
            break;
        case WorldGrid::ShapeToGenerate::Cylinder:
            {
                generator.params = std::make_unique<WorldGrid::CylinderGeneratorParams>();
                auto& params = *static_cast<WorldGrid::CylinderGeneratorParams*>(generator.params.get());
                params.radius = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Radius"));
                params.height = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Height"));
                params.bottomCenterPosition = getVectorWithUnit<glm::ivec3>(getInner(paramsConfig, "Params", "BottomCenterPosition"));
            }
            break;
        case WorldGrid::ShapeToGenerate::Parallelepiped:
            {
                generator.params = std::make_unique<WorldGrid::ParallelepipedGeneratorParams>();
                auto& params = *static_cast<WorldGrid::ParallelepipedGeneratorParams*>(generator.params.get());
                params.width = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Width"));
                params.depth = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Depth"));
                params.height = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Height"));
                params.cornerPosition = getVectorWithUnit<glm::ivec3>(getInner(paramsConfig, "Params", "CornerPosition"));
            }
            break;
        case WorldGrid::ShapeToGenerate::Cube:
            {
                generator.params = std::make_unique<WorldGrid::CubeGeneratorParams>();
                auto& params = *static_cast<WorldGrid::CubeGeneratorParams*>(generator.params.get());
                params.edge = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Edge"));
                params.cornerPosition = getVectorWithUnit<glm::ivec3>(getInner(paramsConfig, "Params", "CornerPosition"));
            }
            break;
        default: throw std::runtime_error("Shape not implemented");
    }

    return generator;
}

Renderer::GraphicsSettings EngineSettingsDeserializer::parseGraphics(const Json::Value& config) {
    Renderer::GraphicsSettings graphics;
    const auto& graphicsSettings = getInner(config, "Config", "Graphics");
    const auto& camera = getInner(graphicsSettings, "Graphics", "Camera");
    graphics.contrast = getNumber<float>(getInner(graphicsSettings, "Graphics", "Contrast"));

    graphics.camera.upVector = getVector<glm::vec3>(getInner(camera, "Camera", "WorldUpVector"));
    graphics.camera.position = getVectorWithUnit<glm::vec3>(getInner(camera, "Camera", "Position"));
    graphics.camera.pitch = getNumberWithUnit<float>(getInner(camera, "Camera", "Pitch"));
    graphics.camera.yaw = getNumberWithUnit<float>(getInner(camera, "Camera", "Yaw"));
    graphics.camera.fov = getNumberWithUnit<float>(getInner(camera, "Camera", "Fov"));

    return graphics;
}