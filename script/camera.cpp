#include "script/camera.hpp"
#include "inanity/math/geometry.hpp"

namespace Firstblood
{

	ScriptCamera::ScriptCamera(mat4x4* engineCameraMatrix) : _engineCameraMatrixPtr(engineCameraMatrix)
	{
	}

	void ScriptCamera::setLookAtLH(const vec3& position, const vec3& direction, const vec3& upVector)
	{
		mat4x4& matrix = *_engineCameraMatrixPtr;
		matrix = CreateLookAtMatrix(position, position + direction, upVector);
	}

}