#ifndef ___FIRSTBLOOD_ENGINE_HPP___
#define ___FIRSTBLOOD_ENGINE_HPP___

#include "general.hpp"
#include "spatial/quadtree.hpp"
#include "spatial/kd_tree.hpp"
#include "rvo/simulator.hpp"
#include "gamelogic/common.hpp"
#include "gamelogic/rvo.hpp"
#include "script/system.hpp"

class Geometry;
class GeometryFormats;
class Painter;

class Engine : public Object
{
protected:
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

	// spatial index
	Spatial::IIndex2D<Firstblood::ISpatiallyIndexable>* spatialIndex;
	// rvo
	ptr<Firstblood::RvoSimulation> rvoSimulation;
	// scripts
	ptr<Firstblood::ScriptSystem> scripts;

	int screenWidth, screenHeight;
	float cameraAlpha, cameraBeta;

	Ticker ticker;

	ptr<Geometry> boxGeometry;

	virtual void Step(float frameTime) = 0;

public:
	Engine();
	~Engine();

	void Run();
	void Tick();

	ptr<Texture> LoadTexture(const String& fileName);
	ptr<Geometry> LoadDebugGeometry(const String& fileName);
};

#endif
