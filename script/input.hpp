#ifndef __FB_SCRIPT_INPUT_HPP__
#define __FB_SCRIPT_INPUT_HPP__

#include <vector>
#include "inanity/ptr.hpp"
#include "inanity/math/basic.hpp"
#include "inanity/meta/decl.hpp"
#include "inanity/script/Any.hpp"
#include "inanity/inanity-input.hpp"

using namespace Inanity;
using namespace Inanity::Math;

namespace Firstblood
{

	class ScriptInput : public Inanity::Object
	{
	public:
		void fini();
		
		bool handleEvent(const Inanity::Input::Event& event);
		void setState(const Inanity::Input::State* state);
		void update();
		bool isKeyDown(uint keyCode);
		bool isMouseKeyDown(uint keyCode);
		vec2 getCursorPosition();

		void addListener(ptr<Inanity::Script::Any> closure);
		void removeListener(ptr<Inanity::Script::Any> closure);

	private:
		// second element of the pair is "dead" mark
		std::vector<std::pair<Inanity::Script::Any*, bool>> _listeners;
		const Inanity::Input::State* _state;

	META_DECLARE_CLASS(ScriptInput);
	};

}

#endif