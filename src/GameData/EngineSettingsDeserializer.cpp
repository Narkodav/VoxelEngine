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

GeneratorSettings EngineSettingsDeserializer::parseGenerator(const Json::Value& config) {
    GeneratorSettings generator;
    const auto& generatorSettings = getInner(config, "Config", "Generator");

    generator.generatorId = Modular::serviceNameToId(getString(getInner(generatorSettings, "Generator", "Type")));

    generator.seed = getNumber<uint64_t>(getInner(generatorSettings, "Generator", "Seed"));

    const auto& shape = getInner(generatorSettings, "Generator", "Shape");
    generator.shapeSettings.shape = getEnum(s_shapeStringToEnum, shape);

    const auto& paramsConfig = getInner(generatorSettings, "Generator", "Params");
    
    switch(generator.shapeSettings.shape) {
        case WorldGrid::ShapeToGenerate::Sphere:
            {
                generator.shapeSettings.params = std::make_unique<WorldGrid::SphereShapeParams>();
                auto& params = *static_cast<WorldGrid::SphereShapeParams*>(generator.shapeSettings.params.get());
                params.radius = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Radius"));
                params.centerPosition = getVectorWithUnit<glm::ivec3>(getInner(paramsConfig, "Params", "CenterPosition"));
            }
            break;
        case WorldGrid::ShapeToGenerate::Cylinder:
            {
                generator.shapeSettings.params = std::make_unique<WorldGrid::CylinderShapeParams>();
                auto& params = *static_cast<WorldGrid::CylinderShapeParams*>(generator.shapeSettings.params.get());
                params.radius = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Radius"));
                params.height = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Height"));
                params.bottomCenterPosition = getVectorWithUnit<glm::ivec3>(getInner(paramsConfig, "Params", "BottomCenterPosition"));
            }
            break;
        case WorldGrid::ShapeToGenerate::Parallelepiped:
            {
                generator.shapeSettings.params = std::make_unique<WorldGrid::ParallelepipedShapeParams>();
                auto& params = *static_cast<WorldGrid::ParallelepipedShapeParams*>(generator.shapeSettings.params.get());
                params.width = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Width"));
                params.depth = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Depth"));
                params.height = getNumberWithUnit<size_t>(getInner(paramsConfig, "Params", "Height"));
                params.cornerPosition = getVectorWithUnit<glm::ivec3>(getInner(paramsConfig, "Params", "CornerPosition"));
            }
            break;
        case WorldGrid::ShapeToGenerate::Cube:
            {
                generator.shapeSettings.params = std::make_unique<WorldGrid::CubeShapeParams>();
                auto& params = *static_cast<WorldGrid::CubeShapeParams*>(generator.shapeSettings.params.get());
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