#include <iostream>
#include <assert.h>
#include "script/utils.hpp"

namespace Firstblood
{

	/** Script logger */
	ScriptLogger* ScriptLogger::_instance = nullptr;

	ScriptLogger::ScriptLogger()
	{
		assert(_instance == nullptr);
		_instance = this;
	}

	void ScriptLogger::write(const Inanity::String& message)
	{
		std::cout << "[SCRIPT] " << message << std::endl;
	}

	ptr<ScriptLogger> ScriptLogger::getInstance()
	{
		return _instance;
	}


	/** Script painter */
	ScriptPainter* ScriptPainter::_instance = nullptr;

	ScriptPainter::ScriptPainter(ptr<Painter> painter)
	{
		assert(_instance == nullptr);
		_instance = this;
		_painter = painter;
	}

	ScriptPainter::~ScriptPainter()
	{
		_painter = nullptr;
	}

	void ScriptPainter::drawLine(const vec3& a, const vec3& b, uint color, float thickness)
	{
		_painter->DebugDrawLine(a, b, colorToVec3(color), thickness);
	}

	void ScriptPainter::drawAABB(const vec3& min, const vec3& max, uint color)
	{
		_painter->DebugDrawAABB(min, max, colorToVec3(color));
	}

	void ScriptPainter::drawRect(const vec2& min, const vec2& max, float z, uint color, float thickness)
	{
		_painter->DebugDrawRectangle(min.x, min.y, max.x, max.y, z, colorToVec3(color), thickness);
	}

	vec3 ScriptPainter::colorToVec3(uint color)
	{
		return vec3((float)(color >> 16) / 255.0f, (float)(color >> 8 & 255) / 255.0f, (float)(color & 255) / 255.0f);
	}

	ptr<ScriptPainter> ScriptPainter::getInstance()
	{
		return _instance;
	}

}