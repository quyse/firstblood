#ifndef __FB_GAMELOGIC_RVO_HPP__
#define __FB_GAMELOGIC_RVO_HPP__

#include "gamelogic/common.hpp"
#include "rvo/interfaces.hpp"
#include "rvo/simulator.hpp"

template<class T>
class Spatial::IIndex2D;

namespace Firstblood
{

	class RvoAgent : public ISpatiallyIndexable, public RVO::Agent 
	{
	public:
		virtual bool raycast(const vec2& origin, const vec2& end, float& dist) { return true; };
		virtual float getRadius() { return radius; };
		virtual vec2 getPosition() { return position; };
		virtual uint32_t getMask() { return mask; };
		virtual vec2 getVelocity() { return velocity_; };
	};


	class RvoSimulation : public RVO::Simulator, public RVO::NearestNeighborsFinder
	{
	public:
		RvoSimulation(size_t maxAgents, Spatial::IIndex2D<ISpatiallyIndexable>* spatialIndex);
		virtual ~RvoSimulation();

		RvoAgent* create(const vec2& position);
		void destroy(RvoAgent* agent);
		void update(float dt);
		size_t collectSpatialData(ISpatiallyIndexable** list, size_t maxSize);

		virtual size_t find(RVO::Agent* agent, RVO::NeighborEntity* result, size_t maxResultLength);

	private:
		Spatial::IIndex2D<ISpatiallyIndexable>* _spatialIndex;
		PoolAllocator* _allocator;
	};

}

#endif