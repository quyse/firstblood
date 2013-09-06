#ifndef ___FIRSTBLOOD_GAME_HPP___
#define ___FIRSTBLOOD_GAME_HPP___

#include "general.hpp"

class Geometry;
class GeometryFormats;
class Painter;

/// Класс игры.
class Game : public Object
{
private:
	ptr<Platform::Window> window;
	ptr<Device> device;
	ptr<Context> context;
	ptr<Presenter> presenter;

	ptr<GeometryFormats> geometryFormats;

	ptr<Painter> painter;

	ptr<FileSystem> fileSystem;

	ptr<Input::Manager> inputManager;

	ptr<TextureManager> textureManager;
	ptr<FontManager> fontManager;
	ptr<TextDrawer> textDrawer;
	ptr<Font> font;

	int screenWidth, screenHeight;
	float cameraAlpha, cameraBeta;

	Ticker ticker;

	ptr<Physics::World> physicsWorld;

	ptr<Geometry> boxGeometry;

public:
	Game();

	void Run();
	void Tick();

	ptr<Texture> LoadTexture(const String& fileName);
	ptr<Geometry> LoadDebugGeometry(const String& fileName);
};

#endif
