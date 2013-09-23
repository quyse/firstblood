#include "rvo/simulator.hpp"
#include "rvo/agent.hpp"
#include "spatial/kd_tree.hpp"
#include "spatial/tree_base.hpp"

namespace RVO 
{

	// todo: kill hardcode
	// todo: when abstract pod storage with indirection is ready, use it as agents' container
	Simulator::Simulator() : defaultAgent_(NULL), _agentsCount(0)
	{
		Spatial::KdTree<Agent>* kdTree = new Spatial::KdTree<Agent>(10, 1024 * 1024);
		_spatialIndex = kdTree;
		_agents = new Agent[512];
	}

	Simulator::~Simulator()
	{
		if (defaultAgent_ != NULL)
			delete defaultAgent_;

		delete[] _agents;

		delete _spatialIndex;
	}

	Agent* Simulator::addAgent(const vec2 &position)
	{
		Agent *agent =_agents + _agentsCount;
		++_agentsCount;
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
		Agent* agent =_agents + _agentsCount;
		++_agentsCount;
		agent->position = position;
		agent->maxNeighbors = maxNeighbors;
		agent->maxSpeed = maxSpeed;
		agent->neighborDist = neighborDist;
		agent->radius = radius;
		agent->timeHorizon = timeHorizon;
		agent->velocity_ = velocity;
		return agent;
	}

	void Simulator::doStep(float dt)
	{
		_spatialIndex->purge();
		_spatialIndex->build(_agents, _agentsCount);

		for (size_t i = 0; i < _agentsCount; ++i) 
		{
			Agent* agent = _agents + i;
			if (!agent->immobilized)
				agent->computeNewVelocity(dt, _spatialIndex);
		}

		for (size_t i = 0; i < _agentsCount; ++i) 
		{
			Agent* agent = _agents + i;
			if (!agent->immobilized)
				agent->update(dt);
		}
	}

	size_t Simulator::getNumAgents() const
	{
		return _agentsCount;
	}

	void Simulator::setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2 &velocity)
	{
		if (defaultAgent_ == NULL)
			defaultAgent_ = new Agent();
		defaultAgent_->maxNeighbors = maxNeighbors;
		defaultAgent_->maxSpeed = maxSpeed;
		defaultAgent_->neighborDist = neighborDist;
		defaultAgent_->radius = radius;
		defaultAgent_->timeHorizon = timeHorizon;
		defaultAgent_->velocity_ = velocity;
	}

}
