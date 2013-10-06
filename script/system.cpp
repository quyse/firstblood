#include "inanity/FolderFileSystem.hpp"
#include "script/system.hpp"

#define SCRIPTS_FOLDER "res/scripts/"
#define SCRIPTS_ENTRY_FILE "res/scripts/__entry__.js"

namespace Firstblood
{

	ScriptSystem* ScriptSystem::_instance = nullptr;

	ptr<ScriptSystem> ScriptSystem::getInstance() { return _instance; }

	ScriptSystem::ScriptSystem(Painter* painter, ptr<RvoSimulation> rvoSimulation)
	{
		_scriptsVirtualMachine = new Inanity::Script::V8::State();
		_scriptsEntryPoint = _scriptsVirtualMachine->LoadScript(FolderFileSystem::GetNativeFileSystem()->LoadFile(SCRIPTS_ENTRY_FILE));
		
		// register global game objects
		_logger = NEW(ScriptLogger());
		_scriptsVirtualMachine->Register<ScriptLogger>();
		_painter = NEW(ScriptPainter(painter));
		_scriptsVirtualMachine->Register<ScriptPainter>();
		_rvoSimulation = rvoSimulation;
		_scriptsVirtualMachine->Register<RvoSimulation>();

		// register self for script access
		assert(_instance == nullptr);
		_instance = this;
		_scriptsVirtualMachine->Register<ScriptSystem>();
	}

	void ScriptSystem::fini()
	{
		_scriptsVirtualMachine->ReclaimInstance(this);
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

	void ScriptSystem::require(const Inanity::String& fileName)
	{
		if (_processedScriptSources.find(fileName) == _processedScriptSources.end())
		{
			Inanity::String fullName = SCRIPTS_FOLDER + fileName + ".js";
			ptr<File> scriptFile = FolderFileSystem::GetNativeFileSystem()->LoadFile(fullName);
			_processedScriptSources.insert(fileName);
			_scriptsVirtualMachine->LoadScript(scriptFile)->Run();
		}
	}

	void ScriptSystem::update(float dt)
	{
		_scriptsEntryPoint->Run();
	}

}