#include "rvo/simulator.hpp"
#include "rvo/agent.hpp"
#include "rvo/kd_tree.hpp"

namespace RVO 
{

	Simulator::Simulator() : defaultAgent_(NULL), kdTree_(NULL)
	{
		kdTree_ = new KdTree();
	}


	Simulator::Simulator(float timeStep, float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2& velocity) : defaultAgent_(NULL), kdTree_(NULL)
	{
		kdTree_ = new KdTree();
		defaultAgent_ = new Agent();

		defaultAgent_->maxNeighbors = maxNeighbors;
		defaultAgent_->maxSpeed = maxSpeed;
		defaultAgent_->neighborDist = neighborDist;
		defaultAgent_->radius = radius;
		defaultAgent_->timeHorizon = timeHorizon;
		defaultAgent_->velocity_ = velocity;
	}


	Simulator::~Simulator()
	{
		if (defaultAgent_ != NULL) {
			delete defaultAgent_;
		}

		for (size_t i = 0; i < agents_.size(); ++i) {
			delete agents_[i];
		}

		delete kdTree_;
	}


	Agent* Simulator::addAgent(const vec2 &position)
	{
		Agent *agent = new Agent();

		agent->position = position;
		agent->maxNeighbors = defaultAgent_->maxNeighbors;
		agent->maxSpeed = defaultAgent_->maxSpeed;
		agent->neighborDist = defaultAgent_->neighborDist;
		agent->radius = defaultAgent_->radius;
		agent->timeHorizon = defaultAgent_->timeHorizon;
		agent->velocity_ = defaultAgent_->velocity_;

		agents_.push_back(agent);

		return agent;
	}


	Agent* Simulator::addAgent(const vec2 &position, float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2 &velocity)
	{
		Agent *agent = new Agent();

		agent->position = position;
		agent->maxNeighbors = maxNeighbors;
		agent->maxSpeed = maxSpeed;
		agent->neighborDist = neighborDist;
		agent->radius = radius;
		agent->timeHorizon = timeHorizon;
		agent->velocity_ = velocity;

		agents_.push_back(agent);

		return agent;
	}


	void Simulator::doStep(float dt)
	{
		std::vector<std::pair<float, const Agent*>> agentNeighbors;

		kdTree_->buildAgentTree(agents_, agents_.size());

		for (int i = 0; i < static_cast<int>(agents_.size()); ++i) {
			if (agents_[i]->isStatic)
				continue;
			agentNeighbors.clear();
			agents_[i]->agentNeighbors_ = &agentNeighbors;
			agents_[i]->computeNeighbors(kdTree_);
			agents_[i]->computeNewVelocity(dt);
		}

		for (int i = 0; i < static_cast<int>(agents_.size()); ++i) {
			if (!agents_[i]->isStatic)
				agents_[i]->update(dt);
		}
	}

	size_t Simulator::getNumAgents() const
	{
		return agents_.size();
	}


	void Simulator::setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed, const vec2 &velocity)
	{
		if (defaultAgent_ == NULL) {
			defaultAgent_ = new Agent();
		}

		defaultAgent_->maxNeighbors = maxNeighbors;
		defaultAgent_->maxSpeed = maxSpeed;
		defaultAgent_->neighborDist = neighborDist;
		defaultAgent_->radius = radius;
		defaultAgent_->timeHorizon = timeHorizon;
		defaultAgent_->velocity_ = velocity;
	}

}
