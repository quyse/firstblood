#include <algorithm>
#include <assert.h>
#include "gamelogic/rvo.hpp"
#include "rvo/simulator.hpp"
#include "spatial/interfaces.hpp"

namespace Firstblood
{

	/** Rvo agent **/
	void RvoAgent::FreeAsNotReferenced() {}

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

	ptr<RvoAgent> RvoSimulation::create(const vec2& position)
	{
		// todo: remove this crap when Inanity::ReferenceCounted will be available
		void* agentMemory = _allocator->allocMemory(sizeof(RvoAgent));
		RvoAgent* agent = new (agentMemory) RvoAgent;
		agent->position = position;
		addAgent(agent);
		return agent;
	}

	void RvoSimulation::destroy(ptr<RvoAgent> agent)
	{
		removeAgent(agent);
		_allocator->dealloc(agent);
	}

	void RvoSimulation::update(float dt)
	{
		doStep(dt, this);
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
		ISpatiallyIndexable* intermediateResult[RVO_GET_NEAREST_AGENTS_MAX_BUFFER_SIZE];
		size_t count = _spatialIndex->getNeighbours(agent->position, agent->neighborDist, agent->mask, &intermediateResult[0], maxResultLength, ourAgent);
		for (size_t i = 0; i < count; ++i)
		{
			ISpatiallyIndexable* intermediateNeighbor = intermediateResult[i];
			RVO::NeighborEntity* neighbor = result + i;
			neighbor->position = intermediateNeighbor->getPosition();
			neighbor->radius = intermediateNeighbor->getRadius();
			neighbor->velocity = intermediateNeighbor->getVelocity();
		}
		return count;
	}

}