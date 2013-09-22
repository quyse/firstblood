#ifndef __FBE_RVO_KD_TREE_H__
#define __FBE_RVO_KD_TREE_H__

#include "rvo/math.hpp"

namespace RVO {

	class Agent;
	class Simulator;

	class KdTree 
	{
		friend class Agent;
		friend class Simulator;

	private:
		static const size_t MAX_LEAF_SIZE = 10;

		class AgentTreeNode {
		public:
			size_t begin;
			size_t end;
			size_t left;
			float maxX;
			float maxY;
			float minX;
			float minY;
			size_t right;
		};


	private:
		void buildAgentTree(std::vector<Agent*>& agents, size_t agentsCount);
		void buildAgentTreeRecursive(size_t begin, size_t end, size_t node);

		void computeAgentNeighbors(Agent *agent, float &rangeSq) const;

		void queryAgentTreeRecursive(Agent* agent, float& rangeSq, size_t node) const;

		std::vector<Agent*> agents_;
		std::vector<AgentTreeNode> agentTree_;
	};
}

#endif
