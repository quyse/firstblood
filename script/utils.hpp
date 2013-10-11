#ifndef __FB_SCRIPT_UTILS_HPP__
#define __FB_SCRIPT_UTILS_HPP__

#include "inanity/meta/decl.hpp"
#include "inanity/String.hpp"
#include "inanity/ptr.hpp"
#include "Painter.hpp"

using namespace Inanity;

namespace Firstblood
{

	class ScriptLogger : public Inanity::Object
	{
	public:
		void write(const Inanity::String& message);

	META_DECLARE_CLASS(ScriptLogger);
	};


	class ScriptPainter : public Inanity::Object
	{
	public:
		ScriptPainter(ptr<Painter> painter);
		~ScriptPainter();

		void drawLine(const vec3& a, const vec3& b, uint color, float thickness);
		void drawAABB(const vec3& min, const vec3& max, uint color);
		void drawRect(const vec2& min, const vec2& max, float z, uint color, float thickness);

	private:
		vec3 colorToVec3(uint color);

	private:
		ptr<Painter> _painter;

	META_DECLARE_CLASS(ScriptPainter);
	};

}

#endif