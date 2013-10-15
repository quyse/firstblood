#ifndef __FB_GAMELOGIC_RVO_HPP__
#define __FB_GAMELOGIC_RVO_HPP__

#include <vector>
#include <unordered_set>
#include "inanity/script/v8/State.hpp"
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
		virtual bool raycast(const vec3& origin, const vec3& end, float& dist) { return true; };
		virtual float getRadius() { return radius; };
		virtual vec3 getPosition() { return vec3(position.x, position.y, 0); };
		virtual uint32_t getMask() { return mask; };
		virtual vec2 getVelocity() { return velocity_; };

		void setMaxNeighbors(int value);
		void setImmobilized(bool value);
		void setTimeHorizon(float horizon);
		void setMask(uint32_t mask);
		float getMaxSpeed();
		void setMaxSpeed(float value);
		void setPrefVelocity(const vec2& velocity);

		void FreeAsNotReferenced();

	META_DECLARE_CLASS( RvoAgent );
	};


	class RvoSimulation : public RVO::Simulator, public RVO::NearestNeighborsFinder, public Inanity::Object
	{
	public:
		RvoSimulation(size_t maxAgents, Spatial::IIndex2D<ISpatiallyIndexable>* spatialIndex);
		virtual ~RvoSimulation();

		ptr<RvoAgent> create(const vec2& position, int uid);
		void destroy(ptr<RvoAgent> agent);
		void update(float dt);
		void postUpdate();

		// todo: why the fuck does ptr see RVO::Simulation when trying to import these methods directly?
		void setAgentDefaults(float neighborDist, size_t maxNeighbors, float timeHorizon, float radius, float maxSpeed);
		size_t getNumAgents();
		size_t getMaxAgents();

		size_t collectSpatialData(ISpatiallyIndexable** list, size_t maxSize);

		virtual size_t find(RVO::Agent* agent, RVO::NeighborEntity* result, size_t maxResultLength);

	private:
		Spatial::IIndex2D<ISpatiallyIndexable>* _spatialIndex;
		PoolAllocator* _allocator;
		std::vector<RvoAgent*> toBeAddedQueue;
		std::vector<RvoAgent*> toBeRemovedQueue;

	META_DECLARE_CLASS( RvoSimulation );
	};

}

#endif