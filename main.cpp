#include "Graphics/DeviceCaching/PhysicalDeviceCache.h"
#include "Graphics/Rendering/Device.h"

#include "Rendering/Renderer.h"
#include "Graphics/PlatformManagement/Keyboard.h"
#include "Graphics/PlatformManagement/Mouse.h"

#include "GameData/ResourceCache.h"
#include "WorldManagement/WorldGrid.h"

#include "Multithreading/ThreadPool.h"

void handleInputs(Mouse& mouse, Keyboard& keyboard, Window& window, Graphics::CameraPerspective& camera,
	float deltaTime, float sensitivity, float moveSpeed, bool& cursorMode)
{
	if (keyboard.getInputState<KeyboardKey::C, KeyboardKeyState::Pressed>() && 
		keyboard.getInputState<KeyboardKey::C, KeyboardKeyState::Changed>())
	{
		if (cursorMode)
		{
			window.setAttribute<Window::Attributes::Type::CenterCursor>(false);
			window.setAttribute<Window::Attributes::Type::CursorMode>(Window::CursorMode::Normal);
		}
		else
		{
			window.setAttribute<Window::Attributes::Type::CenterCursor>(true);
			window.setAttribute<Window::Attributes::Type::CursorMode>(Window::CursorMode::Disabled);
		}
		cursorMode = !cursorMode;
	}

	if (cursorMode)
	{
		if (mouse.moved())
		{
			auto deltaPos = mouse.getMouseDeltaPos();
			camera.rotate(-deltaPos.y * deltaTime * sensitivity, deltaPos.x * deltaTime * sensitivity);
		}
		glm::vec3 moveDir(0.0f);
		if (keyboard.getInputState<KeyboardKey::W, KeyboardKeyState::Pressed>())
			moveDir += camera.getCamForward();
		if (keyboard.getInputState<KeyboardKey::S, KeyboardKeyState::Pressed>())
			moveDir -= camera.getCamForward();
		if (keyboard.getInputState<KeyboardKey::A, KeyboardKeyState::Pressed>())
			moveDir -= camera.getCamRight();
		if (keyboard.getInputState<KeyboardKey::D, KeyboardKeyState::Pressed>())
			moveDir += camera.getCamRight();
		if (keyboard.getInputState<KeyboardKey::Space, KeyboardKeyState::Pressed>())
			moveDir += camera.getWorldUp();
		if (keyboard.getInputState<KeyboardKey::LShift, KeyboardKeyState::Pressed>())
			moveDir -= camera.getWorldUp();
		glm::normalize(moveDir);

		if (moveDir.length != 0)
			camera.move(moveDir * deltaTime * moveSpeed);
	}

	keyboard.refreshState();
	mouse.refreshState();
}

int main()
{	
	bool cursorMode = 1;

	WorldGrid grid;
	grid.generateCylinder(10, 16, { 0, 0, 0 });

	MT::ThreadPool pool(16);
	Window window = Window(Graphics::Extent2D(800, 600), "app",
		Window::Attributes::firstPersonGameMaximisedAtr());
	Renderer renderer;
	renderer.init("eng", "app", window, pool);

	ResourceCache resources;
	resources.registerResources("res/resourcePack/voxels", "res/resourcePack/textures", "res/resourcePack/models");
	resources.getAssetCache().printStatistics();

	FrameRateCalculator calculator;
	calculator.setFrameTimeBuffer(100);

	Graphics::CameraPerspective camera = Graphics::CameraPerspective(
		glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 16.0f * 17, 0.0f),
		0.0f, 0.0f, 100.0f,
		window.getAspectRatio(), 0.1f, 100000.0f);

	float deltaTime = 0.0f;

	Keyboard keyboard = Keyboard(window);
	Mouse mouse = Mouse(window);

	float mouseSensitivity = 80.f;
	float moveSpeed = 10.f;

	renderer.createAndWriteAssets(resources.getAssetCache(), resources.getVoxelStateCache());
	renderer.resetChunkBuffers(grid);

	std::srand(/*std::time({})*/ 214314);
	for (size_t i = 0; i < grid.getGrid().size(); ++i)
		grid.getBlock(i) = std::rand() % 101 > 50 ? 0 : 1;;

	for(size_t i = 0; i < grid.getAllocatedChunks().size(); ++i)
		renderer.updateChunkAsync(resources, grid.getAllocatedChunks()[i].getIndex(), grid);
	size_t i = 0;
	size_t counter = 0;

	//renderer.updateChunk(resources, i, grid, 0);
	while (!window.shouldClose()) {
		auto startTime = std::chrono::high_resolution_clock::now();
		
		try {
			window.pollEvents();
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}

		camera.setAspectRatio(window.getAspectRatio());

		handleInputs(mouse, keyboard, window, camera, deltaTime, mouseSensitivity, moveSpeed, cursorMode);
		
		//if(counter == 1000)
		//{
		//	renderer.unmeshChunk(i);
		//	i = (i + 1) % grid.getChunks().size();
		//	renderer.updateChunk(resources, i, grid, 0);
		//}
		//counter = (counter + 1) % 1001;
		//renderer.updateChunk(resources, i, grid, 0);
		//i = (i + 1) % grid.getChunks().size();
		//game loop


		renderer.drawFrame(camera);

		auto currentTime = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		calculator.addFrameTime(deltaTime);
		calculator.updateFrameRate();
		//calculator.displayFrameRateToConsole();
	}
	pool.terminate();
	renderer.cleanup(resources.getAssetCache().getStorageCache());
	window.destroy();
}
