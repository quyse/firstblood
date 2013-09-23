#ifndef __FBE_SPATIAL_INDEX_INTERFACES_HPP__
#define __FBE_SPATIAL_INDEX_INTERFACES_HPP__

#include "inanity/math/basic.hpp"
#include <stdint.h>

namespace Spatial
{

	// the object of class T, which is to be stored in spatial index should implement the following methods:
	// float T::getRadius - returns radius of the bounding circle
	// vec2 T::getPosition - returns center of the bounding circle
	// uint32_t T::getMask - bitfield which is used to ignore groups of objects when querying index
	template<class T>
	class ISpatialIndex2D
	{
	public:
		virtual ~ISpatialIndex2D() {};
	
		virtual void build(T* objects, size_t objectsCount) = 0;
		virtual void purge() = 0;
		virtual void optimize() = 0;

		virtual T* raycast(const vec2& origin, const vec2& end, uint32_t mask, float& t, T* skipEntity = nullptr) = 0;
		virtual size_t getNeighbours(const vec2& point, float distance, uint32_t mask, T** result, size_t maxResultLength, T* skipEntity = nullptr) const = 0;
	};

}

#endif