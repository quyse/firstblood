#include "Game.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"
#include "GeometryFormats.hpp"
#include "../inanity/inanity-sqlitefs.hpp"
#include <iostream>

static const float maxAngleChange = 0.1f;

Game::Game() :
	cameraAlpha(0),
	cameraBeta(0)
{}

void Game::Run()
{
	try
	{
		ptr<Graphics::System> system = Inanity::Platform::Game::CreateDefaultGraphicsSystem();

		ptr<Graphics::Adapter> adapter = system->GetAdapters()[0];
		device = system->CreateDevice(adapter);
		ptr<Graphics::Monitor> monitor = adapter->GetMonitors()[0];

		screenWidth = 800;
		screenHeight = 600;
		bool fullscreen = false;

		ptr<Platform::Window> window = monitor->CreateDefaultWindow(
			"First Blood", screenWidth, screenHeight);
		this->window = window;

		inputManager = Inanity::Platform::Game::CreateInputManager(window);

		ptr<Graphics::MonitorMode> monitorMode;
		if(fullscreen)
			monitorMode = monitor->TryCreateMode(screenWidth, screenHeight);
		ptr<Output> output = window->CreateOutput();
		presenter = device->CreatePresenter(output, monitorMode);

		context = system->CreateContext(device);

		const char* shadersCacheFileName =
#ifdef _DEBUG
			"shaders_debug"
#else
			"shaders"
#endif
			;
		ptr<ShaderCache> shaderCache = NEW(ShaderCache(NEW(SQLiteFileSystem(shadersCacheFileName)), device,
			system->CreateShaderCompiler(), system->CreateShaderGenerator(), NEW(Crypto::WhirlpoolStream())));

		fileSystem =
#ifdef PRODUCTION
			NEW(BlobFileSystem(FolderFileSystem::GetNativeFileSystem()->LoadFile("data")))
#else
			NEW(BufferedFileSystem(NEW(FolderFileSystem("res"))))
#endif
		;

		geometryFormats = NEW(GeometryFormats());

		painter = NEW(Painter(device, context, presenter, output, shaderCache, geometryFormats));

		textureManager = NEW(TextureManager(fileSystem, device));
		fontManager = NEW(FontManager(fileSystem, textureManager));
		textDrawer = TextDrawer::Create(device, shaderCache);
		font = fontManager->Get("mnogobukov.font");

		physicsWorld = NEW(Physics::BtWorld());

		boxGeometry = LoadDebugGeometry("box.geo");

		try
		{
			window->Run(Handler::Bind(MakePointer(this), &Game::Tick));
		}
		catch(Exception* exception)
		{
			THROW_SECONDARY("Error while running game", exception);
		}
	}
	catch(Exception* exception)
	{
		THROW_SECONDARY("Can't initialize game", exception);
	}
}

void Game::Tick()
{
	float frameTime = ticker.Tick();

	ptr<Input::Frame> inputFrame = inputManager->GetCurrentFrame();
	while(inputFrame->NextEvent())
	{
		const Input::Event& inputEvent = inputFrame->GetCurrentEvent();

		//std::cout << inputEvent;

		switch(inputEvent.device)
		{
		case Input::Event::deviceKeyboard:
			if(inputEvent.keyboard.type == Input::Event::Keyboard::typeKeyDown)
			{
				switch(inputEvent.keyboard.key)
				{
				case 27: // escape
					window->Close();
					return;
				case 32:
					//physicsCharacter.FastCast<Physics::BtCharacter>()->GetInternalController()->jump();
					break;
				}
			}
			break;
		case Input::Event::deviceMouse:
			switch(inputEvent.mouse.type)
			{
			case Input::Event::Mouse::typeButtonDown:
				break;
			case Input::Event::Mouse::typeButtonUp:
				break;
			case Input::Event::Mouse::typeMove:
				cameraAlpha -= std::max(std::min(inputEvent.mouse.offsetX * 0.005f, maxAngleChange), -maxAngleChange);
				cameraBeta -= std::max(std::min(inputEvent.mouse.offsetY * 0.005f, maxAngleChange), -maxAngleChange);
				break;
			}
			break;
		}
	}

	cameraBeta = clamp(cameraBeta, -1.5f, 1.5f);

	vec3 cameraDirection = vec3(cos(cameraAlpha) * cos(cameraBeta), sin(cameraAlpha) * cos(cameraBeta), sin(cameraBeta));
	vec3 cameraRightDirection = normalize(cross(cameraDirection, vec3(0, 0, 1)));
	vec3 cameraUpDirection = cross(cameraRightDirection, cameraDirection);

	const Input::State& inputState = inputFrame->GetCurrentState();
	/*
	left up right down Q E
	37 38 39 40
	65 87 68 83 81 69
	*/
	float cameraStep = 5;
	vec3 cameraMove(0, 0, 0);
	vec3 cameraMoveDirectionFront(cos(cameraAlpha), sin(cameraAlpha), 0);
	vec3 cameraMoveDirectionUp(0, 0, 1);
	vec3 cameraMoveDirectionRight = cross(cameraMoveDirectionFront, cameraMoveDirectionUp);
	if(inputState.keyboard[37] || inputState.keyboard[65])
		cameraMove -= cameraMoveDirectionRight * cameraStep;
	if(inputState.keyboard[38] || inputState.keyboard[87])
		cameraMove += cameraMoveDirectionFront * cameraStep;
	if(inputState.keyboard[39] || inputState.keyboard[68])
		cameraMove += cameraMoveDirectionRight * cameraStep;
	if(inputState.keyboard[40] || inputState.keyboard[83])
		cameraMove -= cameraMoveDirectionFront * cameraStep;
	if(inputState.keyboard[81])
		cameraMove -= cameraMoveDirectionUp * cameraStep;
	if(inputState.keyboard[69])
		cameraMove += cameraMoveDirectionUp * cameraStep;

	physicsWorld->Simulate(frameTime);

	static vec3 cameraPosition(0, 0, 0);
	cameraPosition += cameraMove * frameTime;

	context->Reset();

	mat4x4 viewMatrix = CreateLookAtMatrix(cameraPosition, cameraPosition + cameraDirection, vec3(0, 0, 1));
	mat4x4 projMatrix = CreateProjectionPerspectiveFovMatrix(3.1415926535897932f / 4, float(screenWidth) / float(screenHeight), 0.1f, 100.0f);

	// зарегистрировать все объекты
	painter->BeginFrame(frameTime);
	painter->SetCamera(projMatrix * viewMatrix, cameraPosition);
	const vec3 sunDirection = normalize(vec3(-1, -1, -1));
	mat4x4 sunTransform =
		CreateProjectionPerspectiveFovMatrix(3.1415926535897932f / 4, 1.0f, 0.1f, 150.0f)
		* CreateLookAtMatrix(sunDirection * -100.0f, vec3(0, 0, 0), vec3(0, 0, 1));
	painter->SetSceneLighting(vec3(1, 1, 1) * 0.1f, vec3(1, 1, 1), sunDirection, sunTransform);

	painter->AddDebugModel(boxGeometry, CreateTranslationMatrix(vec3(0, 0, -1)) * CreateScalingMatrix(vec3(100, 100, 1)), vec3(1, 1, 1) * 0.5f);
	painter->AddDebugModel(boxGeometry, CreateTranslationMatrix(vec3(1, 1, 1)), vec3(1, 0, 0));

	painter->SetupPostprocess(1.0f, 1.0f, 1.0f);

	painter->Draw();

	textDrawer->Prepare(context);
	textDrawer->SetFont(font);

	// fps
	{
		static int tickCount = 0;
		static const int needTickCount = 100;
		static float allTicksTime = 0;
		allTicksTime += frameTime;
		static float lastAllTicksTime = 0;
		if(++tickCount >= needTickCount)
		{
			lastAllTicksTime = allTicksTime;
			allTicksTime = 0;
			tickCount = 0;
		}
		char fpsString[64];
		sprintf(fpsString, "frameTime: %.6f sec, FPS: %.6f\n", lastAllTicksTime / needTickCount, needTickCount / lastAllTicksTime);
		textDrawer->DrawTextLine(fpsString, -0.95f - 2.0f / context->GetTargetState().viewportWidth, -0.95f - 2.0f / context->GetTargetState().viewportHeight, vec4(1, 1, 1, 1), FontAlignments::Left | FontAlignments::Bottom);
		textDrawer->DrawTextLine(fpsString, -0.95f, -0.95f, vec4(1, 0, 0, 1), FontAlignments::Left | FontAlignments::Bottom);
	}

	textDrawer->Flush();

	context->GetTargetState().renderBuffers[0] = 0;

	presenter->Present();
}

ptr<Texture> Game::LoadTexture(const String& fileName)
{
	return textureManager->Get(fileName);
}

ptr<Geometry> Game::LoadDebugGeometry(const String& fileName)
{
	return NEW(Geometry(
		device->CreateStaticVertexBuffer(fileSystem->LoadFile(fileName + ".vertices"), geometryFormats->debug.vl),
		device->CreateStaticIndexBuffer(fileSystem->LoadFile(fileName + ".indices"), sizeof(short))
	));
}
