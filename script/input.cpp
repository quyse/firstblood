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
		bool isMouseEvent = event.device == Inanity::Input::Event::deviceMouse;
		if (!isMouseEvent)
		{
			// we notify scripts only about changes in the keys' state
			if (event.keyboard.type == Inanity::Input::Event::Keyboard::typeCharacter)
			{
				return false;
			}
			else if (event.keyboard.type == Inanity::Input::Event::Keyboard::typeKeyDown)
			{
				if (_charsDown.find(event.keyboard.key) != _charsDown.end())
					return false;
				else
					_charsDown.insert(event.keyboard.key);
			}
			else
			{
				_charsDown.erase(event.keyboard.key);
			}
		}

		ScriptSystem* system = ScriptSystem::getInstance();
		ptr<Inanity::Script::Any> scriptEvent = system->createScriptArray(2);
		ptr<Inanity::Script::Any> eventDetails;

		scriptEvent->Set(0, system->createScriptBoolean(isMouseEvent));
		if (!isMouseEvent)
		{
			eventDetails = system->createScriptArray(2);
			// isDown
			eventDetails->Set(0, system->createScriptBoolean(event.keyboard.type == Inanity::Input::Event::Keyboard::typeKeyDown));
			// keyCode
			eventDetails->Set(1, system->createScriptInteger(event.keyboard.key));
		}
		else
		{
			// todo: fill this gap with something meaningful
			eventDetails = system->createScriptArray(0);
		}
		scriptEvent->Set(1, eventDetails);

		bool result = false;
		for (size_t i = 0; i < _listeners.size(); ++i)
		{
			if (!_listeners[i].second)
			{
				ptr<Inanity::Script::Any> partialResult = _listeners[i].first->ApplyWith(nullptr, &scriptEvent, 1);
				result = result || partialResult->AsInt();

			}
		}
		return result;
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
		return _charsDown.find(keyCode) != _charsDown.end();
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