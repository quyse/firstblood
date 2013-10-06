#include "inanity/FolderFileSystem.hpp"
#include "script/system.hpp"
#include "script/math.hpp"

namespace Firstblood
{

	ScriptSystem* ScriptSystem::_instance = nullptr;

	ptr<ScriptSystem> ScriptSystem::getInstance() { return _instance; }

	ScriptSystem::ScriptSystem(Painter* painter, ptr<RvoSimulation> rvoSimulation)
	{
		_scriptsVirtualMachine = new Inanity::Script::V8::State();
		_scriptsEntryPoint = _scriptsVirtualMachine->LoadScript(FolderFileSystem::GetNativeFileSystem()->LoadFile("res/scripts/main.js"));
		
		// register global game objects
		_logger = new ScriptLogger();
		_scriptsVirtualMachine->Register<ScriptLogger>();
		_painter = new ScriptPainter(painter);
		_scriptsVirtualMachine->Register<ScriptPainter>();
		_rvoSimulation = rvoSimulation;
		_scriptsVirtualMachine->Register<RvoSimulation>();

		// register self for script access
		assert(_instance == nullptr);
		_instance = this;
		_scriptsVirtualMachine->Register<ScriptSystem>();
	}

	ScriptSystem::~ScriptSystem()
	{
		_scriptsEntryPoint = nullptr;
		_scriptsVirtualMachine = nullptr;
		_logger = nullptr;
	}

	ptr<ScriptLogger> ScriptSystem::getLogger()
	{
		return _logger;
	}

	ptr<ScriptPainter> ScriptSystem::getPainter()
	{
		return _painter;
	}

	ptr<RvoSimulation> ScriptSystem::getRvoSimulation()
	{
		return _rvoSimulation;
	}

	void ScriptSystem::update(float dt)
	{
		_scriptsEntryPoint->Run();
	}

}