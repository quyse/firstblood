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
		matrix(3, 0) = position.x;
		matrix(3, 1) = position.y;
		matrix(3, 2) = position.z;
	}

}