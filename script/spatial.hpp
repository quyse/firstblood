#ifndef __FB_SCRIPT_SPATIAL__
#define __FB_SCRIPT_SPATIAL__

#include "gamelogic/common.hpp"
#include "inanity/ptr.hpp"
#include "inanity/math/basic.hpp"
#include "inanity/meta/decl.hpp"
#include "inanity/script/Any.hpp"

using namespace Inanity;
using namespace Inanity::Math;

namespace Firstblood
{

	class ScriptSpatialIndex : public Inanity::Object
	{
	public:
		ScriptSpatialIndex(Spatial::IIndex2D<ISpatiallyIndexable>* index);
		~ScriptSpatialIndex();

		ptr<Inanity::Script::Any> raycast(const vec3& origin, const vec3& end, uint32_t mask);
		ptr<Inanity::Script::Any> getNeighbors(const vec3& point, float distance, uint32_t mask, int maxResultLength);

	private:
		Spatial::IIndex2D<ISpatiallyIndexable>* _index;

	META_DECLARE_CLASS(ScriptSpatialIndex);
	};

}

#endif