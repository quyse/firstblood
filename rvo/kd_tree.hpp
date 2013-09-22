#ifndef __FBE_RVO_KD_TREE_H__
#define __FBE_RVO_KD_TREE_H__

#include "rvo/math.hpp"

namespace RVO {

	class Obstacle;
	class Agent;
	class Simulator;

	class KdTree {
	private:

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


		class ObstacleTreeNode {
		public:
			ObstacleTreeNode *left;
			const Obstacle *obstacle;
			ObstacleTreeNode *right;
		};


		explicit KdTree();
		~KdTree();

		void buildAgentTree(std::vector<Agent*>& agents, size_t agentsCount);
		void buildAgentTreeRecursive(size_t begin, size_t end, size_t node);
		void buildObstacleTree(std::vector<Obstacle*>& obstacles);
		ObstacleTreeNode* buildObstacleTreeRecursive(const std::vector<Obstacle*>& obstacles, std::vector<Obstacle*>& simulatorObstacles);

		void computeAgentNeighbors(Agent *agent, float &rangeSq) const;
		void computeObstacleNeighbors(Agent *agent, float rangeSq) const;
		void deleteObstacleTree(ObstacleTreeNode *node);

		void queryAgentTreeRecursive(Agent* agent, float& rangeSq, size_t node) const;
		void queryObstacleTreeRecursive(Agent* agent, float rangeSq, const ObstacleTreeNode* node) const;

		std::vector<Agent*> agents_;
		std::vector<AgentTreeNode> agentTree_;
		ObstacleTreeNode* obstacleTree_;

		static const size_t MAX_LEAF_SIZE = 10;
		
		friend class Agent;
		friend class Simulator;
	};
}

#endif
