#ifndef __FB_GAMELOGIC_COMMON_HPP__
#define __FB_GAMELOGIC_COMMON_HPP__

#include "rvo/interfaces.hpp"
#include "rvo/agent.hpp"
#include "memory/pool_allocator.hpp"

namespace Firstblood
{

	class ISpatiallyIndexable
	{
	public:
		virtual bool raycast(const vec2& origin, const vec2& end, float& dist) = 0;
		virtual float getRadius() = 0;
		virtual vec2 getPosition() = 0;
		virtual uint32_t getMask() = 0;
		virtual vec2 getVelocity() = 0;
	};

}

#endif