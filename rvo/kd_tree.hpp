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


	public:
		void buildAgentTree(std::vector<Agent*>& agents, size_t agentsCount);
		size_t getNeighbours(Agent* agent, float& rangeSq, size_t maxResultLength, std::pair<float, Agent*>* result) const;


	private:
		void buildAgentTreeRecursive(size_t begin, size_t end, size_t node);
		void queryAgentTreeRecursive(Agent* agent, float& rangeSq, size_t node, size_t& currentResultLength, size_t maxResultLength, std::pair<float, Agent*>* result) const;


	private:
		std::vector<Agent*> agents_;
		std::vector<AgentTreeNode> agentTree_;
	};
}

#endif
