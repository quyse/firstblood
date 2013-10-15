#include "rvo/simulator.hpp"
#include "rvo/agent.hpp"
#include "rvo/interfaces.hpp"
#include "spatial/kd_tree.hpp"
#include "spatial/tree_base.hpp"
#include "memory/pool_allocator.hpp"

namespace RVO 
{

	// todo: kill hardcode
	Simulator::Simulator(size_t maxAgentsCount) : defaultAgent_(NULL), _agentsCount(0), _maxAgentsCount(maxAgentsCount)
	{
		defaultAgent_ = new Agent();
		_agents.reserve(_maxAgentsCount);
		for (size_t i = 0; i < _maxAgentsCount; ++i)
			_agents.push_back(nullptr);
	}

	Simulator::~Simulator()
	{
		_agents.clear();
	}

	Agent* Simulator::addAgent(Agent* agent)
	{
		assert(_agentsCount < _maxAgentsCount);
		agent->_index = _agentsCount ;
		_agents[_agentsCount++] = agent;
		return agent;
	}

	void Simulator::removeAgent(Agent* agent)
	{
		assert(agent->_index < _agentsCount);
		assert(agent == _agents[agent->_index]);
		size_t index = agent->_index;
		_agents[index] = _agents[_agentsCount - 1];
		_agents[index]->_index = index;
		--_agentsCount;
	}

	void Simulator::applyDefaultsToAgent(Agent* agent)
	{
		agent->maxNeighbors = defaultAgent_->maxNeighbors;
		agent->maxSpeed = defaultAgent_->maxSpeed;
		agent->neighborDist = defaultAgent_->neighborDist;
		agent->radius = defaultAgent_->radius;
		agent->timeHorizon = defaultAgent_->timeHorizon;
		agent->velocity_ = defaultAgent_->velocity_;
	}

	void Simulator::doStep(float dt, NearestNeighborsFinder* nearestNeighborsFinder)
	{
		for (size_t i = 0; i < _agentsCount; ++i)
		{
			if (!_agents[i]->immobilized)
				_agents[i]->computeNewVelocity(dt, nearestNeighborsFinder);
			else
				_agents[i]->newVelocity_ = vec2(0, 0);
		}
			
		for (size_t i = 0; i < _agentsCount; ++i)
		{
			_agents[i]->update(dt);
		}	
	}

	size_t Simulator::getNumAgents() const
	{
		return _agentsCount;
	}

	size_t Simulator::getMaxAgents() const
	{
		return _maxAgentsCount;
	}

	void Simulator::setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2 &velocity)
	{
		defaultAgent_->maxNeighbors = maxNeighbors;
		defaultAgent_->maxSpeed = maxSpeed;
		defaultAgent_->neighborDist = neighborDist;
		defaultAgent_->radius = radius;
		defaultAgent_->timeHorizon = timeHorizon;
		defaultAgent_->velocity_ = velocity;
	}

}
