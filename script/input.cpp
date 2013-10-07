#include "script/input.hpp"
#include "script/system.hpp"

namespace Firstblood
{

	void ScriptInput::fini()
	{
		for (size_t i = 0; i < _listeners.size(); ++i)
			_listeners[i].first->Dereference();
		_listeners.clear();
	}

	bool ScriptInput::handleEvent(const Inanity::Input::Event& event)
	{
		for (size_t i = 0; i < _listeners.size(); ++i)
		{
			//ptr<Inanity::Script::Any> scriptEvent = ScriptSystem::getInstance()->createScriptArray(3);

			//ptr<Inanity::Script::Any> result = _listeners[i].first->
		}
		return false;
	}

	void ScriptInput::update()
	{
		size_t listenersCount = _listeners.size();
		for (size_t i = 0; i < listenersCount; ++i)
		{
			if (_listeners[i].second)
			{
				_listeners[i].first->Dereference();
				--i;
				--listenersCount;
				_listeners.pop_back();
			}
		}
	}

	void ScriptInput::setState(const Inanity::Input::State* state)
	{
		_state = state;
	}

	bool ScriptInput::isKeyDown(uint keyCode)
	{
		return keyCode < 256 ? _state->keyboard[keyCode] != 0 : false;
	}

	bool ScriptInput::isMouseKeyDown(uint keyCode)
	{
		return keyCode < 3 ? _state->mouseButtons[keyCode] : false;
	}

	vec2 ScriptInput::getCursorPosition()
	{
		return vec2(_state->mouseX, _state->mouseY);
	}

	void ScriptInput::addListener(ptr<Inanity::Script::Any> listener)
	{
		listener->Reference();
		_listeners.push_back(std::make_pair(listener, false));
	}

	void ScriptInput::removeListener(ptr<Inanity::Script::Any> listener)
	{
		for (size_t i = 0; i < _listeners.size(); ++i)
		{
			if ((Inanity::Script::Any*)listener == _listeners[i].first)
			{
				_listeners[i].second = true;
				break;
			}
		}
	}

}