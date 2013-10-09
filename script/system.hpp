#ifndef __FB_SCRIPT_SYSTEM__
#define __FB_SCRIPT_SYSTEM__

#include <unordered_set>
#include "inanity/inanity-input.hpp"
#include "inanity/math/basic.hpp"
#include "inanity/script/State.hpp"
#include "inanity/script/Function.hpp"
#include "inanity/File.hpp"
#include "script/utils.hpp"
#include "script/time.hpp"
#include "script/camera.hpp"
#include "script/input.hpp"
#include "script/spatial.hpp"
#include "gamelogic/rvo.hpp"
#include "Painter.hpp"

namespace Firstblood
{

	class ScriptSystem : public Inanity::Object
	{
	public:
		ScriptSystem(Painter* painter, ptr<RvoSimulation>, mat4x4* cameraViewMatrix, Spatial::IIndex2D<ISpatiallyIndexable>* spatialIndex);
		~ScriptSystem();
		void fini();

		static ptr<ScriptSystem> getInstance();

		void update(float dt);
		bool handleInputEvent(const Inanity::Input::Event& event);
		void setInputState(const Inanity::Input::State* state);

		void removeFromScript(Inanity::RefCounted* object);
		ptr<Inanity::Script::Any> createScriptArray(size_t size);
		ptr<Inanity::Script::Any> createScriptInteger(long integer);
		ptr<Inanity::Script::Any> createScriptFloat(float number);
		ptr<Inanity::Script::Any> createScriptBoolean(bool boolean);

		// script getters
		ptr<ScriptLogger> getLogger();
		ptr<ScriptPainter> getPainter();
		ptr<ScriptTime> getTime();
		ptr<ScriptCamera> getCamera();
		ptr<ScriptInput> getInput();
		ptr<ScriptSpatialIndex> getSpatialIndex();
		ptr<RvoSimulation> getRvoSimulation();
		
		// primitive analogue of python's import statement
		void require(const Inanity::String& file);
		
	public:
		static ScriptSystem* _instance;

	private:
		// virtual machine
		ptr<Inanity::Script::State> _scriptsVirtualMachine;
		ptr<Inanity::Script::Function> _scriptsEntryPoint;

		// global js objects
		ptr<ScriptLogger> _logger;
		ptr<ScriptPainter> _painter;
		ptr<ScriptTime> _time;
		ptr<ScriptCamera> _camera;
		ptr<ScriptInput> _input;
		ptr<ScriptSpatialIndex> _spatialIndex;
		ptr<RvoSimulation> _rvoSimulation;
		
		// processed script files
		std::unordered_set<Inanity::String> _processedScriptSources;
	};

}


#endif