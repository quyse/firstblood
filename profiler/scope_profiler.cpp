#include "scope_profiler.h"

#if defined(__INANITY_WINDOWS)

#include <Windows.h>
#include <stdio.h>

ScopeProfiler::ScopeProfiler(const char* codeAlias, const char* fileName, unsigned long lineNumber): _codeAlias(codeAlias), _fileName(fileName), _lineNumber(lineNumber)
{
	__int64 countPerSecond = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countPerSecond);
	_secondsPerCount = 1.0f / (float)countPerSecond;

	_start = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&_start);
}


ScopeProfiler::~ScopeProfiler()
{
	__int64 finish = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&finish);

	float dt = (finish - _start) * _secondsPerCount;
	processResult(dt);
}


void ScopeProfiler::processResult(float dt)
{
	printf("time for %s (%s at %u): %f\n", _codeAlias, _fileName, _lineNumber, dt);
}

#endif
