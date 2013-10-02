#ifndef __FBE_RVO_INTERFACES_HPP__
#define __FBE_RVO_INTERFACES_HPP__

#include <stdint.h>
#include "inanity/math/basic.hpp"

using namespace Inanity::Math;

namespace RVO
{
	
	class Agent;

	struct NeighborEntity
	{
		vec2 position;
		vec2 velocity;
		float radius;
	};

	class NearestNeighborsFinder
	{
	public:
		virtual size_t find(Agent* agent, NeighborEntity* result, size_t maxResultLength) = 0;
	};

}

#endif