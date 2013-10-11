#ifndef __FB_SCRIPT_CAMERA_HPP__
#define __FB_SCRIPT_CAMERA_HPP__

#include "inanity/ptr.hpp"
#include "inanity/math/basic.hpp"
#include "inanity/meta/decl.hpp"

using namespace Inanity::Math;

namespace Firstblood
{

	class ScriptCamera : public Inanity::Object
	{
	public:
		ScriptCamera(mat4x4* viewMatrix);

		void setLookAtLH(const vec3& position, const vec3& direction, const vec3& upVector);

	private:
		mat4x4* _engineCameraMatrixPtr;

	META_DECLARE_CLASS(ScriptCamera);
	};

}

#endif