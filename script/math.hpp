#ifndef __FB_SCRIPT_MATH_HPP__
#define __FB_SCRIPT_MATH_HPP__

#include "inanity/meta/decl.hpp"
#include "inanity/math/basic.hpp"
#include "inanity/Strings.hpp"
#include "inanity/ptr.hpp"

using namespace Inanity;

namespace Firstblood
{

	class ScriptVec3 : public Math::vec3, public Inanity::RefCounted
	{
	public:
		ScriptVec3();
		ScriptVec3(float x, float y, float z);
		float getX();
		float getY();
		float getZ();
		void setX(float x);
		void setY(float y);
		void setZ(float z);
		void set(ptr<ScriptVec3> other);

		ptr<ScriptVec3> add(ptr<ScriptVec3> other);

		Inanity::String toString();

		void operator=(const Math::vec3& other);

	protected:
		virtual void FreeAsNotReferenced();

	META_DECLARE_CLASS(ScriptVec3);
	};

}

#endif