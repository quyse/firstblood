#ifndef __FBE_RVO_AGENT_HPP__
#define __FBE_RVO_AGENT_HPP__

#include "rvo/math.hpp"

namespace RVO 
{

	class Simulator;
	class KdTree;

	class Agent 
	{
		friend class KdTree;
		friend class Simulator;


	private:
		explicit Agent();
		void computeNeighbors(KdTree* spatialIndex);
		void computeNewVelocity(float dt);
		void insertAgentNeighbor(const Agent *agent, float &rangeSq);
		void update(float dt);


	public:
		vec2 position;
		vec2 prefVelocity;
		float radius;
		float timeHorizon;
		float neighborDist;
		size_t maxNeighbors;
		float maxSpeed;
		bool isStatic;


	private:
		vec2 velocity_;
		vec2 newVelocity_;
		// to be cleared
		std::vector<std::pair<float, const Agent*>>* agentNeighbors_; 

	};

}

#endif
