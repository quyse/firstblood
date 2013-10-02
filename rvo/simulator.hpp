#ifndef __FBE_RVO_SIMULATOR_H__
#define __FBE_RVO_SIMULATOR_H__

#include <vector>
#include "rvo/math.hpp"
#include "spatial/interfaces.hpp"

using namespace Inanity::Math;

class PoolAllocator;

namespace RVO 
{

	class Agent;
	class NearestNeighborsFinder;

	class Simulator 
	{
		friend class Agent;

	public:
		Simulator(size_t maxAgentsCount);
		virtual ~Simulator();

		Agent* addAgent(Agent* agent);
		void removeAgent(Agent* agent);
		size_t getNumAgents() const;
		size_t getMaxAgents() const;
		void setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2& velocity = vec2());

		void doStep(float dt, NearestNeighborsFinder* nearestNeighborsFinder);

	protected:
		std::vector<Agent*> _agents;
		size_t _agentsCount;
		size_t _maxAgentsCount;
		Agent* defaultAgent_;
	};
}

#endif
