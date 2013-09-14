#ifndef __FBE_SPATIAL_INDEX_INTERFACES_HPP__
#define __FBE_SPATIAL_INDEX_INTERFACES_HPP__

#include "../../inanity/math/basic.hpp"
#include <stdint.h>

template<class T>
class ISpatialIndex2D
{
public:
	virtual ~ISpatialIndex2D() {};
	
	virtual void addBoundingCircle(vec2& center, float radius, uint32_t mask, T* entity) = 0;
	virtual void purge() = 0;

	virtual float raycast(vec2& origin, vec2& end, uint32_t mask, T** entity) = 0;
	inline float raycast(vec3& origin, vec3& end, uint32_t mask, T** entity)
	{
		return raycast(vec2(origin.x, origin.y), vec2(end.x, end.y), mask, entity);
	}

	// returns true, if result buffer's overflow has occured
	virtual bool getNeighbours(vec2& point, float distance, uint32_t mask, T** result, int maxResultLength) = 0;
	inline bool getNeighbours(vec3& point, float distance, uint32_t mask, T** result, int maxResultLength)
	{
		return getNeighbours(vec2(point.x, point.y), distance, mask, result, maxResultLength);
	}
};

#endif