#ifndef __FBE_RVO_AGENT_HPP__
#define __FBE_RVO_AGENT_HPP__

#include <stdint.h>
#include "rvo/math.hpp"
#include "spatial/interfaces.hpp"

namespace RVO 
{

	class Simulator;
	class KdTree;

	class Agent 
	{
		friend class Simulator;

	public:
		// spatial index interface
		inline vec2 getPosition() { return position; }
		inline float getRadius() { return radius; }
		inline uint32_t getMask() { return 1; }
		inline bool raycast(const vec2& origin, const vec2& end, float& dist) { return true; }

	private:
		explicit Agent();
		void computeNewVelocity(float dt, Spatial::ISpatialIndex2D<Agent>* spatialIndex);
		void update(float dt);

	public:
		vec2 position;
		vec2 prefVelocity;
		float radius;
		float timeHorizon;
		float neighborDist;
		size_t maxNeighbors;
		float maxSpeed;
		bool immobilized;

	private:
		vec2 velocity_;
		vec2 newVelocity_;
	};

}

#endif
