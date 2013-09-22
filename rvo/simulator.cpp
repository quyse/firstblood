#include "rvo/simulator.hpp"
#include "rvo/agent.hpp"
#include "rvo/kd_tree.hpp"
#include "rvo/obstacle.hpp"

namespace RVO 
{

	Simulator::Simulator() : defaultAgent_(NULL), kdTree_(NULL)
	{
		kdTree_ = new KdTree();
	}

	Simulator::Simulator(float timeStep, float neighborDist, size_t maxNeighbors, float timeHorizon, float timeHorizonObst, float radius, float maxSpeed, const vec2& velocity) : defaultAgent_(NULL), kdTree_(NULL)
	{
		kdTree_ = new KdTree();
		defaultAgent_ = new Agent();

		defaultAgent_->maxNeighbors = maxNeighbors;
		defaultAgent_->maxSpeed = maxSpeed;
		defaultAgent_->neighborDist = neighborDist;
		defaultAgent_->radius = radius;
		defaultAgent_->timeHorizon = timeHorizon;
		defaultAgent_->timeHorizonObst = timeHorizonObst;
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

		for (size_t i = 0; i < obstacles_.size(); ++i) {
			delete obstacles_[i];
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
		agent->timeHorizonObst = defaultAgent_->timeHorizonObst;
		agent->velocity_ = defaultAgent_->velocity_;

		agents_.push_back(agent);

		return agent;
	}

	Agent* Simulator::addAgent(const vec2 &position, float neighborDist, size_t maxNeighbors, float timeHorizon, float timeHorizonObst, float radius, float maxSpeed, const vec2 &velocity)
	{
		Agent *agent = new Agent();

		agent->position = position;
		agent->maxNeighbors = maxNeighbors;
		agent->maxSpeed = maxSpeed;
		agent->neighborDist = neighborDist;
		agent->radius = radius;
		agent->timeHorizon = timeHorizon;
		agent->timeHorizonObst = timeHorizonObst;
		agent->velocity_ = velocity;

		agents_.push_back(agent);

		return agent;
	}

	size_t Simulator::addObstacle(const std::vector<vec2>& vertices)
	{
		const size_t obstacleNo = obstacles_.size();

		for (size_t i = 0; i < vertices.size(); ++i) {
			Obstacle *obstacle = new Obstacle();
			obstacle->point_ = vertices[i];

			if (i != 0) {
				obstacle->prevObstacle_ = obstacles_.back();
				obstacle->prevObstacle_->nextObstacle_ = obstacle;
			}

			if (i == vertices.size() - 1) {
				obstacle->nextObstacle_ = obstacles_[obstacleNo];
				obstacle->nextObstacle_->prevObstacle_ = obstacle;
			}

			obstacle->unitDir_ = normalize(vertices[(i == vertices.size() - 1 ? 0 : i + 1)] - vertices[i]);

			if (vertices.size() == 2) {
				obstacle->isConvex_ = true;
			}
			else {
				obstacle->isConvex_ = (leftOf(vertices[(i == 0 ? vertices.size() - 1 : i - 1)], vertices[i], vertices[(i == vertices.size() - 1 ? 0 : i + 1)]) >= 0.0f);
			}

			obstacle->id_ = obstacles_.size();

			obstacles_.push_back(obstacle);
		}

		return obstacleNo;
	}

	void Simulator::doStep(float dt)
	{
		std::vector<std::pair<float, const Agent*>> agentNeighbors;
		std::vector<std::pair<float, const Obstacle*>> obstacleNeighbors;

		kdTree_->buildAgentTree(agents_, agents_.size());

		for (int i = 0; i < static_cast<int>(agents_.size()); ++i) {
			if (agents_[i]->isStatic)
				continue;
			agentNeighbors.clear();
			obstacleNeighbors.clear();
			agents_[i]->agentNeighbors_ = &agentNeighbors;
			agents_[i]->obstacleNeighbors_ = &obstacleNeighbors;
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

	void Simulator::processObstacles()
	{
		kdTree_->buildObstacleTree(obstacles_);
	}

	void Simulator::setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float timeHorizonObst, float radius, float maxSpeed, const vec2 &velocity)
	{
		if (defaultAgent_ == NULL) {
			defaultAgent_ = new Agent();
		}

		defaultAgent_->maxNeighbors = maxNeighbors;
		defaultAgent_->maxSpeed = maxSpeed;
		defaultAgent_->neighborDist = neighborDist;
		defaultAgent_->radius = radius;
		defaultAgent_->timeHorizon = timeHorizon;
		defaultAgent_->timeHorizonObst = timeHorizonObst;
		defaultAgent_->velocity_ = velocity;
	}

}
