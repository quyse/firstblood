#include "math.hpp"
#include <string>
#include <sstream>

namespace Firstblood
{

	ScriptVec3::ScriptVec3() : Math::vec3(0, 0, 0) {}
	ScriptVec3::ScriptVec3(float x, float y, float z) : Math::vec3(x, y, z) {}

	float ScriptVec3::getX() { return x; }
	float ScriptVec3::getY() { return y; }
	float ScriptVec3::getZ() { return z; }

	void ScriptVec3::setX(float value) { x = value; }
	void ScriptVec3::setY(float value) { y = value; }
	void ScriptVec3::setZ(float value) { z = value; }

	void ScriptVec3::set(ptr<ScriptVec3> other) { *this = *other; }


	ptr<ScriptVec3> ScriptVec3::add(ptr<ScriptVec3> other)
	{
		ptr<ScriptVec3> result = new ScriptVec3(0, 0, 0);
		*result = *this + *other;
		return result;
	}

	Inanity::String ScriptVec3::toString()
	{
		std::ostringstream stringStream;
		stringStream << "(" << x << ", " << y << ", " << z << ")";
		return stringStream.str();
	}

	void ScriptVec3::operator=(const Math::vec3& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}

	void ScriptVec3::FreeAsNotReferenced()
	{
		delete this;
	}

}