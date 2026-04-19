#include "Graphics/Graphics.h"

#include "Rendering/Renderer.h"

#include "GameData/ResourceCache.h"
#include "GameData/EngineFilesystem.h"
#include "GameData/EngineSettingsDeserializer.h"

#include "WorldManagement/WorldGrid.h"
#include "WorldManagement/Generator.h"

#include "CommonApi/PlatformAbstractions/Console.h"

glm::vec3 moveDir(0.0f);
bool CkeyPressed = false;
bool cursorMode = true;
Graphics::Utility::CameraPerspective camera;
Platform::Window window;
Generator generator;
WorldGrid grid;
Renderer renderer;
ResourceCache resources;

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

		// if (keyboard.keyPressed<Platform::KeyboardKey::LeftControl>() || keyboard.keyPressed<Platform::KeyboardKey::RightControl>() &&
		// 	keyboard.keyJustPressed<Platform::KeyboardKey::R>()) {
			
		// }

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
	MT::MinimalThreadPool pool;
	pool.init(16, std::cerr);

	EngineFilesystem engineFiles;
	engineFiles.init();
	engineFiles.printDirectories();
	engineFiles.printPaths();

	Json::Value settings = Json::Value::fromFile(engineFiles.getSettingsFile().string()).front();

	cursorMode = 1;	
	grid.generate(EngineSettingsDeserializer::parseGenerator(settings));
	auto graphicsSettings = EngineSettingsDeserializer::parseGraphics(settings);

	Graphics::Utility::CameraPerspective camera = Graphics::Utility::CameraPerspective(
		graphicsSettings.camera.upVector, graphicsSettings.camera.position, 
		graphicsSettings.camera.pitch, graphicsSettings.camera.yaw, graphicsSettings.camera.fov, 
		800.f / 600.f, 0.1f, 100000.0f);
	
	generator.set(1234);
    
    window.create({ 800, 600 }, "app", Platform::WindowAttributes::firstPersonGameMaximisedAtr());

    window.registerCallback<Platform::IOEvents::KeyPressed>(keyPressed);
    //window.registerCallback<Platform::IOEvents::KeyReleased>(keyReleased);

	window.registerCallback<Platform::IOEvents::MouseMovedScreen>([&](Platform::Position){
		if(cursorMode) window.centerCursor();
	});

	renderer.init("eng", "app", window, pool, engineFiles);
	renderer.setContrast(graphicsSettings.contrast);

	resources.registerResources(engineFiles);
	resources.getAssetCache().printStatistics();

	float deltaTime = 0.0f;

	auto movementSettings = EngineSettingsDeserializer::parseMovement(settings);
	auto inputSettings = EngineSettingsDeserializer::parseInput(settings);

	renderer.createAndWriteAssets(resources.getAssetCache(), resources.getVoxelStateCache());
	renderer.resetChunkBuffers(grid);
	renderer.dumpHandles();

	grid.sortAllocationsByDistance(glm::ivec3(
		camera.getPosition().x / Constants::chunkWidth,
		camera.getPosition().y / Constants::chunkHeight,
		camera.getPosition().z / Constants::chunkDepth));
	for (size_t i = 0; i < grid.getAllocatedChunks().size(); ++i)
		pool.pushTask([i]() {
			//generator.fillChunk(grid, i, Generator::BlockTypes::Dirt);
			generator.setChunkData(grid, i);
			renderer.updateChunkAsync(resources, grid.getAllocatedChunks()[i].getIndex(), grid);
		});
	//pool.waitIdle();
	
	
	// for (size_t i = 0; i < grid.getAllocatedChunks().size(); ++i)
	// 	renderer.updateChunkAsync(resources, grid.getAllocatedChunks()[i].getIndex(), grid);
	
	while (!window.shouldClose()) {
		auto startTime = std::chrono::high_resolution_clock::now();
		
		try {
			window.pollEvents();
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		
		camera.setAspectRatio(window.getAspectRatio());
		
		handleInputs(window, camera, deltaTime, inputSettings.mouseSensitivity, movementSettings.moveVelocity, movementSettings.fastMoveVelocity);
		
		renderer.drawFrame(camera);
		
		auto currentTime = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
	}
	pool.destroy(pool.wait());
	renderer.cleanup(resources.getAssetCache().getStorageCache());
	window.destroy();
}