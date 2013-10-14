#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include "script/utils.hpp"

#define MIN_SEGMENTS_FOR_CIRCLE (uint)16

namespace Firstblood
{

	/** Script logger */
	void ScriptLogger::write(const Inanity::String& message)
	{
		std::cout << "[SCRIPT] " << message << std::endl;
	}


	/** Script painter */
	ScriptPainter::ScriptPainter(ptr<Painter> painter) : _scale(1.0f)
	{
		_painter = painter;
	}

	ScriptPainter::~ScriptPainter()
	{
		_painter = nullptr;
	}

	void ScriptPainter::setGlobalScale(float scale)
	{
		_scale = scale;
	}

	void ScriptPainter::drawLine(const vec3& a, const vec3& b, uint color, float thickness)
	{
		_painter->DebugDrawLine(a * _scale, b * _scale, colorToVec3(color), thickness);
	}

	void ScriptPainter::drawAABB(const vec3& min, const vec3& max, uint color)
	{
		_painter->DebugDrawAABB(min * _scale, max * _scale, colorToVec3(color));
	}

	void ScriptPainter::drawRect(const vec2& min, const vec2& max, float z, uint color, float thickness)
	{
		_painter->DebugDrawRectangle(min.x * _scale, min.y * _scale, max.x * _scale, max.y * _scale, z, colorToVec3(color), thickness);
	}

	void ScriptPainter::drawCircle(const vec3& center, float radius, uint color, uint segments)
	{
		_painter->DebugDrawCircle(center * _scale, radius * _scale, colorToVec3(color), std::max(MIN_SEGMENTS_FOR_CIRCLE, segments));
	}

	vec3 ScriptPainter::colorToVec3(uint color)
	{
		return vec3((float)(color >> 16) / 255.0f, (float)(color >> 8 & 255) / 255.0f, (float)(color & 255) / 255.0f);
	}

}