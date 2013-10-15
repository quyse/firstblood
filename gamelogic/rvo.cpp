#include <algorithm>
#include <assert.h>
#include "gamelogic/rvo.hpp"
#include "rvo/simulator.hpp"
#include "spatial/interfaces.hpp"
#include "script/system.hpp"

namespace Firstblood
{

	/** Rvo agent **/
	void RvoAgent::FreeAsNotReferenced() {}

	void RvoAgent::setMaxNeighbors(int value)
	{
		maxNeighbors = value;
	}

	void RvoAgent::setMask(uint32_t mask)
	{
		this->mask = mask;
	}

	void RvoAgent::setImmobilized(bool value)
	{
		immobilized = value;
	}

	void RvoAgent::setTimeHorizon(float horizon)
	{
		timeHorizon = horizon;
	}

	float RvoAgent::getMaxSpeed()
	{
		return maxSpeed;
	}

	void RvoAgent::setMaxSpeed(float value)
	{
		maxSpeed = value;
	}

	void RvoAgent::setPrefVelocity(const vec2& velocity)
	{
		prefVelocity = velocity;
	}


	/** Rvo simulation **/
	RvoSimulation::RvoSimulation(size_t maxAgents, Spatial::IIndex2D<ISpatiallyIndexable>* spatialIndex) : _spatialIndex(spatialIndex), RVO::Simulator(maxAgents)
	{
		_allocator = new PoolAllocator(sizeof(RvoAgent), maxAgents);
	}

	RvoSimulation::~RvoSimulation()
	{
		delete _allocator;
	}

	ptr<RvoAgent> RvoSimulation::create(const vec2& position, int uid)
	{
		void* agentMemory = _allocator->allocMemory(sizeof(RvoAgent));
		RvoAgent* agent = new (agentMemory) RvoAgent;
		applyDefaultsToAgent(agent);
		agent->position = position;
		agent->uid = uid;
		toBeAddedQueue.push_back(agent);
		return agent;
	}

	void RvoSimulation::destroy(ptr<RvoAgent> agent)
	{
		for (size_t i = 0; i < toBeRemovedQueue.size(); ++i)
			if (toBeRemovedQueue[i] == (RvoAgent*)agent)
				return;
		toBeRemovedQueue.push_back(agent);
	}

	void RvoSimulation::update(float dt)
	{
		doStep(dt, this);
	}

	void RvoSimulation::postUpdate()
	{
		ScriptSystem* scripts = ScriptSystem::getInstance();
		for (size_t i = 0; i < toBeRemovedQueue.size(); ++i)
		{
			RvoAgent* agent = toBeRemovedQueue[i];
			scripts->removeFromScript(agent);
			removeAgent(agent);
			_allocator->dealloc(agent);
		}
		toBeRemovedQueue.clear();

		for (size_t i = 0; i < toBeAddedQueue.size(); ++i)
		{
			addAgent(toBeAddedQueue[i]);
		}
		toBeAddedQueue.clear();
	}

	void RvoSimulation::setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed)
	{
		RVO::Simulator::setAgentDefaults(neighborDist, maxNeighbors, timeHorizon, radius, maxSpeed);
	}

	size_t RvoSimulation::getMaxAgents()
	{
		return RVO::Simulator::getMaxAgents();
	}

	size_t RvoSimulation::getNumAgents()
	{
		return RVO::Simulator::getNumAgents();
	}

	size_t RvoSimulation::collectSpatialData(ISpatiallyIndexable** list, size_t maxSize)
	{
		size_t amount = std::min(maxSize, _agentsCount);
		for (size_t i = 0; i < amount; ++i)
			*(list + i) = static_cast<RvoAgent*>(_agents[i]);
		return amount;
	}

	size_t RvoSimulation::find(RVO::Agent* agent, RVO::NeighborEntity* result, size_t maxResultLength)
	{
		RvoAgent* ourAgent = static_cast<RvoAgent*>(agent);
		Spatial::NearestNeighbor<ISpatiallyIndexable> intermediateResult[RVO_GET_NEAREST_AGENTS_MAX_BUFFER_SIZE];
		size_t count = _spatialIndex->getNeighbours(vec3(agent->position.x, agent->position.y, 0), agent->neighborDist, agent->mask, &intermediateResult[0], maxResultLength, ourAgent);
		for (size_t i = 0; i < count; ++i)
		{
			ISpatiallyIndexable* intermediateNeighbor = intermediateResult[i].entity;
			RVO::NeighborEntity* neighbor = result + i;
			vec3 neighborPosition = intermediateNeighbor->getPosition();
			neighbor->position = vec2(neighborPosition.x, neighborPosition.y);
			neighbor->radius = intermediateNeighbor->getRadius();
			neighbor->velocity = intermediateNeighbor->getVelocity();
		}
		return count;
	}

}