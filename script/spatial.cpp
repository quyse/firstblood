#include "script/spatial.hpp"
#include "script/system.hpp"

#define MAX_SCRIPT_NEAREST_NEIGHBORS 64

namespace Firstblood
{

	ScriptSpatialIndex::ScriptSpatialIndex(Spatial::IIndex2D<ISpatiallyIndexable>* index) : _index(index)
	{
	}

	ScriptSpatialIndex::~ScriptSpatialIndex() 
	{
	}

	ptr<Inanity::Script::Any> ScriptSpatialIndex::raycast(const vec3& origin, const vec3& end, uint32_t mask)
	{
		float distance;
		ISpatiallyIndexable* entity = _index->raycast(origin, end, mask, distance);
		ScriptSystem* system = ScriptSystem::getInstance();
		ptr<Inanity::Script::Any> result = system->createScriptArray(2);
		if (entity != nullptr)
		{
			result->Set(0, system->createScriptInteger(entity->uid));
			result->Set(1, system->createScriptFloat(distance));
		}
		else
		{
			result->Set(0, system->createScriptInteger(0));
		}
		return result;
	}

	ptr<Inanity::Script::Any> ScriptSpatialIndex::getNeighbors(const vec3& point, float distance, uint32_t mask, int maxResultLength)
	{
		maxResultLength = std::min(MAX_SCRIPT_NEAREST_NEIGHBORS, maxResultLength);
		Spatial::NearestNeighbor<ISpatiallyIndexable> rawNeighbors[MAX_SCRIPT_NEAREST_NEIGHBORS];
		size_t count = _index->getNeighbours(point, distance, mask, rawNeighbors, maxResultLength);
		ScriptSystem* system = ScriptSystem::getInstance();
		ptr<Inanity::Script::Any> result = system->createScriptArray(2 * count);
		for (size_t i = 0; i < count; ++i)
		{
			result->Set(2 * i, system->createScriptInteger(rawNeighbors[i].entity->uid));
			result->Set(2 * i + 1, system->createScriptFloat(rawNeighbors[i].distance));
		}
		return result;
	}

}