#include "Graphics/DeviceCaching/PhysicalDeviceCache.h"
#include "Graphics/Rendering/Device.h"

#include "Rendering/Renderer.h"
#include "Graphics/PlatformManagement/Keyboard.h"
#include "Graphics/PlatformManagement/Mouse.h"

#include "GameData/ResourceCache.h"

void handleInputs(Mouse& mouse, Keyboard& keyboard, Graphics::CameraPerspective& camera,
	float deltaTime, float sensitivity, float moveSpeed)
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

	keyboard.refreshState();
	mouse.refreshState();
}

int main()
{	
	Window window = Window(Graphics::Extent2D(800, 600), "app",
		Window::Attributes::firstPersonGameMinimisedAtr());
	Renderer renderer;
	renderer.init("eng", "app", window);

	ResourceCache resources;
	resources.registerResources("res/resourcePack/voxels");
	resources.getAssetCache().printStatistics();

	FrameRateCalculator calculator;
	calculator.setFrameTimeBuffer(100);

	Graphics::CameraPerspective camera = Graphics::CameraPerspective(
		glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(-2.0f, -2.0f, -2.0f),
		0.0f, 0.0f, 100.0f,
		window.getAspectRatio(), 0.1f, 100.0f);

	float deltaTime = 0.0f;

	Keyboard keyboard = Keyboard(window);
	Mouse mouse = Mouse(window);

	float mouseSensitivity = 80.f;
	float moveSpeed = 10.f;

	renderer.createAndWriteAssets(resources.getAssetCache(), resources.getVoxelStateCache());
	auto dataMapping = renderer.getChunkDataMapping();
	auto coordMapping = renderer.getChunkCoordMapping();

	for (size_t i = 0; i < Constants::chunkSize; ++i)
		dataMapping[i] = resources.getVoxelStateCache().data().size() - 1;

	coordMapping[0] = glm::vec4(0, 0, 0, 1);

	while (!window.shouldClose()) {
		auto startTime = std::chrono::high_resolution_clock::now();
		
		try {
			window.pollEvents();
		}
		catch (const std::exception& e) {
			std::cerr << e.what() << std::endl;
		}

		camera.setAspectRatio(window.getAspectRatio());

		handleInputs(mouse, keyboard, camera, deltaTime, mouseSensitivity, moveSpeed);
		renderer.updatePerFrameUniform(camera);

		
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
