#ifndef __FB_GAMELOGIC_RVO_HPP__
#define __FB_GAMELOGIC_RVO_HPP__

#include "inanity/meta/decl.hpp"
#include "inanity/ptr.hpp"
#include "gamelogic/common.hpp"
#include "rvo/interfaces.hpp"
#include "rvo/simulator.hpp"

using namespace Inanity;

namespace Spatial
{
	template<class T>
	class IIndex2D;
};

namespace Firstblood
{

	class RvoAgent : public ISpatiallyIndexable, public Inanity::RefCounted, public RVO::Agent
	{
	public:
		virtual bool raycast(const vec2& origin, const vec2& end, float& dist) { return true; };
		virtual float getRadius() { return radius; };
		virtual vec2 getPosition() { return position; };
		virtual uint32_t getMask() { return mask; };
		virtual vec2 getVelocity() { return velocity_; };

		void setMaxSpeed(float value);
		void setPrefVelocity(const vec2& velocity);
		// todo: remove this crap when c++->script conversion routines for math types are available
		float getX() { return position.x; };
		float getY() { return position.y; };

		void FreeAsNotReferenced();

	META_DECLARE_CLASS( RvoAgent );
	};


	class RvoSimulation : public RVO::Simulator, public RVO::NearestNeighborsFinder, public Inanity::Object
	{
	public:
		RvoSimulation(size_t maxAgents, Spatial::IIndex2D<ISpatiallyIndexable>* spatialIndex);
		virtual ~RvoSimulation();

		ptr<RvoAgent> create(const vec2& position);
		void destroy(ptr<RvoAgent> agent);
		void update(float dt);

		// todo: why the fuck does ptr see RVO::Simulation when trying to import these methods directly?
		void setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed);
		size_t getNumAgents();
		size_t getMaxAgents();

		size_t collectSpatialData(ISpatiallyIndexable** list, size_t maxSize);

		virtual size_t find(RVO::Agent* agent, RVO::NeighborEntity* result, size_t maxResultLength);

	private:
		Spatial::IIndex2D<ISpatiallyIndexable>* _spatialIndex;
		PoolAllocator* _allocator;

	META_DECLARE_CLASS( RvoSimulation );
	};

}

#endif