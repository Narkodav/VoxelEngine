#include "Graphics/Graphics.h"

#include "Rendering/Renderer.h"

#include "GameData/ResourceCache.h"
#include "GameData/EngineFilesystem.h"

#include "WorldManagement/WorldGrid.h"
#include "WorldManagement/Generator.h"

#include "PlatformAbstractions/Console.h"

glm::vec3 moveDir(0.0f);
bool CkeyPressed = false;
bool cursorMode = true;
Graphics::Utility::CameraPerspective camera;
Platform::Window window;

template<typename T>
T getNumber(const Json::Value& value) {
    if (value.isInteger()) return value.asInteger(); 
    else if (value.isNumber()) return value.asNumber();
    else throw std::runtime_error("value must contain a number");
}

template<typename T>
T getVector(const Json::Value& value) {
	T vec;
	if(value.isObject()) {
		vec.x = getNumber<decltype(vec.x)>(value.asObject().at("X"));
		vec.y = getNumber<decltype(vec.y)>(value.asObject().at("Y"));
		vec.z = getNumber<decltype(vec.z)>(value.asObject().at("Z"));
	} else if(value.isArray()) {
		vec.x = getNumber<decltype(vec.x)>(value.asArray()[0]);
		vec.y = getNumber<decltype(vec.y)>(value.asArray()[1]);
		vec.z = getNumber<decltype(vec.z)>(value.asArray()[2]);
	} else throw std::runtime_error("Cannot parse vector that is not object or array");
	return vec;
}

void handleInputs(Platform::Window& window, Graphics::Utility::CameraPerspective& camera,
	float deltaTime, float sensitivity, float moveVelocity, float speedMoveVelocity)
{
	if (cursorMode)
	{
        auto& mouse = window.getMouse();
		auto& keyboard = window.getKeyboard();

		if (mouse.moved() && cursorMode) {
			auto deltaPos = mouse.getMouseDeltaPos();
			camera.rotate(-deltaPos.y * deltaTime * sensitivity, deltaPos.x * deltaTime * sensitivity);
		}

		if (keyboard.keyPressed<Platform::KeyboardKey::W>()) {
			moveDir += camera.getCamForward();
		}
		if (keyboard.keyPressed<Platform::KeyboardKey::S>()) {
			moveDir -= camera.getCamForward();
		}
		if (keyboard.keyPressed<Platform::KeyboardKey::A>()) {
			moveDir -= camera.getCamRight();
		}
		if (keyboard.keyPressed<Platform::KeyboardKey::D>()) {
			moveDir += camera.getCamRight();
		}
		if (keyboard.keyPressed<Platform::KeyboardKey::Space>()) {
			moveDir += camera.getWorldUp();
		}
		if (keyboard.keyPressed<Platform::KeyboardKey::LeftShift>()) {
			moveDir -= camera.getWorldUp();
		}

		if (glm::length(moveDir) > std::numeric_limits<float>::epsilon()) {
			moveDir = glm::normalize(moveDir);
			if(mouse.buttonPressed<Platform::MouseButton::Rmb>())
				camera.move(moveDir * deltaTime * speedMoveVelocity);
			else camera.move(moveDir * deltaTime * moveVelocity);
			moveDir = glm::vec3(0);		
		}
	}
}

void keyPressed(Platform::KeyboardKey key) {
    switch(key) {
        case Platform::KeyboardKey::C:
            if (cursorMode)
			{
				window.setCursorMode(Platform::CursorMode::Normal);
			}
			else
			{
				window.centerCursor();
				window.setCursorMode(Platform::CursorMode::Disabled);
			}
            cursorMode = !cursorMode;
            break;
        default: break;
    }
}

int main()
{
	std::cout << std::endl;
	MT::ThreadPool pool;
	pool.init(16);

	EngineFilesystem engineFiles;
	engineFiles.init();
	engineFiles.printDirectories();
	engineFiles.printPaths();

	auto renderConfigPath = engineFiles.getRootDirectory() / "EngineSettings.json";
	Json::Value config = Json::Value::fromFile(renderConfigPath.string()).front();

	cursorMode = 1;
	Generator generator;
	WorldGrid grid;
	
	auto& generatorSettings = config.asObject().at("Generator").asObject();
	if(generatorSettings.at("Type") == "Cube") {
		auto edge = generatorSettings.at("Edge").asInteger();
		glm::ivec3 cornerPos = getVector<glm::ivec3>(generatorSettings.at("CornerPostition"));
		grid.generateCube(edge, cornerPos);
	}
	else if(generatorSettings.at("Type") == "Parallelogram") {
		auto width = generatorSettings.at("Width").asInteger();
		auto height = generatorSettings.at("Height").asInteger();
		auto depth = generatorSettings.at("Depth").asInteger();
		glm::ivec3 cornerPos = getVector<glm::ivec3>(generatorSettings.at("CornerPostition"));
		grid.generateParallelogram(width, height, depth, cornerPos);
	}
	else if(generatorSettings.at("Type") == "Sphere") {
		auto radius = generatorSettings.at("Radius").asInteger();
		glm::ivec3 centerPos = getVector<glm::ivec3>(generatorSettings.at("CenterPostition"));
		grid.generateSphere(radius, centerPos);
	}
	else if(generatorSettings.at("Type") == "Cylinder") {
		auto radius = generatorSettings.at("Radius").asInteger();
		auto height = generatorSettings.at("Height").asInteger();
		glm::ivec3 bottomCenterPostition = getVector<glm::ivec3>(generatorSettings.at("BottomCenterPostition"));
		grid.generateCylinder(radius, height, bottomCenterPostition);
	}
	else throw std::runtime_error("Shape not implemented");
	
	auto& cameraSettings = config.asObject().at("CameraSettings").asObject();
	glm::vec3 worldUpVector = getVector<glm::vec3>(cameraSettings.at("WorldUpVector"));
	glm::vec3 position = getVector<glm::vec3>(cameraSettings.at("Position"));
	float pitch = cameraSettings.at("Pitch").asNumber();
	float yaw = cameraSettings.at("Yaw").asNumber();
	float fov = cameraSettings.at("Fov").asNumber();

	Graphics::Utility::CameraPerspective camera = Graphics::Utility::CameraPerspective(
		worldUpVector, position, pitch, yaw, fov, 800.f / 600.f, 0.1f, 100000.0f);
	
	generator.set(1234);
    
    window.create({ 800, 600 }, "app", Platform::WindowAttributes::firstPersonGameMaximisedAtr());

    window.registerCallback<Platform::IOEvents::KeyPressed>(keyPressed);
    //window.registerCallback<Platform::IOEvents::KeyReleased>(keyReleased);

	window.registerCallback<Platform::IOEvents::MouseMovedScreen>([&](Platform::Position){
		if(cursorMode) window.centerCursor();
	});

	Renderer renderer;
	renderer.init("eng", "app", window, pool, engineFiles);

	ResourceCache resources;
	resources.registerResources(engineFiles);
	resources.getAssetCache().printStatistics();

	float deltaTime = 0.0f;

	float mouseSensitivity = config.asObject().at("MouseSensitivity").asNumber();
	float moveVelocity = config.asObject().at("MoveVelocity").asNumber();
	float speedMoveVelocity = config.asObject().at("SpeedMoveVelocity").asNumber();

	renderer.createAndWriteAssets(resources.getAssetCache(), resources.getVoxelStateCache());
	renderer.resetChunkBuffers(grid);

	grid.sortAllocationsByDistance(glm::ivec3(
		camera.getPosition().x / Constants::chunkWidth,
		camera.getPosition().y / Constants::chunkHeight,
		camera.getPosition().z / Constants::chunkDepth));
	for (size_t i = 0; i < grid.getAllocatedChunks().size(); ++i)
		pool.pushTask([&generator, &grid, i]() {
			//generator.fillChunk(grid, i, Generator::BlockTypes::Dirt);
			generator.setChunkData(grid, i);
		});
	pool.pausePool();
	renderer.dumpHandles();
	
	for (size_t i = 0; i < grid.getAllocatedChunks().size(); ++i)
		renderer.updateChunkAsync(resources, grid.getAllocatedChunks()[i].getIndex(), grid);
	
	while (!window.shouldClose()) {
		auto startTime = std::chrono::high_resolution_clock::now();
		
		try {
			window.pollEvents();
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		
		camera.setAspectRatio(window.getAspectRatio());
		
		handleInputs(window, camera, deltaTime, mouseSensitivity, moveVelocity, speedMoveVelocity);
		
		renderer.drawFrame(camera);
		
		auto currentTime = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	}
	pool.terminate();
	renderer.cleanup(resources.getAssetCache().getStorageCache());
	window.destroy();
}