#include "rvo/simulator.hpp"
#include "rvo/agent.hpp"
#include "spatial/kd_tree.hpp"
#include "spatial/tree_base.hpp"
#include "memory/pool_allocator.hpp"

namespace RVO 
{

	// todo: kill hardcode
	Simulator::Simulator(size_t maxAgentsCount) : defaultAgent_(NULL), _agentsCount(0), _maxAgentsCount(maxAgentsCount)
	{
		_allocator = new PoolAllocator(sizeof(Agent), _maxAgentsCount + 1);
		_agents.reserve(_maxAgentsCount);
		for (size_t i = 0; i < _maxAgentsCount; ++i)
			_agents.push_back(nullptr);

		defaultAgent_ = _allocator->alloc<Agent>();

		Spatial::KdTree<Agent>* kdTree = new Spatial::KdTree<Agent>(10, 1024 * 1024);
		_spatialIndex = kdTree;
	}

	Simulator::~Simulator()
	{
		delete _spatialIndex;
		delete _allocator;
		_agents.clear();
	}

	Agent* Simulator::addAgent(const vec2 &position)
	{
		assert(_agentsCount < _maxAgentsCount);
		Agent* agent = _allocator->alloc<Agent>();
		agent->_index = _agentsCount ;
		_agents[_agentsCount++] = agent;
		agent->position = position;
		agent->maxNeighbors = defaultAgent_->maxNeighbors;
		agent->maxSpeed = defaultAgent_->maxSpeed;
		agent->neighborDist = defaultAgent_->neighborDist;
		agent->radius = defaultAgent_->radius;
		agent->timeHorizon = defaultAgent_->timeHorizon;
		agent->velocity_ = defaultAgent_->velocity_;
		return agent;
	}

	Agent* Simulator::addAgent(const vec2 &position, float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2 &velocity)
	{
		assert(_agentsCount < _maxAgentsCount);
		Agent* agent = _allocator->alloc<Agent>();
		agent->_index = _agentsCount;
		_agents[_agentsCount++] = agent;
		agent->position = position;
		agent->maxNeighbors = maxNeighbors;
		agent->maxSpeed = maxSpeed;
		agent->neighborDist = neighborDist;
		agent->radius = radius;
		agent->timeHorizon = timeHorizon;
		agent->velocity_ = velocity;
		return agent;
	}

	void Simulator::removeAgent(Agent* agent)
	{
		assert(agent->_index < _agentsCount);
		assert(agent == _agents[agent->_index]);
		size_t index = agent->_index;
		_allocator->dealloc(agent);
		_agents[index] = _agents[_agentsCount - 1];
		_agents[index]->_index = index;
		--_agentsCount;
	}

	void Simulator::doStep(float dt)
	{
		_spatialIndex->purge();
		_spatialIndex->build(&_agents[0], _agentsCount);

		for (size_t i = 0; i < _agentsCount; ++i) 
		{
			Agent* agent = _agents[i];
			if (!agent->immobilized)
				agent->computeNewVelocity(dt, _spatialIndex);
		}

		for (size_t i = 0; i < _agentsCount; ++i) 
		{
			Agent* agent = _agents[i];
			if (!agent->immobilized)
				agent->update(dt);
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
