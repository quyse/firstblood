#include "gamelogic/rvo.hpp"
#include "rvo/simulator.hpp"
#include "spatial/interfaces.hpp"

namespace Firstblood
{

	RvoSimulation::RvoSimulation(size_t maxAgents, Spatial::IIndex2D<ISpatiallyIndexable>* spatialIndex) : _spatialIndex(spatialIndex), RVO::Simulator(maxAgents)
	{
		_allocator = new PoolAllocator(sizeof(RvoAgent), maxAgents);
	}

	RvoSimulation::~RvoSimulation()
	{
		delete _allocator;
	}

	RvoAgent* RvoSimulation::create(const vec2& position)
	{
		RvoAgent* agent = _allocator->alloc<RvoAgent>();
		agent->position = position;
		addAgent(agent);
		return agent;
	}

	void RvoSimulation::destroy(RvoAgent* agent)
	{
		removeAgent(agent);
		_allocator->dealloc(agent);
	}

	void RvoSimulation::update(float dt)
	{
		doStep(dt, this);
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