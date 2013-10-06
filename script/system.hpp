#ifndef __FB_SCRIPT_SYSTEM__
#define __FB_SCRIPT_SYSTEM__

#include <unordered_set>
#include "inanity/script/State.hpp"
#include "inanity/script/Function.hpp"
#include "inanity/File.hpp"
#include "script/utils.hpp"
#include "script/time.hpp"
#include "gamelogic/rvo.hpp"
#include "Painter.hpp"

namespace Firstblood
{

	class ScriptSystem : public Inanity::Object
	{
	public:
		ScriptSystem(Painter* painter, ptr<RvoSimulation>);
		~ScriptSystem();
		void fini();

		static ptr<ScriptSystem> getInstance();

		void update(float dt);
		void removeFromScript(Inanity::RefCounted* object);

		// script getters
		ptr<ScriptLogger> getLogger();
		ptr<ScriptPainter> getPainter();
		ptr<ScriptTime> getTime();
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
		ptr<RvoSimulation> _rvoSimulation;

		// processed script files
		std::unordered_set<Inanity::String> _processedScriptSources;
	};

}


#endif