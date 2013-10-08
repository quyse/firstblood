#include "inanity/platform/FileSystem.hpp"
#include "inanity/script/Any.hpp"
#include "script/system.hpp"

#define SCRIPTS_FOLDER "res/scripts/"
#define SCRIPTS_ENTRY_FILE "res/scripts/__entry__.js"

namespace Firstblood
{

	ScriptSystem* ScriptSystem::_instance = nullptr;

	ptr<ScriptSystem> ScriptSystem::getInstance() { return _instance; }

	ScriptSystem::ScriptSystem(Painter* painter, ptr<RvoSimulation> rvoSimulation, mat4x4* cameraViewMatrix)
	{
		ptr<Inanity::Script::V8::State> v8State = new Inanity::Script::V8::State();
		_scriptsVirtualMachine = v8State;
		_scriptsEntryPoint = _scriptsVirtualMachine->LoadScript(Platform::FileSystem::GetNativeFileSystem()->LoadFile(SCRIPTS_ENTRY_FILE));
		
		// register global game objects
		_logger = NEW(ScriptLogger());
		v8State->Register<ScriptLogger>();
		_painter = NEW(ScriptPainter(painter));
		v8State->Register<ScriptPainter>();
		_time = NEW(ScriptTime());
		v8State->Register<ScriptTime>();
		_camera = NEW(ScriptCamera(cameraViewMatrix));
		v8State->Register<ScriptCamera>();
		_input = NEW(ScriptInput());
		v8State->Register<ScriptInput>();
		_rvoSimulation = rvoSimulation;
		v8State->Register<RvoSimulation>();

		// register self for script access
		assert(_instance == nullptr);
		_instance = this;
		v8State->Register<ScriptSystem>();
	}

	ScriptSystem::~ScriptSystem()
	{
		_logger = nullptr;
		_time->fini();
		_time = nullptr;
		_input->fini();
		_input = nullptr;
		_camera = nullptr;
		_painter = nullptr;
		_rvoSimulation = nullptr;
		_scriptsEntryPoint = nullptr;
		_scriptsVirtualMachine = nullptr;
	}

	void ScriptSystem::fini()
	{
		_scriptsVirtualMachine->ReclaimInstance(this);
	}

	ptr<ScriptLogger> ScriptSystem::getLogger()
	{
		return _logger;
	}

	ptr<ScriptPainter> ScriptSystem::getPainter()
	{
		return _painter;
	}

	ptr<ScriptTime> ScriptSystem::getTime()
	{
		return _time;
	}

	ptr<ScriptCamera> ScriptSystem::getCamera()
	{
		return _camera;
	}

	ptr<ScriptInput> ScriptSystem::getInput()
	{
		return _input;
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
			ptr<File> scriptFile = Platform::FileSystem::GetNativeFileSystem()->LoadFile(fullName);
			_processedScriptSources.insert(fileName);
			_scriptsVirtualMachine->LoadScript(scriptFile)->Run();
		}
	}

	void ScriptSystem::update(float dt)
	{
		_time->update();
		_scriptsEntryPoint->Run();
		_input->update();
	}

	bool ScriptSystem::handleInputEvent(const Inanity::Input::Event& event)
	{
		return _input->handleEvent(event);
	}

	void ScriptSystem::setInputState(const Inanity::Input::State* state)
	{
		_input->setState(state);
	}

	void ScriptSystem::removeFromScript(Inanity::RefCounted* object)
	{
		_scriptsVirtualMachine->ReclaimInstance(object);
	}

	ptr<Inanity::Script::Any> ScriptSystem::createScriptArray(size_t size)
	{
		return _scriptsVirtualMachine->NewArray(size);
	}

	ptr<Inanity::Script::Any> ScriptSystem::createScriptInteger(long integer)
	{
		return _scriptsVirtualMachine->NewInteger(integer);
	}

	ptr<Inanity::Script::Any> ScriptSystem::createScriptFloat(float number)
	{
		return _scriptsVirtualMachine->NewNumber(number);
	}

	ptr<Inanity::Script::Any> ScriptSystem::createScriptBoolean(bool boolean)
	{
		return _scriptsVirtualMachine->NewBoolean(boolean);
	}

}