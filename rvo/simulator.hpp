#ifndef __FBE_RVO_SIMULATOR_H__
#define __FBE_RVO_SIMULATOR_H__

#include "rvo/math.hpp"

using namespace Inanity::Math;

namespace RVO {

	class Agent;
	class KdTree;


	class Simulator 
	{
		friend class Agent;
		friend class KdTree;
		friend class Obstacle;


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
		std::vector<Agent*> agents_;
		Agent* defaultAgent_;
		KdTree* kdTree_;
	};
}

#endif
