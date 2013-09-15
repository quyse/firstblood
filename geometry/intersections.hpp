#ifndef __FBE_GEOMETRY_INTERSECTIONS__
#define __FBE_GEOMETRY_INTERSECTIONS__

#include "../../inanity/math/basic.hpp"
#include <math.h>
#include <algorithm>

static float epsilon = 1e-6f;

inline bool testSegmentSegment1D(float a0, float a1, float b0, float b1)
{
	return (a1 - b0) * (a0 - b1) <= 0;
}


template<typename T, int N>
inline bool testPointAABB(const xvec<T, N>& point, const xvec<T, N>& boxMin, const xvec<T, N>& boxMax)
{
	for (int i = 0; i < N; ++i)
	{
		T pointValue = point(i);
		if (boxMin(i) > pointValue || pointValue > boxMax(i))
			return false;
	}
	return true;
}


template<typename T, int N>
inline bool testAABBAABB(const xvec<T, N>& bMin0, const xvec<T, N>& bMax0, const xvec<T, N>& bMin1, const xvec<T, N>& bMax1)
{
	for (int i = 0; i < N; ++i)
	{
		if (!testSegmentSegment1D(bMin0(i), bMax0(i), bMin1(i), bMax1(i)))
			return false;
	}
	return true;
}


// if segment intersects AABB, sets intersection0 and intersection1 to be start/end points of intersection
template<typename T, int N>
inline bool intersectSegmentAABB(const xvec<T, N>& a0, const xvec<T, N>& a1, const xvec<T, N>& boxMin, const xvec<T, N>& boxMax, xvec<T, N>& intersection0, xvec<T, N>& intersection1, T& tmin, T& tmax)
{
	xvec<T, N> d = a1 - a0;
	tmin = -FLT_MAX;
	tmax = FLT_MAX;

	for (int i = 0; i < N; ++i) 
	{
		if (fabsf(d(i)) < (T)epsilon) 
		{
			if (a0(i) < boxMin(i) || a0(i) > boxMax(i)) 
				return false;
		} 
		else 
		{
			float ood = 1.0f / d(i);
			float t1 = (boxMin(i) - a0(i)) * ood;
			float t2 = (boxMax(i) - a0(i)) * ood;
			if (t1 > t2)
				std::swap(t1, t2);
			if (t1 > tmin) 
				tmin = t1;
			if (t2 < tmax) 
				tmax = t2;
			if (tmin > tmax) 
				return false;
		}
	}
	
	if (!testSegmentSegment1D(0, 1, tmin, tmax))
		return false;
	tmin = clamp(tmin, (T)0.0, (T)1.0);
	tmax = clamp(tmax, (T)0.0, (T)1.0);
	intersection0 = a0 + d * tmin;
	intersection1 = a0 + d * tmax;
	return true;
}


// if segment intersects sphere, sets intersection0 and intersection1 to be start/end points of intersection
template<typename T, int N>
inline bool intersectSegmentSphere(const xvec<T, N>& a0, const xvec<T, N>& a1, const xvec<T, N>& center, T radius, xvec<T, N>& intersection0, xvec<T, N>& intersection1, T& tmin, T& tmax)
{
	xvec<T, N> d = a1 - a0;
	xvec<T, N> m = a0 - center;
	T b = dot(m, d);
	T c = dot(m, m) - radius * radius;
	if (c > (T)0.0 && b > (T)0.0) 
		return false;

	float discr = b * b - c;
	if (discr < (T)0.0) 
		return false;
	T sqrtDiscr = sqrtf(discr);
	tmin = -b - sqrtDiscr;
	tmax = -b + sqrtDiscr;

	if (!testSegmentSegment1D(0, 1, tmin, tmax))
		return false;
	tmin = clamp(tmin, (T)0.0, (T)1.0);
	tmax = clamp(tmax, (T)0.0, (T)1.0);
	intersection0 = a0 + d * tmin;
	intersection1 = a0 + d * tmax;
	return true;
}

#endif