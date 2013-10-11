#include "script/time.hpp"

namespace Firstblood
{

	ScriptTime::ScriptTime() :  _timerIdCounter(0)
	{
		_firstTimeMeasurement = Inanity::Time::GetTicks();
		_ticksPerSecond = Inanity::Time::GetTicksPerSecond();
	}

	void ScriptTime::fini()
	{
		for (size_t i = 0; i < _timers.size(); ++i)
			_timers[i]->closure->Dereference();
		_timers.clear();
	}

	float ScriptTime::getTime()
	{
		long long timeNow = Inanity::Time::GetTicks();
		long long timeSpent = timeNow - _firstTimeMeasurement;
		float seconds = static_cast<float>((double)timeSpent / (double)_ticksPerSecond);
		return seconds;
	}

	unsigned long long ScriptTime::createTimer(ptr<Inanity::Script::Any> closure, float timeSpan, bool repeatOnce)
	{
		DeferredScriptFunction* timer = _timersPool.Allocate();
		timer->id = ++_timerIdCounter;
		timer->closure = closure;
		closure->Reference();
		timer->lastInvokedAt = getTime();
		timer->timeSpan = timeSpan; 
		timer->repeatOnce = repeatOnce;
		timer->markedAsKilled = false;
		_timers.push_back(timer);
		return timer->id;
	}

	void ScriptTime::destroyTimer(unsigned long long timerId)
	{
		for (size_t i = 0; i < _timers.size(); ++i)
		{
			DeferredScriptFunction* timer = _timers[i];
			if (timerId == timer->id)
			{
				timer->markedAsKilled = true;
				break;
			}
		}
	}

	void ScriptTime::update()
	{
		for (size_t i = 0; i < _timers.size(); ++i)
		{
			DeferredScriptFunction* timer = _timers[i];
			float timeNow = getTime();
			if (!timer->markedAsKilled)
			{
				float shouldBeInvokedAt = timer->lastInvokedAt + timer->timeSpan;
				if (shouldBeInvokedAt < timeNow)
				{
					timer->closure->ApplyWith(nullptr, nullptr, 0);
					if (timer->repeatOnce)
						timer->markedAsKilled = true;
					else
						timer->lastInvokedAt = getTime();
				}
			}
			
			if (timer->markedAsKilled)
			{
				timer->closure->Dereference();
				_timersPool.Free(timer);
				_timers[i] = _timers[_timers.size() - 1];
				_timers.pop_back();
				--i;
			}
		}
	}

}