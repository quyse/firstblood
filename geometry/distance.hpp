#ifndef __FBE_GEOMETRY_DISTANCE__
#define __FBE_GEOMETRY_DISTANCE__

#include "inanity/math/basic.hpp"
#include <math.h>
#include "constants.hpp"

template<typename T>
inline T distanceSquaredPointSegment(const xvec<T, 2>& a, const xvec<T, 2>& b, const xvec<T, 2>& c)
{
	xvec<T, 2> segmentVector = b - a;
	T segmentVectorLengthSquared = length2(segmentVector);

	if (segmentVectorLengthSquared < EPSILON)
		return false;

	xvec<T, 2> fromAtoC = c - a;
	const float r = dot(fromAtoC, segmentVector) / segmentVectorLengthSquared;

	if (r < 0.0f) 
	{
		return length2(fromAtoC);
	}
	else if (r > 1.0f) 
	{
		return length2(c - b);
	}
	else 
	{
		return length2(c - (a + segmentVector * r));
	}
}


template<typename T, int N>
inline T distanceSquaredPointAABB(const xvec<T, N>& point, const xvec<T, N>& min, const xvec<T, N>& max)
{
	T sqrDist = (T)0.0;
	for (size_t i = 0; i < N; ++i)
	{
		T v = point(i);
		if (v < min(i)) 
			sqrDist += sqr(min(i) - v);
		if (v > max(i)) 
			sqrDist += sqr(v - max(i));
	}
	return sqrDist;
}

#endif