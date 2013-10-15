#include "Engine.hpp"
#include "Painter.hpp"
#include "Geometry.hpp"
#include "GeometryFormats.hpp"
#include "../inanity/inanity-sqlitefs.hpp"
#include <iostream>

static const float maxAngleChange = 0.1f;

Engine::Engine() :
	cameraAlpha(0),
	cameraBeta(-3.1415926535897932f * 0.25f)
{}

Engine::~Engine()
{
	delete spatialIndex;
}

void Engine::Run()
{
	try
	{
		ptr<Graphics::System> system = Inanity::Platform::Game::CreateDefaultGraphicsSystem();

		ptr<Graphics::Adapter> adapter = system->GetAdapters()[0];
		device = system->CreateDevice(adapter);
		ptr<Graphics::Monitor> monitor = adapter->GetMonitors()[0];

		screenWidth = 1024;
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
			NEW(BlobFileSystem(Platform::FileSystem::GetNativeFileSystem()->LoadFile("data")))
#else
			NEW(BufferedFileSystem(NEW(Platform::FileSystem("res"))))
#endif
		;

		geometryFormats = NEW(GeometryFormats());

		painter = NEW(Painter(device, context, presenter, output, shaderCache, geometryFormats));

		textureManager = NEW(TextureManager(fileSystem, device));
		fontManager = NEW(FontManager(fileSystem, textureManager));
		textDrawer = TextDrawer::Create(device, shaderCache);
		font = fontManager->Get("mnogobukov.font");

		boxGeometry = LoadDebugGeometry("box.geo");

		// spatial index
		//spatialIndex = NEW(Spatial::Quadtree<Firstblood::ISpatiallyIndexable>(5, 512.0f, 32 * 1024));
		spatialIndex = NEW(Spatial::KdTree<Firstblood::ISpatiallyIndexable>(8, 32 * 1024));
		// rvo
		rvoSimulation = NEW(Firstblood::RvoSimulation(512, spatialIndex));
		// scripts
		scripts = NEW(Firstblood::ScriptSystem(painter, rvoSimulation, &cameraViewMatrix, spatialIndex));

		try
		{
			window->Run(Handler::Bind(MakePointer(this), &Engine::Tick));
		}
		catch(Exception* exception)
		{
			THROW_SECONDARY("Error while running game", exception);
		}
		
		scripts->fini();
	}
	catch(Exception* exception)
	{
		THROW_SECONDARY("Can't initialize game", exception);
	}
}

void Engine::Tick()
{
	float frameTime = ticker.Tick();

	ptr<Input::Frame> inputFrame = inputManager->GetCurrentFrame();
	const Input::State& inputState = inputFrame->GetCurrentState();
	scripts->setInputState(&inputState);
	while(inputFrame->NextEvent())
	{
		const Input::Event& inputEvent = inputFrame->GetCurrentEvent();
		scripts->handleInputEvent(inputEvent);
	}

	mat4x4 projMatrix = CreateProjectionPerspectiveFovMatrix(3.1415926535897932f / 4, float(screenWidth) / float(screenHeight), 0.1f, 1000.0f);
	// рисование кадра

	painter->BeginFrame(frameTime);
	Step(frameTime);

	const vec3 sunDirection = normalize(vec3(-1, -1, -1));
	mat4x4 sunTransform =
		CreateProjectionPerspectiveFovMatrix(3.1415926535897932f / 4, 1.0f, 0.1f, 150.0f)
		* CreateLookAtMatrix(sunDirection * -100.0f, vec3(0, 0, 0), vec3(0, 0, 1));

	painter->SetSceneLighting(vec3(1, 1, 1) * 0.1f, vec3(1, 1, 1), sunDirection, sunTransform);
	vec3 translation(cameraViewMatrix(3, 0), cameraViewMatrix(3, 1), cameraViewMatrix(3, 2));
	painter->SetCamera(projMatrix * cameraViewMatrix, translation);
	painter->SetupPostprocess(1.0f, 1.0f, 1.0f);
	painter->Draw();

	Context::LetFrameBuffer lfb(context, presenter->GetFrameBuffer());
	Context::LetViewport lv(context, screenWidth, screenHeight);

	textDrawer->Prepare(context, screenWidth, screenHeight);
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
		textDrawer->DrawTextLine(fpsString, -0.95f - 2.0f / screenWidth, -0.8f - 2.0f / screenHeight, vec4(1, 1, 1, 1), FontAlignments::Left | FontAlignments::Bottom);
		textDrawer->DrawTextLine(fpsString, -0.95f, -0.8f, vec4(1, 0, 0, 1), FontAlignments::Left | FontAlignments::Bottom);
		textDrawer->Flush();
	}

	presenter->Present();
}

ptr<Texture> Engine::LoadTexture(const String& fileName)
{
	return textureManager->Get(fileName);
}

ptr<Geometry> Engine::LoadDebugGeometry(const String& fileName)
{
	return NEW(Geometry(
		device->CreateStaticVertexBuffer(fileSystem->LoadFile(fileName + ".vertices"), geometryFormats->debug.vl),
		device->CreateStaticIndexBuffer(fileSystem->LoadFile(fileName + ".indices"), sizeof(short))
	));
}
