#include "rvo/kd_tree.hpp"
#include "rvo/agent.hpp"
#include "rvo/simulator.hpp"
#include "rvo/obstacle.hpp"

namespace RVO 
{

	KdTree::KdTree() : obstacleTree_(NULL) {}

	KdTree::~KdTree()
	{
		deleteObstacleTree(obstacleTree_);
	}

	void KdTree::buildAgentTree(std::vector<Agent*>& agents, size_t agentsCount)
	{
		agents_.resize(agentsCount);
		for (size_t i = 0; i < agentsCount; ++i)
			agents_[i] = agents[i];

		agentTree_.resize(2 * agentsCount - 1);

		if (!agents_.empty()) 
			buildAgentTreeRecursive(0, agentsCount, 0);
	}

	void KdTree::buildAgentTreeRecursive(size_t begin, size_t end, size_t node)
	{
		agentTree_[node].begin = begin;
		agentTree_[node].end = end;
		agentTree_[node].minX = agentTree_[node].maxX = agents_[begin]->position.x;
		agentTree_[node].minY = agentTree_[node].maxY = agents_[begin]->position.y;

		for (size_t i = begin + 1; i < end; ++i) {
			agentTree_[node].maxX = std::max(agentTree_[node].maxX, agents_[i]->position.x);
			agentTree_[node].minX = std::min(agentTree_[node].minX, agents_[i]->position.x);
			agentTree_[node].maxY = std::max(agentTree_[node].maxY, agents_[i]->position.y);
			agentTree_[node].minY = std::min(agentTree_[node].minY, agents_[i]->position.y);
		}

		if (end - begin > MAX_LEAF_SIZE) {
			/* No leaf node. */
			const bool isVertical = (agentTree_[node].maxX - agentTree_[node].minX > agentTree_[node].maxY - agentTree_[node].minY);
			const float splitValue = (isVertical ? 0.5f * (agentTree_[node].maxX + agentTree_[node].minX) : 0.5f * (agentTree_[node].maxY + agentTree_[node].minY));

			size_t left = begin;
			size_t right = end;

			while (left < right) {
				while (left < right && (isVertical ? agents_[left]->position.x : agents_[left]->position.y) < splitValue) {
					++left;
				}

				while (right > left && (isVertical ? agents_[right - 1]->position.x : agents_[right - 1]->position.y) >= splitValue) {
					--right;
				}

				if (left < right) {
					std::swap(agents_[left], agents_[right - 1]);
					++left;
					--right;
				}
			}

			if (left == begin) {
				++left;
				++right;
			}

			agentTree_[node].left = node + 1;
			agentTree_[node].right = node + 2 * (left - begin);

			buildAgentTreeRecursive(begin, left, agentTree_[node].left);
			buildAgentTreeRecursive(left, end, agentTree_[node].right);
		}
	}

	void KdTree::buildObstacleTree(std::vector<Obstacle*>& simulatorObstacles)
	{
		deleteObstacleTree(obstacleTree_);

		std::vector<Obstacle *> obstacles(simulatorObstacles.size());

		for (size_t i = 0; i < simulatorObstacles.size(); ++i)
			obstacles[i] = simulatorObstacles[i];

		obstacleTree_ = buildObstacleTreeRecursive(obstacles, simulatorObstacles);
	}


	KdTree::ObstacleTreeNode *KdTree::buildObstacleTreeRecursive(const std::vector<Obstacle *> &obstacles, std::vector<Obstacle*>& simulatorObstacles)
	{
		if (obstacles.empty()) {
			return NULL;
		}
		else {
			ObstacleTreeNode *const node = new ObstacleTreeNode;

			size_t optimalSplit = 0;
			size_t minLeft = obstacles.size();
			size_t minRight = obstacles.size();

			for (size_t i = 0; i < obstacles.size(); ++i) {
				size_t leftSize = 0;
				size_t rightSize = 0;

				const Obstacle *const obstacleI1 = obstacles[i];
				const Obstacle *const obstacleI2 = obstacleI1->nextObstacle_;

				/* Compute optimal split node. */
				for (size_t j = 0; j < obstacles.size(); ++j) {
					if (i == j) {
						continue;
					}

					const Obstacle *const obstacleJ1 = obstacles[j];
					const Obstacle *const obstacleJ2 = obstacleJ1->nextObstacle_;

					const float j1LeftOfI = leftOf(obstacleI1->point_, obstacleI2->point_, obstacleJ1->point_);
					const float j2LeftOfI = leftOf(obstacleI1->point_, obstacleI2->point_, obstacleJ2->point_);

					if (j1LeftOfI >= -EPSILON && j2LeftOfI >= -EPSILON) {
						++leftSize;
					}
					else if (j1LeftOfI <= EPSILON && j2LeftOfI <= EPSILON) {
						++rightSize;
					}
					else {
						++leftSize;
						++rightSize;
					}

					if (std::make_pair(std::max(leftSize, rightSize), std::min(leftSize, rightSize)) >= std::make_pair(std::max(minLeft, minRight), std::min(minLeft, minRight))) {
						break;
					}
				}

				if (std::make_pair(std::max(leftSize, rightSize), std::min(leftSize, rightSize)) < std::make_pair(std::max(minLeft, minRight), std::min(minLeft, minRight))) {
					minLeft = leftSize;
					minRight = rightSize;
					optimalSplit = i;
				}
			}

			/* Build split node. */
			std::vector<Obstacle *> leftObstacles(minLeft);
			std::vector<Obstacle *> rightObstacles(minRight);

			size_t leftCounter = 0;
			size_t rightCounter = 0;
			const size_t i = optimalSplit;

			const Obstacle *const obstacleI1 = obstacles[i];
			const Obstacle *const obstacleI2 = obstacleI1->nextObstacle_;

			for (size_t j = 0; j < obstacles.size(); ++j) {
				if (i == j) {
					continue;
				}

				Obstacle *const obstacleJ1 = obstacles[j];
				Obstacle *const obstacleJ2 = obstacleJ1->nextObstacle_;

				const float j1LeftOfI = leftOf(obstacleI1->point_, obstacleI2->point_, obstacleJ1->point_);
				const float j2LeftOfI = leftOf(obstacleI1->point_, obstacleI2->point_, obstacleJ2->point_);

				if (j1LeftOfI >= -EPSILON && j2LeftOfI >= -EPSILON) {
					leftObstacles[leftCounter++] = obstacles[j];
				}
				else if (j1LeftOfI <= EPSILON && j2LeftOfI <= EPSILON) {
					rightObstacles[rightCounter++] = obstacles[j];
				}
				else {
					/* Split obstacle j. */
					const float t = det(obstacleI2->point_ - obstacleI1->point_, obstacleJ1->point_ - obstacleI1->point_) / det(obstacleI2->point_ - obstacleI1->point_, obstacleJ1->point_ - obstacleJ2->point_);

					const vec2 splitpoint = obstacleJ1->point_ + (obstacleJ2->point_ - obstacleJ1->point_) * t;

					Obstacle *const newObstacle = new Obstacle();
					newObstacle->point_ = splitpoint;
					newObstacle->prevObstacle_ = obstacleJ1;
					newObstacle->nextObstacle_ = obstacleJ2;
					newObstacle->isConvex_ = true;
					newObstacle->unitDir_ = obstacleJ1->unitDir_;

					newObstacle->id_ = simulatorObstacles.size();
					simulatorObstacles.push_back(newObstacle);

					obstacleJ1->nextObstacle_ = newObstacle;
					obstacleJ2->prevObstacle_ = newObstacle;

					if (j1LeftOfI > 0.0f) {
						leftObstacles[leftCounter++] = obstacleJ1;
						rightObstacles[rightCounter++] = newObstacle;
					}
					else {
						rightObstacles[rightCounter++] = obstacleJ1;
						leftObstacles[leftCounter++] = newObstacle;
					}
				}
			}

			node->obstacle = obstacleI1;
			node->left = buildObstacleTreeRecursive(leftObstacles, simulatorObstacles);
			node->right = buildObstacleTreeRecursive(rightObstacles, simulatorObstacles);
			return node;
		}
	}

	void KdTree::computeAgentNeighbors(Agent *agent, float &rangeSq) const
	{
		queryAgentTreeRecursive(agent, rangeSq, 0);
	}

	void KdTree::computeObstacleNeighbors(Agent *agent, float rangeSq) const
	{
		queryObstacleTreeRecursive(agent, rangeSq, obstacleTree_);
	}

	void KdTree::deleteObstacleTree(ObstacleTreeNode *node)
	{
		if (node != NULL) {
			deleteObstacleTree(node->left);
			deleteObstacleTree(node->right);
			delete node;
		}
	}

	void KdTree::queryAgentTreeRecursive(Agent *agent, float &rangeSq, size_t node) const
	{
		if (agentTree_[node].end - agentTree_[node].begin <= MAX_LEAF_SIZE) {
			for (size_t i = agentTree_[node].begin; i < agentTree_[node].end; ++i) {
				agent->insertAgentNeighbor(agents_[i], rangeSq);
			}
		}
		else {
			const float distSqLeft = sqr(std::max(0.0f, agentTree_[agentTree_[node].left].minX - agent->position.x)) + sqr(std::max(0.0f, agent->position.x - agentTree_[agentTree_[node].left].maxX)) + sqr(std::max(0.0f, agentTree_[agentTree_[node].left].minY - agent->position.y)) + sqr(std::max(0.0f, agent->position.y - agentTree_[agentTree_[node].left].maxY));

			const float distSqRight = sqr(std::max(0.0f, agentTree_[agentTree_[node].right].minX - agent->position.x)) + sqr(std::max(0.0f, agent->position.x - agentTree_[agentTree_[node].right].maxX)) + sqr(std::max(0.0f, agentTree_[agentTree_[node].right].minY - agent->position.y)) + sqr(std::max(0.0f, agent->position.y - agentTree_[agentTree_[node].right].maxY));

			if (distSqLeft < distSqRight) {
				if (distSqLeft < rangeSq) {
					queryAgentTreeRecursive(agent, rangeSq, agentTree_[node].left);

					if (distSqRight < rangeSq) {
						queryAgentTreeRecursive(agent, rangeSq, agentTree_[node].right);
					}
				}
			}
			else {
				if (distSqRight < rangeSq) {
					queryAgentTreeRecursive(agent, rangeSq, agentTree_[node].right);

					if (distSqLeft < rangeSq) {
						queryAgentTreeRecursive(agent, rangeSq, agentTree_[node].left);
					}
				}
			}

		}
	}

	void KdTree::queryObstacleTreeRecursive(Agent *agent, float rangeSq, const ObstacleTreeNode *node) const
	{
		if (node == NULL) {
			return;
		}
		else {
			const Obstacle *const obstacle1 = node->obstacle;
			const Obstacle *const obstacle2 = obstacle1->nextObstacle_;

			const float agentLeftOfLine = leftOf(obstacle1->point_, obstacle2->point_, agent->position);

			queryObstacleTreeRecursive(agent, rangeSq, (agentLeftOfLine >= 0.0f ? node->left : node->right));

			const float distSqLine = sqr(agentLeftOfLine) / length2(obstacle2->point_ - obstacle1->point_);

			if (distSqLine < rangeSq) {
				if (agentLeftOfLine < 0.0f) {
					/*
					 * Try obstacle at this node only if agent is on right side of
					 * obstacle (and can see obstacle).
					 */
					agent->insertObstacleNeighbor(node->obstacle, rangeSq);
				}

				/* Try other side of line. */
				queryObstacleTreeRecursive(agent, rangeSq, (agentLeftOfLine >= 0.0f ? node->right : node->left));

			}
		}
	}

}
