#ifndef __FB_SCRIPT_TIME_HPP__
#define __FB_SCRIPT_TIME_HPP__

#include <vector>
#include "inanity/ptr.hpp"
#include "inanity/Time.hpp"
#include "inanity/meta/decl.hpp"
#include "inanity/script/Any.hpp"
#include "inanity/TypedPool.hpp"

using namespace Inanity;

namespace Firstblood
{

	class ScriptTime : public Inanity::Object
	{
	private:
		struct DeferredScriptFunction
		{
			unsigned long long id;
			Inanity::Script::Any* closure;
			float lastInvokedAt;
			float timeSpan;
			bool repeatOnce;
			// we can't destroy timer instantly, because scripts can request timer kills during iteration over timers
			bool markedAsKilled;
		};


	public:
		ScriptTime();
		
		void fini();
		void update();

		float getTime();
		unsigned long long createTimer(ptr<Inanity::Script::Any> closure, float timeSpan, bool repeatOnce);
		void destroyTimer(unsigned long long timeoutId);

	private:
		long long _firstTimeMeasurement;
		long long _ticksPerSecond;

		std::vector<DeferredScriptFunction*> _timers;
		TypedPool<DeferredScriptFunction> _timersPool;
		unsigned long long _timerIdCounter;

	META_DECLARE_CLASS(ScriptTime);
	};

}

#endif