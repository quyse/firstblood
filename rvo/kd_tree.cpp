#include "rvo/kd_tree.hpp"
#include "rvo/agent.hpp"
#include "rvo/simulator.hpp"

namespace RVO 
{

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


	size_t KdTree::getNeighbours(Agent* agent, float& rangeSq, size_t maxResultLength, std::pair<float, Agent*>* result) const
	{
		size_t neighboursCount = 0;
		queryAgentTreeRecursive(agent, rangeSq, 0, neighboursCount, maxResultLength, result);
		return neighboursCount;
	}


	void KdTree::queryAgentTreeRecursive(Agent *agent, float &rangeSq, size_t node, size_t& currentResultLength, size_t maxResultLength, std::pair<float, Agent*>* result) const
	{
		if (agentTree_[node].end - agentTree_[node].begin <= MAX_LEAF_SIZE) 
		{
			for (size_t i = agentTree_[node].begin; i < agentTree_[node].end; ++i) 
			{
				Agent* agentToInsert = agents_[i];
				if (agentToInsert != agent) 
				{
					const float dist = std::max(0.0f, length(agentToInsert->position - agent->position) - agentToInsert->radius - agent->radius);
					const float distSq = dist * dist;

					if (distSq < rangeSq) 
					{
						if (currentResultLength < maxResultLength) 
						{
							*(result + currentResultLength) = std::make_pair(distSq, agentToInsert);
							++currentResultLength;
						}

						size_t j = currentResultLength - 1;

						while (j != 0 && distSq < (*(result + j - 1)).first) 
						{
							*(result + j) = *(result + j - 1);
							--j;
						}

						*(result + j) = std::make_pair(distSq, agentToInsert);

						if (currentResultLength == maxResultLength)
							rangeSq = (*(result + currentResultLength - 1)).first;
					}
				}
			}
		}
		else 
		{
			const float distSqLeft = sqr(std::max(0.0f, agentTree_[agentTree_[node].left].minX - agent->position.x)) + sqr(std::max(0.0f, agent->position.x - agentTree_[agentTree_[node].left].maxX)) + sqr(std::max(0.0f, agentTree_[agentTree_[node].left].minY - agent->position.y)) + sqr(std::max(0.0f, agent->position.y - agentTree_[agentTree_[node].left].maxY));

			const float distSqRight = sqr(std::max(0.0f, agentTree_[agentTree_[node].right].minX - agent->position.x)) + sqr(std::max(0.0f, agent->position.x - agentTree_[agentTree_[node].right].maxX)) + sqr(std::max(0.0f, agentTree_[agentTree_[node].right].minY - agent->position.y)) + sqr(std::max(0.0f, agent->position.y - agentTree_[agentTree_[node].right].maxY));

			if (distSqLeft < distSqRight) {
				if (distSqLeft < rangeSq) {
					queryAgentTreeRecursive(agent, rangeSq, agentTree_[node].left, currentResultLength, maxResultLength, result);

					if (distSqRight < rangeSq) {
						queryAgentTreeRecursive(agent, rangeSq, agentTree_[node].right, currentResultLength, maxResultLength, result);
					}
				}
			}
			else {
				if (distSqRight < rangeSq) {
					queryAgentTreeRecursive(agent, rangeSq, agentTree_[node].right, currentResultLength, maxResultLength, result);

					if (distSqLeft < rangeSq) {
						queryAgentTreeRecursive(agent, rangeSq, agentTree_[node].left, currentResultLength, maxResultLength, result);
					}
				}
			}
		}
	}

}
