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

	MT::ThreadPool pool(16);
	Window window = Window(Graphics::Extent2D(800, 600), "app",
		Window::Attributes::firstPersonGameMaximisedAtr());
	Renderer renderer;
	renderer.init("eng", "app", window, pool);

	ResourceCache resources;
	resources.registerResources("res/resourcePack/voxels");
	resources.getAssetCache().printStatistics();

	FrameRateCalculator calculator;
	calculator.setFrameTimeBuffer(100);

	Graphics::CameraPerspective camera = Graphics::CameraPerspective(
		glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-1.0f, -1.0f, -1.0f),
		0.0f, 0.0f, 100.0f,
		window.getAspectRatio(), 0.1f, 100000.0f);

	float deltaTime = 0.0f;

	Keyboard keyboard = Keyboard(window);
	Mouse mouse = Mouse(window);

	WorldGrid grid;
	grid.resetSphereRadius(10, { 0, 0, 0 });

	float mouseSensitivity = 80.f;
	float moveSpeed = 10.f;

	renderer.createAndWriteAssets(resources.getAssetCache(), resources.getVoxelStateCache());
	renderer.resetChunkBuffers(grid.getChunks().size());

	for (size_t i = 0; i < grid.getGrid().size(); ++i)
		grid.getBlock(i) = 1;

	for(size_t i = 0; i < grid.getChunks().size(); ++i)
		renderer.updateChunkAsync(resources, i, grid.getChunks()[i], grid.getGrid());

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

		//std::copy(gridVec.begin(), gridVec.end(), gridMapping.begin());
		//std::copy(chunkVec.begin(), chunkVec.end(), chunkMapping.begin());
		
		//game loop


		renderer.drawFrame(camera);

		auto currentTime = std::chrono::high_resolution_clock::now();
		deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		calculator.addFrameTime(deltaTime);
		calculator.updateFrameRate();
		//calculator.displayFrameRateToConsole();
	}

	renderer.cleanup();
	window.destroy();

}
