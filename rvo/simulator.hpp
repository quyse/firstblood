#ifndef __FBE_RVO_SIMULATOR_H__
#define __FBE_RVO_SIMULATOR_H__

#include "rvo/math.hpp"
#include "spatial/interfaces.hpp"

using namespace Inanity::Math;

namespace RVO 
{

	class Agent;
	class SpatialIndex;

	class Simulator 
	{
		friend class Agent;

	public:
		Simulator();
		Simulator(float timeStep, float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2& velocity = vec2());
		~Simulator();

		Agent* addAgent(const vec2& position);
		Agent* addAgent(const vec2& position, float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2& velocity = vec2());
		size_t getNumAgents() const;
		void setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2& velocity = vec2());

		void doStep(float dt);

	private:
		Agent* _agents;
		size_t _agentsCount;
		Agent* defaultAgent_;
		Spatial::ISpatialIndex2D<Agent>* _spatialIndex;
	};
}

#endif
