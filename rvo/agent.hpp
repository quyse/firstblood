#ifndef __FBE_RVO_AGENT_HPP__
#define __FBE_RVO_AGENT_HPP__

#include <stdint.h>
#include "rvo/math.hpp"
#include "spatial/interfaces.hpp"
#include "rvo/interfaces.hpp"

#define RVO_GET_NEAREST_AGENTS_MAX_BUFFER_SIZE (size_t)128

namespace RVO 
{

	class Simulator;
	class KdTree;

	class Agent 
	{
	friend class Simulator;

	public:
		Agent();
		virtual ~Agent();

	private:
		void computeNewVelocity(float dt, NearestNeighborsFinder* nearestNeighborsFinder);
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
		uint32_t mask;

	protected:
		vec2 velocity_;
		vec2 newVelocity_;
		size_t _index;
	};

}

#endif
